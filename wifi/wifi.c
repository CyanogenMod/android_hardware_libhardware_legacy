/*
 * Copyright 2008, The Android Open Source Project
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
#include <dirent.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#ifdef USES_TI_MAC80211
#include <dirent.h>
#include <net/if.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>
#endif

#include "hardware_legacy/wifi.h"
#ifdef LIBWPA_CLIENT_EXISTS
#include "libwpa_client/wpa_ctrl.h"
#endif

#define LOG_TAG "WifiHW"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

extern int do_dhcp();
extern int ifc_init();
extern void ifc_close();
extern char *dhcp_lasterror();
extern void get_dhcp_info();
extern int init_module(void *, unsigned long, const char *);
extern int delete_module(const char *, unsigned int);
void wifi_close_sockets();

#ifndef LIBWPA_CLIENT_EXISTS
#define WPA_EVENT_TERMINATING "CTRL-EVENT-TERMINATING "
struct wpa_ctrl {};
void wpa_ctrl_cleanup(void) {}
struct wpa_ctrl *wpa_ctrl_open(const char *ctrl_path) { return NULL; }
void wpa_ctrl_close(struct wpa_ctrl *ctrl) {}
int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
	char *reply, size_t *reply_len, void (*msg_cb)(char *msg, size_t len))
	{ return 0; }
int wpa_ctrl_attach(struct wpa_ctrl *ctrl) { return 0; }
int wpa_ctrl_detach(struct wpa_ctrl *ctrl) { return 0; }
int wpa_ctrl_recv(struct wpa_ctrl *ctrl, char *reply, size_t *reply_len)
	{ return 0; }
int wpa_ctrl_get_fd(struct wpa_ctrl *ctrl) { return 0; }
#endif

static struct wpa_ctrl *ctrl_conn;
static struct wpa_ctrl *monitor_conn;

/* socket pair used to exit from a blocking read */
static int exit_sockets[2];
static int wifi_mode = 0;

static char primary_iface[PROPERTY_VALUE_MAX];
// TODO: use new ANDROID_SOCKET mechanism, once support for multiple
// sockets is in

#ifdef USES_TI_MAC80211
#define P2P_INTERFACE			"p2p0"
struct nl_sock *nl_soc;
struct nl_cache *nl_cache;
struct genl_family *nl80211;
#endif

#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG          ""
#endif
#ifndef WIFI_DRIVER_MODULE_AP_ARG
#define WIFI_DRIVER_MODULE_AP_ARG       ""
#endif
#ifndef WIFI_FIRMWARE_LOADER
#define WIFI_FIRMWARE_LOADER		""
#endif
#define WIFI_TEST_INTERFACE		"sta"

#ifndef WIFI_DRIVER_FW_PATH_STA
#define WIFI_DRIVER_FW_PATH_STA		NULL
#endif
#ifndef WIFI_DRIVER_FW_PATH_AP
#define WIFI_DRIVER_FW_PATH_AP		NULL
#endif
#ifndef WIFI_DRIVER_FW_PATH_P2P
#define WIFI_DRIVER_FW_PATH_P2P		NULL
#endif

#ifdef WIFI_EXT_MODULE_NAME
static const char EXT_MODULE_NAME[] = WIFI_EXT_MODULE_NAME;
#ifdef WIFI_EXT_MODULE_ARG
static const char EXT_MODULE_ARG[] = WIFI_EXT_MODULE_ARG;
#else
static const char EXT_MODULE_ARG[] = "";
#endif
#endif
#ifdef WIFI_EXT_MODULE_PATH
static const char EXT_MODULE_PATH[] = WIFI_EXT_MODULE_PATH;
#endif

#ifndef WIFI_DRIVER_FW_PATH_PARAM
#define WIFI_DRIVER_FW_PATH_PARAM	"/sys/module/wlan/parameters/fwpath"
#endif

#define WIFI_DRIVER_LOADER_DELAY	1000000

