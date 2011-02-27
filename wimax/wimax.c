/*
 * Copyright 2011, The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "hardware_legacy/wimax.h"
#include "libwpa_client/wpa_ctrl.h"

#define LOG_TAG "WiMaxHW"

#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

static struct wpa_ctrl *ctrl_conn;
static struct wpa_ctrl *monitor_conn;

extern int do_dhcp();
extern int ifc_init();
extern void ifc_close();
extern int ifc_up(const char *name);
extern char *dhcp_lasterror();
extern void get_dhcp_info();

static char iface[PROPERTY_VALUE_MAX];

#ifndef WIMAX_DRIVER_MODULE_PATH
#define WIMAX_DRIVER_MODULE_PATH	"/system/lib/modules/sequans_sdio.ko"
#endif
#ifndef WIMAX_DRIVER_MODULE_NAME
#define WIMAX_DRIVER_MODULE_NAME	"sequans_sdio"
#endif
#ifndef WIMAX_DRIVER_MODULE_ARG
#define WIMAX_DRIVER_MODULE_ARG		""
#endif
#ifndef WIMAX_SERVICE_NAME
#define WIMAX_SERVICE_NAME		"sequansd"
#endif
#ifndef WIMAX_GETPROP_NAME
#define WIMAX_GETPROP_NAME		"getWMXPropd"
#endif
#ifndef WIMAX_SETPROP_NAME
#define WIMAX_SETPROP_NAME		"setWMXPropd"
#endif
#ifndef WIMAX_DHCP_RENEW
#define WIMAX_DHCP_RENEW		"wimaxDhcpRenew"
#endif
#ifndef WIMAX_DHCP_RELEASE
#define WIMAX_DHCP_RELEASE		"wimaxDhcpRelease"
#endif

#ifndef WIMAX_IPD
#define WIMAX_IPD                  "ipd"
#endif

#define WIMAX_TEST_INTERFACE		"thp"

#define WIMAX_DRIVER_LOADER_DELAY	1000000

static const char IFACE_DIR[]           = "/dev/thp";
static const char DRIVER_MODULE_PATH[]	= WIMAX_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_NAME[]	= WIMAX_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_TAG[]   = WIMAX_DRIVER_MODULE_NAME " ";
static const char DRIVER_MODULE_ARG[]   = WIMAX_DRIVER_MODULE_ARG;
static const char GET_PROP_NAME[]       = WIMAX_GETPROP_NAME;
static const char SET_PROP_NAME[]       = WIMAX_SETPROP_NAME;
static const char WIMAX_DHCP_START[]    = WIMAX_DHCP_RENEW;
static const char WIMAX_DHCP_NAME[]        = "dhcpWimax";
static const char WIMAX_DHCP_STOP[]     = WIMAX_DHCP_RELEASE;
static const char DRIVER_PROP_NAME[]    = "wimax.sequansd.pid";
static const char MODULE_FILE[]         = "/proc/modules";
static const char SQN_CONFIG_TEMPLATE[] = "/system/etc/wimax/sequansd/sequansd_app.xml";
static const char SQN_CONFIG_FILE[]     = "/etc/wimax/sequansd/sequansd_app.xml";
static const char PROP_CONFIG_TEMPLATE[] = "/system/etc/wimax/sequansd/DefaultTree.xml";
static const char PROP_CONFIG_FILE[]    = "/etc/wimax/sequansd/DefaultTree.xml";
static const char SERVICE_NAME[]	= WIMAX_SERVICE_NAME;
static const char IP_ROUTE_NAME[]	= WIMAX_IPD;
static const char WIMAX_ADD_ROUTE[]	= "wimaxAddRoute";
static const char WIMAX_DELETE_DEFAULT[]= "exetest";
static const char DHCP_DAEMON_NAME[]        = "dhcpcd";
static const char DHCP_DAEMON_PROP_NAME[]   = "init.svc.dhcpcd";

extern int init_module(void *, unsigned long, const char *);
extern int delete_module(const char *, unsigned int);

static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = delete_module(modname, O_NONBLOCK | O_EXCL | O_TRUNC);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        LOGD("Unable to unload driver module \"%s\": %s\n", modname, strerror(errno));
    return ret;
}

static int check_driver_loaded() {
    char driver_status[PROPERTY_VALUE_MAX];
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];
    int propExists = property_get(DRIVER_PROP_NAME, driver_status, NULL);
    
    if (!propExists || strcmp(driver_status, "") == 0) {
        return 0;  /* driver not loaded */
    }
    /*
     * If the property says the driver is loaded, check to
     * make sure that the property setting isn't just left
     * over from a previous manual shutdown or a runtime
     * crash.
     */
    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        LOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        property_set(DRIVER_PROP_NAME, "");
        return 0;
    }
    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }
    fclose(proc);
    property_set(DRIVER_PROP_NAME, "");
    return 0;
}