static const char IFACE_DIR[]           = "/data/system/wpa_supplicant";
#ifdef WIFI_DRIVER_MODULE_PATH
static const char DRIVER_MODULE_NAME[]  = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME " ";
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = WIFI_DRIVER_MODULE_ARG;
static const char DRIVER_MODULE_AP_ARG[] = WIFI_DRIVER_MODULE_AP_ARG;
#endif
static const char FIRMWARE_LOADER[]     = WIFI_FIRMWARE_LOADER;
static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";
static const char SUPPLICANT_NAME[]     = "wpa_supplicant";
static const char SUPP_PROP_NAME[]      = "init.svc.wpa_supplicant";
static const char P2P_SUPPLICANT_NAME[] = "p2p_supplicant";
static const char P2P_PROP_NAME[]       = "init.svc.p2p_supplicant";
static const char SUPP_CONFIG_TEMPLATE[]= "/system/etc/wifi/wpa_supplicant.conf";
static const char SUPP_CONFIG_FILE[]    = "/data/misc/wifi/wpa_supplicant.conf";
static const char P2P_CONFIG_FILE[]     = "/data/misc/wifi/p2p_supplicant.conf";
static const char CONTROL_IFACE_PATH[]  = "/data/misc/wifi/sockets";
static const char MODULE_FILE[]         = "/proc/modules";

static const char IFNAME[]              = "IFNAME=";
#define IFNAMELEN			(sizeof(IFNAME) - 1)
static const char WPA_EVENT_IGNORE[]    = "CTRL-EVENT-IGNORE ";

static const char SUPP_ENTROPY_FILE[]   = WIFI_ENTROPY_FILE;
static unsigned char dummy_key[21] = { 0x02, 0x11, 0xbe, 0x33, 0x43, 0x35,
                                       0x68, 0x47, 0x84, 0x99, 0xa9, 0x2b,
                                       0x1c, 0xd3, 0xee, 0xff, 0xf1, 0xe2,
                                       0xf3, 0xf4, 0xf5 };

/* Is either SUPPLICANT_NAME or P2P_SUPPLICANT_NAME */
static char supplicant_name[PROPERTY_VALUE_MAX];
/* Is either SUPP_PROP_NAME or P2P_PROP_NAME */
static char supplicant_prop_name[PROPERTY_KEY_MAX];


#ifdef SAMSUNG_WIFI
char* get_samsung_wifi_type()
{
    char buf[10];
    int fd = open("/data/.cid.info", O_RDONLY);
    if (fd < 0)
        return NULL;

    if (read(fd, buf, sizeof(buf)) < 0) {
        close(fd);
        return NULL;
    }

    close(fd);

    if (strncmp(buf, "murata", 6) == 0)
        return "_murata";

    if (strncmp(buf, "semcove", 7) == 0)
        return "_semcove";

    if (strncmp(buf, "semcosh", 7) == 0)
        return "_semcosh";

    return NULL;
}
#endif

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
        ret = delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        ALOGD("Unable to unload driver module \"%s\": %s\n",
             modname, strerror(errno));
    return ret;
}

int do_dhcp_request(int *ipaddr, int *gateway, int *mask,
                    int *dns1, int *dns2, int *server, int *lease) {
    /* For test driver, always report success */
    if (strcmp(primary_iface, WIFI_TEST_INTERFACE) == 0)
        return 0;

    if (ifc_init() < 0)
        return -1;

    if (do_dhcp(primary_iface) < 0) {
        ifc_close();
        return -1;
    }
    ifc_close();
    get_dhcp_info(ipaddr, gateway, mask, dns1, dns2, server, lease);
    return 0;
}

const char *get_dhcp_error_string() {
    return dhcp_lasterror();
}

int is_wifi_driver_loaded() {
    char driver_status[PROPERTY_VALUE_MAX];
#ifdef WIFI_DRIVER_MODULE_PATH
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];
#endif

    if (!property_get(DRIVER_PROP_NAME, driver_status, NULL)
            || strcmp(driver_status, "ok") != 0) {
        return 0;  /* driver not loaded */
    }