int unloadWimaxDriver()
{
    LOGI("NATIVE::unloadWimaxDriver() - Unloading wimax driver...");
    char pid[PROPERTY_VALUE_MAX];
    int count = 20; /* wait at most 10 seconds for completion */
    property_set("ctl.stop", SERVICE_NAME);
    if (property_get(DRIVER_PROP_NAME, pid, NULL)) {
	LOGI("NATIVE::unloadWimaxDriver() - Killing sequansd...");
        kill(atoi(pid), SIGQUIT);
    }
    sched_yield();
    property_set(DRIVER_PROP_NAME, "");
    if (rmmod(DRIVER_MODULE_NAME) == 0) {
        while (count-- > 0) {
            if (!check_driver_loaded())
                break;
            usleep(500000);
        }
        if (count) {
            return 0;
        }
        return -1;
    } else
        return -1;
}

int loadWimaxDriver()
{
    char driver_status[PROPERTY_VALUE_MAX];
    int count = 50; /* wait at most 10 seconds for completion */

	LOGI("NATIVE::loadWimaxDriver() - Checking driver...");
    if (check_driver_loaded()) {
	    LOGI("NATIVE::loadWimaxDriver() - Driver already loaded!");
        return 0;
    }

	LOGI("NATIVE::loadWimaxDriver() - insmod(driver_mod)");
    if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0)
	    LOGI("NATIVE::loadWimaxDriver() - insmod succeeded!");

        sched_yield();
        while (count-- > 0) {
            if (property_get(DRIVER_PROP_NAME, driver_status, NULL)) {
                if (strcmp(driver_status, "") != 0) {
                    usleep(100000);
                    LOGI("NATIVE::loadWimaxDriver() - sleeping to let sequansd die...");
            }
        }
    }

    count = 50;
	LOGI("NATIVE::loadWimaxDriver() - starting sequansd...");
        property_set("ctl.start", SERVICE_NAME);

    while (count-- > 0) {
        if (property_get(DRIVER_PROP_NAME, driver_status, NULL)) {
            if (strcmp(driver_status, "") != 0)
                return 0;
            else {
                unloadWimaxDriver();
                return -1;
            }
        }
        usleep(200000);
    }
    property_set(DRIVER_PROP_NAME, "");
    unloadWimaxDriver();
    return -1;
}

/*
getwimaxprop is wrong here. it should take a parameter which it gets from the htc api and store it.  the prop and the value of the prop. getwimaxprop serches mtd0 for the parameter and stores both the value and the parameter name.
*/

int getWimaxProp()
{
    int count = 200; /* wait at most 20 seconds for completion */

    property_set("ctl.start", GET_PROP_NAME);
    sched_yield();

    //while (count-- > 0) {
    //    usleep(100000);
    //}
    return -1;
}

/*
setwimaxprop is wrong here. needs two parmeters from getwimaxprop and write them to wimax_properties in data/wimax
*/
int setWimaxProp()
{
    int count = 200; /* wait at most 20 seconds for completion */

    property_set("ctl.start", SET_PROP_NAME);
    sched_yield();

    //while (count-- > 0) {
    //    usleep(100000);
    //}
    return -1;
}

int dhcpRelease() {
   int count = 75; /* wait at most 5 seconds for completion */

   property_set("ctl.stop", WIMAX_DHCP_NAME);
   usleep(10000);
   return 0;
}

int startWimaxDaemon()
{
    char wimax_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; // wait at most 15 seconds for completion

    sched_yield();
    //Check whether already running 
    if (property_get(DRIVER_PROP_NAME, wimax_status, NULL)
           && strcmp(wimax_status, "running") == 0) {
        LOGI("NATIVE::startWimaxDaemon() - daemon already running");
        return 0;

        if (strcmp(SERVICE_NAME,"") == 0) {
            LOGI("NATIVE::startWimaxDaemon() - sleeping...");
       	    usleep(100000);
        } else {
            LOGI("NATIVE::startWimaxDaemon() - starting wimax daemon!");
            property_set("ctl.start", SERVICE_NAME);
        }
    }

    while (count-- > 0) {
        if (property_get(DRIVER_PROP_NAME, wimax_status, NULL)) {
            LOGI("NATIVE::startWimaxDaemon() - DRIVER_PROP_NAME not null");
            if (strcmp(wimax_status, "") != 0) {
                LOGI("NATIVE::startWimaxDaemon() - daemon started!");
                return 0;
            } /*else {
                LOGI("NATIVE::startWimaxDaemon() - something failed - unloading driver");
                unloadWimaxDriver();
                return -1;
            }*/
        }
        usleep(100000);
    }
    property_set(DRIVER_PROP_NAME, "");
    LOGI("NATIVE::startWimaxDaemon() - fail condition...");
    unloadWimaxDriver();
    LOGI("NATIVE::startWimaxDaemon() - returning false");
    return -1;
} 