#ifdef WIFI_DRIVER_MODULE_PATH
    /*
     * If the property says the driver is loaded, check to
     * make sure that the property setting isn't just left
     * over from a previous manual shutdown or a runtime
     * crash.
     */
    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        ALOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        property_set(DRIVER_PROP_NAME, "unloaded");
        return 0;
    }
    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }
    fclose(proc);
    property_set(DRIVER_PROP_NAME, "unloaded");
    return 0;
#else
    return 1;
#endif
}

int wifi_load_driver()
{
#ifdef WIFI_DRIVER_MODULE_PATH
    char driver_status[PROPERTY_VALUE_MAX];
    int count = 100; /* wait at most 20 seconds for completion */
    char module_arg2[256];
#ifdef SAMSUNG_WIFI
    char* type = get_samsung_wifi_type();

    if (wifi_mode == 1) {
        snprintf(module_arg2, sizeof(module_arg2), "%s%s", DRIVER_MODULE_AP_ARG, type == NULL ? "" : type);
    } else {
        snprintf(module_arg2, sizeof(module_arg2), "%s%s", DRIVER_MODULE_ARG, type == NULL ? "" : type);
    }

    if (insmod(DRIVER_MODULE_PATH, module_arg2) < 0) {
#else

    property_set(DRIVER_PROP_NAME, "loading");

#ifdef WIFI_EXT_MODULE_PATH
    if (insmod(EXT_MODULE_PATH, EXT_MODULE_ARG) < 0)
        return -1;
    usleep(200000);
#endif

    if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0) {
#endif

#ifdef WIFI_EXT_MODULE_NAME
        rmmod(EXT_MODULE_NAME);
#endif
        return -1;
    }

    if (strcmp(FIRMWARE_LOADER,"") == 0) {
        /* usleep(WIFI_DRIVER_LOADER_DELAY); */
        property_set(DRIVER_PROP_NAME, "ok");
    }
    else {
        property_set("ctl.start", FIRMWARE_LOADER);
    }
    sched_yield();
    while (count-- > 0) {
        if (property_get(DRIVER_PROP_NAME, driver_status, NULL)) {
            if (strcmp(driver_status, "ok") == 0)
                return 0;
            else if (strcmp(driver_status, "failed") == 0) {
                wifi_unload_driver();
                return -1;
            }
        }
        usleep(200000);
    }
    property_set(DRIVER_PROP_NAME, "timeout");
    wifi_unload_driver();
    return -1;
#else
    property_set(DRIVER_PROP_NAME, "ok");
    return 0;
#endif
}

int wifi_unload_driver()
{
    usleep(200000); /* allow to finish interface down */
#ifdef WIFI_DRIVER_MODULE_PATH
    if (rmmod(DRIVER_MODULE_NAME) == 0) {
        int count = 20; /* wait at most 10 seconds for completion */
        while (count-- > 0) {
            if (!is_wifi_driver_loaded())
                break;
            usleep(500000);
        }
        usleep(500000); /* allow card removal */
        if (count) {
#ifdef WIFI_EXT_MODULE_NAME
            if (rmmod(EXT_MODULE_NAME) == 0)
#endif
            return 0;
        }
        return -1;
    } else
        return -1;
#else
    property_set(DRIVER_PROP_NAME, "unloaded");
    return 0;
#endif
}

int ensure_entropy_file_exists()
{
    int ret;
    int destfd;

    ret = access(SUPP_ENTROPY_FILE, R_OK|W_OK);
    if ((ret == 0) || (errno == EACCES)) {
        if ((ret != 0) &&
            (chmod(SUPP_ENTROPY_FILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
            ALOGE("Cannot set RW to \"%s\": %s", SUPP_ENTROPY_FILE, strerror(errno));
            return -1;
        }
        return 0;
    }
    destfd = TEMP_FAILURE_RETRY(open(SUPP_ENTROPY_FILE, O_CREAT|O_RDWR, 0660));
    if (destfd < 0) {
        ALOGE("Cannot create \"%s\": %s", SUPP_ENTROPY_FILE, strerror(errno));
        return -1;
    }

    if (TEMP_FAILURE_RETRY(write(destfd, dummy_key, sizeof(dummy_key))) != sizeof(dummy_key)) {
        ALOGE("Error writing \"%s\": %s", SUPP_ENTROPY_FILE, strerror(errno));
        close(destfd);
        return -1;
    }
    close(destfd);

    /* chmod is needed because open() didn't set permisions properly */
    if (chmod(SUPP_ENTROPY_FILE, 0660) < 0) {
        ALOGE("Error changing permissions of %s to 0660: %s",
             SUPP_ENTROPY_FILE, strerror(errno));
        unlink(SUPP_ENTROPY_FILE);
        return -1;
    }

    if (chown(SUPP_ENTROPY_FILE, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
             SUPP_ENTROPY_FILE, AID_WIFI, strerror(errno));
        unlink(SUPP_ENTROPY_FILE);
        return -1;
    }
    return 0;
}

int ensure_config_file_exists(const char *config_file)
{
    char buf[2048];
    int srcfd, destfd;
    struct stat sb;
    int nread;
    int ret;

    ret = access(config_file, R_OK|W_OK);
    if ((ret == 0) || (errno == EACCES)) {
        if ((ret != 0) &&
            (chmod(config_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
            ALOGE("Cannot set RW to \"%s\": %s", config_file, strerror(errno));
            return -1;
        }
        return 0;
    } else if (errno != ENOENT) {
        ALOGE("Cannot access \"%s\": %s", config_file, strerror(errno));
        return -1;
    }

    srcfd = TEMP_FAILURE_RETRY(open(SUPP_CONFIG_TEMPLATE, O_RDONLY));
    if (srcfd < 0) {
        ALOGE("Cannot open \"%s\": %s", SUPP_CONFIG_TEMPLATE, strerror(errno));
        return -1;
    }

    destfd = TEMP_FAILURE_RETRY(open(config_file, O_CREAT|O_RDWR, 0660));
    if (destfd < 0) {
        close(srcfd);
        ALOGE("Cannot create \"%s\": %s", config_file, strerror(errno));
        return -1;
    }

    while ((nread = TEMP_FAILURE_RETRY(read(srcfd, buf, sizeof(buf)))) != 0) {
        if (nread < 0) {
            ALOGE("Error reading \"%s\": %s", SUPP_CONFIG_TEMPLATE, strerror(errno));
            close(srcfd);
            close(destfd);
            unlink(config_file);
            return -1;
        }
        TEMP_FAILURE_RETRY(write(destfd, buf, nread));
    }

    close(destfd);
    close(srcfd);

    /* chmod is needed because open() didn't set permisions properly */
    if (chmod(config_file, 0660) < 0) {
        ALOGE("Error changing permissions of %s to 0660: %s",
             config_file, strerror(errno));
        unlink(config_file);
        return -1;
    }

    if (chown(config_file, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
             config_file, AID_WIFI, strerror(errno));
        unlink(config_file);
        return -1;
    }
    return 0;
}

#ifdef USES_TI_MAC80211
static int init_nl()
{
    int err;

    nl_soc = nl_socket_alloc();
    if (!nl_soc) {
        ALOGE("Failed to allocate netlink socket.");
        return -ENOMEM;
    }

    if (genl_connect(nl_soc)) {
        ALOGE("Failed to connect to generic netlink.");
        err = -ENOLINK;
        goto out_handle_destroy;
    }

    genl_ctrl_alloc_cache(nl_soc, &nl_cache);
    if (!nl_cache) {
        ALOGE("Failed to allocate generic netlink cache.");
        err = -ENOMEM;
        goto out_handle_destroy;
    }

    nl80211 = genl_ctrl_search_by_name(nl_cache, "nl80211");
    if (!nl80211) {
        ALOGE("nl80211 not found.");
        err = -ENOENT;
        goto out_cache_free;
    }

    return 0;

out_cache_free:
    nl_cache_free(nl_cache);
out_handle_destroy:
    nl_socket_free(nl_soc);
    return err;
}

static void deinit_nl()
{
    genl_family_put(nl80211);
    nl_cache_free(nl_cache);
    nl_socket_free(nl_soc);
}

// ignore the "." and ".." entries
static int dir_filter(const struct dirent *name)
{
    if (0 == strcmp("..", name->d_name) ||
        0 == strcmp(".", name->d_name))
            return 0;

    return 1;
}

// lookup the only active phy
int phy_lookup()
{
    char buf[200];
    int fd, pos;
    struct dirent **namelist;
    int n, i;

    n = scandir("/sys/class/ieee80211", &namelist, dir_filter,
                (int (*)(const struct dirent**, const struct dirent**))alphasort);
    if (n != 1) {
        ALOGE("unexpected - found %d phys in /sys/class/ieee80211", n);
        for (i = 0; i < n; i++)
            free(namelist[i]);
        free(namelist);
        return -1;
    }

    snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index",
             namelist[0]->d_name);
    free(namelist[0]);
    free(namelist);

    fd = open(buf, O_RDONLY);
    if (fd < 0)
        return -1;
    pos = read(fd, buf, sizeof(buf) - 1);
    if (pos < 0) {
        close(fd);
        return -1;
    }
    buf[pos] = '\0';
    close(fd);
    return atoi(buf);
}

int nl_error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
    int *ret = (int *)arg;
    *ret = err->error;
    return NL_STOP;
}

int nl_finish_handler(struct nl_msg *msg, void *arg)
{
     int *ret = (int *)arg;
     *ret = 0;
     return NL_SKIP;
}

int nl_ack_handler(struct nl_msg *msg, void *arg)
{
    int *ret = (int *)arg;
    *ret = 0;
    return NL_STOP;
}

static int execute_nl_interface_cmd(const char *iface,
                                    enum nl80211_iftype type,
                                    uint8_t cmd)
{
    struct nl_cb *cb;
    struct nl_msg *msg;
    int devidx = 0;
    int err;
    int add_interface = (cmd == NL80211_CMD_NEW_INTERFACE);

    if (add_interface) {
        devidx = phy_lookup();
    } else {
        devidx = if_nametoindex(iface);
        if (devidx == 0) {
            ALOGE("failed to translate ifname to idx");
            return -errno;
        }
    }

    msg = nlmsg_alloc();
    if (!msg) {
        ALOGE("failed to allocate netlink message");
        return 2;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        ALOGE("failed to allocate netlink callbacks");
        err = 2;
        goto out_free_msg;
    }

    genlmsg_put(msg, 0, 0, genl_family_get_id(nl80211), 0, 0, cmd, 0);

    if (add_interface) {
        NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, devidx);
    } else {
        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
    }

    if (add_interface) {
        NLA_PUT_STRING(msg, NL80211_ATTR_IFNAME, iface);
        NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, type);
    }

    err = nl_send_auto_complete(nl_soc, msg);
    if (err < 0)
        goto out;

    err = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, nl_error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, nl_finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, nl_ack_handler, &err);

    while (err > 0)
        nl_recvmsgs(nl_soc, cb);