int stopWimaxDaemon()
{
    char wimax_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 300; /* wait at most 30 seconds for completion */

    /* Check whether supplicant already stopped */
    LOGI("NATIVE::stopWimaxDaemon() - checking driver loaded");
    if (property_get(DRIVER_PROP_NAME, wimax_status, NULL)
            && strcmp(wimax_status, "stopped") == 0) {
        LOGI("NATIVE::stopWimaxDaemon() - wimax driver already stopped!");
        return 0;
    }

    LOGI("NATIVE::stopWimaxDaemon() - stopping wimax daemon...");
    property_set("ctl.stop", SERVICE_NAME);
    sched_yield();

    while (count-- > 0) {
        if (property_get(DRIVER_PROP_NAME, wimax_status, NULL)) {
            if (strcmp(wimax_status, "stopped") == 0)
                LOGI("NATIVE::stopWimaxDaemon() - wimax daemon stopped!");
                return 0;
        }
        usleep(100000);
    }
    return -1;
}

int doWimaxDhcpRequest(int *ipaddr, int *gateway, int *mask,
                    int *dns1, int *dns2, int *server, int *lease) {

    if (ifc_init() < 0)
        return -1;

    if (do_dhcp(iface) < 0) {
        ifc_close();
        return -1;
    }
    ifc_close();
    get_dhcp_info(ipaddr, gateway, mask, dns1, dns2, server, lease);
    return 0;
}

const char *getWimaxDhcpError() {
    return dhcp_lasterror();
}

 int startDhcpWimaxDaemon()
{
    char dhcp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 100; /* wait at most 10 seconds for completion */

    if (property_get("dhcp.wimax0.pid", dhcp_status, NULL)) {
        if (strcmp(dhcp_status, "") != 0) {
            LOGI("NATIVE::startDhcpWimaxDaemon() - Killing dhcpcd...");
            property_set("dhcp.wimax0.pid", "");
            kill(atoi(dhcp_status), SIGQUIT);
        }
    }

    ifc_init();
    ifc_up("wimax0");
    LOGI("NATIVE::startDhcpWimaxDaemon() - wimax0 up!");

    property_set("ctl.start", WIMAX_DHCP_NAME);
    sched_yield();
    LOGI("NATIVE::startDhcpWimaxDaemon() - dhcp starting...");
   
    while (count-- > 0) {
        if (property_get("dhcp.wimax0.reason", dhcp_status, NULL)) {
            if (strcmp(dhcp_status, "BOUND") == 0 || strcmp(dhcp_status, "RENEW") == 0 || strcmp(dhcp_status, "PREINIT") == 0) {
                LOGI("NATIVE::startDhcpWimaxDaemon() - dhcp finished!");
                return 0;
            }
        }
        usleep(100000);
    }
    return -1;
}

int terminateProcess(char *pid) {
  if(!kill(atoi(pid), SIGTERM)) {
    LOGD("Process terminated successfully.");
    stopDhcpWimax();
    return 0;
  } else {
    LOGE("Process could not be killed!");
  }
  return -1;
}

int stopDhcpWimax()
{
    LOGD("Stopping DHCP...");
    char dhcp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds for completion */

    /* Check whether dhcpcd already stopped */
    if (property_get(DHCP_DAEMON_PROP_NAME, dhcp_status, NULL)
        && strcmp(dhcp_status, "stopped") == 0) {
        return 0;
    }

    property_set("ctl.stop", DHCP_DAEMON_NAME);
    sched_yield();

    while (count-- > 0) {
        if (property_get(DHCP_DAEMON_PROP_NAME, dhcp_status, NULL)) {
            if (strcmp(dhcp_status, "stopped") == 0)
                return 0;
        }
        usleep(100000);
    }
    return -1;
}

int addRouteToGateway()
{
    char wimax_status[PROPERTY_VALUE_MAX] = {'\0'};

    LOGW("addRouteToGateway() - calling wimaxAddRoute!");
    property_set("ctl.start", WIMAX_ADD_ROUTE);
   
    return 0;
}

int getBlockSize() {
        return 0;
}