out:
    nl_cb_put(cb);
out_free_msg:
    nlmsg_free(msg);
    return err;
nla_put_failure:
    ALOGW("building message failed");
    return 2;
}

int add_remove_p2p_interface(int add)
{
    int ret;

    ret = init_nl();
    if (ret != 0)
        return ret;

    if (add) {
        ret = execute_nl_interface_cmd(P2P_INTERFACE, NL80211_IFTYPE_STATION,
                                       NL80211_CMD_NEW_INTERFACE);
        if (ret != 0) {
            ALOGE("could not add P2P interface: %d", ret);
            goto cleanup;
        }
    } else {
        ret = execute_nl_interface_cmd(P2P_INTERFACE, NL80211_IFTYPE_STATION,
                                       NL80211_CMD_DEL_INTERFACE);
        if (ret != 0) {
            ALOGE("could not remove P2P interface: %d", ret);
            goto cleanup;
        }
    }

    ALOGD("added/removed p2p interface. add: %d", add);

cleanup:
    deinit_nl();
    return ret;
}
#endif /* USES_TI_MAC80211 */

int wifi_start_supplicant(int p2p_supported)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 200; /* wait at most 20 seconds for completion */
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    const prop_info *pi;
    unsigned serial = 0, i;
#endif

    if (p2p_supported) {
        strcpy(supplicant_name, P2P_SUPPLICANT_NAME);
        strcpy(supplicant_prop_name, P2P_PROP_NAME);

        /* Ensure p2p config file is created */
        if (ensure_config_file_exists(P2P_CONFIG_FILE) < 0) {
            ALOGE("Failed to create a p2p config file");
            return -1;
        }

    } else {
        strcpy(supplicant_name, SUPPLICANT_NAME);
        strcpy(supplicant_prop_name, SUPP_PROP_NAME);
    }

    /* Check whether already running */
    if (property_get(supplicant_prop_name, supp_status, NULL)
            && strcmp(supp_status, "running") == 0) {
        return 0;
    }

    /* Before starting the daemon, make sure its config file exists */
    if (ensure_config_file_exists(SUPP_CONFIG_FILE) < 0) {
        ALOGE("Wi-Fi will not be enabled");
        return -1;
    }

    if (ensure_entropy_file_exists() < 0) {
        ALOGE("Wi-Fi entropy file was not created");
    }

#ifdef USES_TI_MAC80211
    if (p2p_supported && add_remove_p2p_interface(1) < 0) {
        ALOGE("Wi-Fi - could not create p2p interface");
        return -1;
    }
#endif

    /* Clear out any stale socket files that might be left over. */
    wpa_ctrl_cleanup();

    /* Reset sockets used for exiting from hung state */
    exit_sockets[0] = exit_sockets[1] = -1;

#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    /*
     * Get a reference to the status property, so we can distinguish
     * the case where it goes stopped => running => stopped (i.e.,
     * it start up, but fails right away) from the case in which
     * it starts in the stopped state and never manages to start
     * running at all.
     */
    pi = __system_property_find(supplicant_prop_name);
    if (pi != NULL) {
        serial = __system_property_serial(pi);
    }
#endif
    property_get("wifi.interface", primary_iface, WIFI_TEST_INTERFACE);

    property_set("ctl.start", supplicant_name);
    sched_yield();

    while (count-- > 0) {
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
        if (pi == NULL) {
            pi = __system_property_find(supplicant_prop_name);
        }
        if (pi != NULL) {
            /*
             * property serial updated means that init process is scheduled
             * after we sched_yield, further property status checking is based on this */
            if (__system_property_serial(pi) != serial) {
                __system_property_read(pi, NULL, supp_status);
                if (strcmp(supp_status, "running") == 0) {
                    return 0;
                } else if (strcmp(supp_status, "stopped") == 0) {
                    return -1;
                }
            }
        }
#else
        if (property_get(supplicant_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "running") == 0)
                return 0;
        }
#endif
        usleep(100000);
    }
    return -1;
}

int wifi_stop_supplicant(int p2p_supported)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds for completion */

    if (p2p_supported) {
        strcpy(supplicant_name, P2P_SUPPLICANT_NAME);
        strcpy(supplicant_prop_name, P2P_PROP_NAME);
    } else {
        strcpy(supplicant_name, SUPPLICANT_NAME);
        strcpy(supplicant_prop_name, SUPP_PROP_NAME);
    }

    /* Check whether supplicant already stopped */
    if (property_get(supplicant_prop_name, supp_status, NULL)
        && strcmp(supp_status, "stopped") == 0) {
        return 0;
    }

#ifdef USES_TI_MAC80211
    if (p2p_supported && add_remove_p2p_interface(0) < 0) {
        ALOGE("Wi-Fi - could not remove p2p interface");
        return -1;
    }
#endif

    property_set("ctl.stop", supplicant_name);
    sched_yield();

    while (count-- > 0) {
        if (property_get(supplicant_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "stopped") == 0)
                return 0;
        }
        usleep(100000);
    }
    ALOGE("Failed to stop supplicant");
    return -1;
}

int wifi_connect_on_socket_path(const char *path)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    /* Make sure supplicant is running */
    if (!property_get(supplicant_prop_name, supp_status, NULL)
            || strcmp(supp_status, "running") != 0) {
        ALOGE("Supplicant not running, cannot connect");
        return -1;
    }

    ctrl_conn = wpa_ctrl_open(path);
    if (ctrl_conn == NULL) {
        ALOGE("Unable to open connection to supplicant on \"%s\": %s",
             path, strerror(errno));
        return -1;
    }
    monitor_conn = wpa_ctrl_open(path);
    if (monitor_conn == NULL) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
        return -1;
    }
    if (wpa_ctrl_attach(monitor_conn) != 0) {
        wpa_ctrl_close(monitor_conn);
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = monitor_conn = NULL;
        return -1;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets) == -1) {
        wpa_ctrl_close(monitor_conn);
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = monitor_conn = NULL;
        return -1;
    }

    return 0;
}

/* Establishes the control and monitor socket connections on the interface */
int wifi_connect_to_supplicant()
{
    static char path[PATH_MAX];

    if (access(IFACE_DIR, F_OK) == 0) {
        snprintf(path, sizeof(path), "%s/%s", IFACE_DIR, primary_iface);
    } else {
        snprintf(path, sizeof(path), "@android:wpa_%s", primary_iface);
    }
    return wifi_connect_on_socket_path(path);
}

int wifi_send_command(const char *cmd, char *reply, size_t *reply_len)
{
    int ret;
    if (ctrl_conn == NULL) {
        ALOGV("Not connected to wpa_supplicant - \"%s\" command dropped.\n", cmd);
        return -1;
    }
    ret = wpa_ctrl_request(ctrl_conn, cmd, strlen(cmd), reply, reply_len, NULL);
    if (ret == -2) {
        ALOGD("'%s' command timed out.\n", cmd);
        /* unblocks the monitor receive socket for termination */
        TEMP_FAILURE_RETRY(write(exit_sockets[0], "T", 1));
        return -2;
    } else if (ret < 0 || strncmp(reply, "FAIL", 4) == 0) {
        return -1;
    }
    if (strncmp(cmd, "PING", 4) == 0) {
        reply[*reply_len] = '\0';
    }
    return 0;
}

int wifi_supplicant_connection_active()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    if (property_get(supplicant_prop_name, supp_status, NULL)) {
        if (strcmp(supp_status, "stopped") == 0)
            return -1;
    }

    return 0;
}

int wifi_ctrl_recv(char *reply, size_t *reply_len)
{
    int res;
    int ctrlfd;
    struct pollfd rfds[2];

    if (monitor_conn == NULL) {
        ALOGE("%s: monitor_conn is NULL\n", __func__);
        return -2;
    }
    ctrlfd = wpa_ctrl_get_fd(monitor_conn);
    memset(rfds, 0, 2 * sizeof(struct pollfd));
    rfds[0].fd = ctrlfd;
    rfds[0].events |= POLLIN;
    rfds[1].fd = exit_sockets[1];
    rfds[1].events |= POLLIN;
    do {
        res = TEMP_FAILURE_RETRY(poll(rfds, 2, 30000));
        if (res < 0) {
            ALOGE("Error poll = %d", res);
            return res;
        } else if (res == 0) {
            /* timed out, check if supplicant is active
             * or not ..
             */
            res = wifi_supplicant_connection_active();
            if (res < 0)
                return -2;
        }
    } while (res == 0);

    if (rfds[0].revents & POLLIN) {
        return wpa_ctrl_recv(monitor_conn, reply, reply_len);
    }

    /* it is not rfds[0], then it must be rfts[1] (i.e. the exit socket)
     * or we timed out. In either case, this call has failed ..
     */
    return -2;
}

int wifi_wait_on_socket(char *buf, size_t buflen)
{
    size_t nread = buflen - 1;
    int result;
    char *match, *match2;

    if (monitor_conn == NULL) {
        return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
                        primary_iface, WPA_EVENT_TERMINATING);
    }

    result = wifi_ctrl_recv(buf, &nread);

    /* Terminate reception on exit socket */
    if (result == -2) {
        return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
                        primary_iface, WPA_EVENT_TERMINATING);
    }

    if (result < 0) {
        ALOGD("wifi_ctrl_recv failed: %s\n", strerror(errno));
        return snprintf(buf, buflen, "IFNAME=%s %s - recv error",
                        primary_iface, WPA_EVENT_TERMINATING);
    }
    buf[nread] = '\0';
    /* Check for EOF on the socket */
    if (result == 0 && nread == 0) {
        /* Fabricate an event to pass up */
        ALOGD("Received EOF on supplicant socket\n");
        return snprintf(buf, buflen, "IFNAME=%s %s - signal 0 received",
                        primary_iface, WPA_EVENT_TERMINATING);
    }
    /*
     * Events strings are in the format
     *
     *     IFNAME=iface <N>CTRL-EVENT-XXX 
     *        or
     *     <N>CTRL-EVENT-XXX 
     *
     * where N is the message level in numerical form (0=VERBOSE, 1=DEBUG,
     * etc.) and XXX is the event name. The level information is not useful
     * to us, so strip it off.
     */

    if (strncmp(buf, IFNAME, IFNAMELEN) == 0) {
        match = strchr(buf, ' ');
        if (match != NULL) {
            if (match[1] == '<') {
                match2 = strchr(match + 2, '>');
                if (match2 != NULL) {
                    nread -= (match2 - match);
                    memmove(match + 1, match2 + 1, nread - (match - buf) + 1);
                }
            }
        } else {
            return snprintf(buf, buflen, "%s", WPA_EVENT_IGNORE);
        }
    } else if (buf[0] == '<') {
        match = strchr(buf, '>');
        if (match != NULL) {
            nread -= (match + 1 - buf);
            memmove(buf, match + 1, nread + 1);
            ALOGV("supplicant generated event without interface - %s\n", buf);
        }
    } else {
        /* let the event go as is! */
        ALOGW("supplicant generated event without interface and without message level - %s\n", buf);
    }

    return nread;
}

int wifi_wait_for_event(char *buf, size_t buflen)
{
    return wifi_wait_on_socket(buf, buflen);
}

void wifi_close_sockets()
{
    if (ctrl_conn != NULL) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
    }

    if (monitor_conn != NULL) {
        wpa_ctrl_close(monitor_conn);
        monitor_conn = NULL;
    }

    if (exit_sockets[0] >= 0) {
        close(exit_sockets[0]);
        exit_sockets[0] = -1;
    }

    if (exit_sockets[1] >= 0) {
        close(exit_sockets[1]);
        exit_sockets[1] = -1;
    }
}

void wifi_close_supplicant_connection()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds to ensure init has stopped stupplicant */

    wifi_close_sockets();

    while (count-- > 0) {
        if (property_get(supplicant_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "stopped") == 0)
                return;
        }
        usleep(100000);
    }
}

int wifi_command(const char *command, char *reply, size_t *reply_len)
{
    return wifi_send_command(command, reply, reply_len);
}

const char *wifi_get_fw_path(int fw_type)
{
    switch (fw_type) {
    case WIFI_GET_FW_PATH_STA:
        return WIFI_DRIVER_FW_PATH_STA;
    case WIFI_GET_FW_PATH_AP:
        return WIFI_DRIVER_FW_PATH_AP;
    case WIFI_GET_FW_PATH_P2P:
        return WIFI_DRIVER_FW_PATH_P2P;
    }
    return NULL;
}

int wifi_change_fw_path(const char *fwpath)
{
    int len;
    int fd;
    int ret = 0;

    if (!fwpath)
        return ret;
    fd = TEMP_FAILURE_RETRY(open(WIFI_DRIVER_FW_PATH_PARAM, O_WRONLY));
    if (fd < 0) {
        ALOGE("Failed to open wlan fw path param (%s)", strerror(errno));
        return -1;
    }
    len = strlen(fwpath) + 1;
    if (TEMP_FAILURE_RETRY(write(fd, fwpath, len)) != len) {
        ALOGE("Failed to write wlan fw path param (%s)", strerror(errno));
        ret = -1;
    }
    close(fd);
    return ret;
}

int wifi_set_mode(int mode) {
    wifi_mode = mode;
    return 0;
}
