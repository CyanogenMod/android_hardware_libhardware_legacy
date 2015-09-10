/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Not a Contribution.
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
#include <errno.h>
#include <string.h>

#define LOG_TAG "WifiFST"
#include "cutils/log.h"
#include "cutils/properties.h"

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "hardware_legacy/wifi.h"

#ifndef WIFI_FST_DRIVER_MODULE_PATH
#define WIFI_FST_DRIVER_MODULE_PATH ""
#endif
#ifndef WIFI_FST_DRIVER_MODULE_ARG
#define WIFI_FST_DRIVER_MODULE_ARG ""
#endif
#ifndef WIFI_FST_DRIVER_MODULE_NAME
#define WIFI_FST_DRIVER_MODULE_NAME ""
#endif

static const char WIFI_FST_DRIVER_MODULE_TAG[] = WIFI_FST_DRIVER_MODULE_NAME " ";
static const char FST_DRIVER_PROP_NAME[] = "wlan.fst.driver.status";

static const char SUPPLICANT_GLOBAL_CTRL_IFACE[] = "@android:wpa_wlan0";
static const char FSTMAN_NAME[] = "fstman";
static const char FSTMAN_START_PROP_NAME[] = "netd.fstman.start";
static const char FSTMAN_PROP_NAME[] = "init.svc.fstman";
static const char FSTMAN_CONFIG_TEMPLATE[] = "/system/etc/wifi/fstman.ini";
static const char FSTMAN_CONFIG_FILE[] = "/data/misc/wifi/fstman.ini";
static const char FST_RATE_UPGRADE_ENABLED_PROP_NAME[] = "persist.fst.rate.upgrade.en";
static const char FST_SOFTAP_ENABLED_PROP_NAME[] = "persist.fst.softap.en";

static const char MODULE_FILE[] = "/proc/modules";

int is_fst_enabled()
{
    char prop_value[PROPERTY_VALUE_MAX] = { '\0' };

    if (property_get(FST_RATE_UPGRADE_ENABLED_PROP_NAME, prop_value, NULL) &&
        strcmp(prop_value, "1") == 0) {
        return 1;
    }

    return 0;
}

int is_fst_softap_enabled() {
    char prop_value[PROPERTY_VALUE_MAX] = { '\0' };

    if (is_fst_enabled() &&
        property_get(FST_SOFTAP_ENABLED_PROP_NAME, prop_value, NULL) &&
        strcmp(prop_value, "1") == 0) {
        return 1;
    }

    return 0;
}

int is_fst_driver_loaded()
{
    char driver_status[PROPERTY_VALUE_MAX];
    FILE *proc;
    char line[sizeof(WIFI_FST_DRIVER_MODULE_TAG)+10];

    if (!is_fst_enabled())
        return 1;

    if (!property_get(FST_DRIVER_PROP_NAME, driver_status, NULL) ||
        strcmp(driver_status, "ok") != 0)
        return 0;  /* driver not loaded */

    /*
    * If the property says the driver is loaded, check to
    * make sure that the property setting isn't just left
    * over from a previous manual shutdown or a runtime
    * crash.
    */
    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        ALOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        property_set(FST_DRIVER_PROP_NAME, "unloaded");
        return 0;
    }
    while ((fgets(line, sizeof(line), proc)) != NULL)
        if (strncmp(line, WIFI_FST_DRIVER_MODULE_TAG,
                strlen(WIFI_FST_DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }

    fclose(proc);
    property_set(FST_DRIVER_PROP_NAME, "unloaded");
    return 0;
}

int wifi_fst_load_driver()
{
    if (!is_fst_enabled())
        return 0;

    if (is_fst_driver_loaded())
        return 0;

    if (insmod(WIFI_FST_DRIVER_MODULE_PATH, WIFI_FST_DRIVER_MODULE_ARG) < 0)
        return -1;

    property_set(FST_DRIVER_PROP_NAME, "ok");

    return 0;
}

int wifi_fst_unload_driver()
{
    int count = 20; /* wait at most 10 seconds for completion */

    if (!is_fst_enabled())
        return 0;

    if (rmmod(WIFI_FST_DRIVER_MODULE_NAME) != 0)
        return -1;

    while (count-- > 0) {
        if (!is_fst_driver_loaded())
            break;
        usleep(500000);
    }
    usleep(500000); /* allow card removal */
    if (count)
        return 0;

    return -1;
}

int wifi_start_fstman(int softap_mode)
{
    char fstman_status[PROPERTY_VALUE_MAX] = { '\0' };
    char fstman_start_cmd[PROPERTY_VALUE_MAX] = { '\0' };
    int count = 50; /* wait at most 5 seconds for completion */

    if (!is_fst_enabled())
        return 0;

    if (ensure_config_file_exists(FSTMAN_CONFIG_FILE, FSTMAN_CONFIG_TEMPLATE) < 0) {
        ALOGE("Failed to create fstman config file");
        return -1;
    }

    /* Check whether already running */
    if (property_get(FSTMAN_PROP_NAME, fstman_status, NULL) &&
        strcmp(fstman_status, "running") == 0)
        return 0;

    ALOGD("Starting FST Manager");
    /* when invoked from netd, use different property because of different
       selinux permissions */
    if (softap_mode) {
        property_set(FSTMAN_START_PROP_NAME, "true");
    } else {
        snprintf(fstman_start_cmd, sizeof(fstman_start_cmd), "%s:%s",
            FSTMAN_NAME, SUPPLICANT_GLOBAL_CTRL_IFACE);
        property_set("ctl.start", fstman_start_cmd);
    }
    sched_yield();

    while (count-- > 0) {
        if (property_get(FSTMAN_PROP_NAME, fstman_status, NULL) &&
            strcmp(fstman_status, "running") == 0)
                return 0;
        usleep(100000);
    }

    ALOGE("Failed to start FST Manager");
    return -1;
}

int wifi_stop_fstman(int softap_mode)
{
    char fstman_status[PROPERTY_VALUE_MAX] = { '\0' };
    int count = 50; /* wait at most 5 seconds for completion */

    if (!is_fst_enabled())
        return 0;

    /* Check whether already stopped */
    if (property_get(FSTMAN_PROP_NAME, fstman_status, NULL) &&
        strcmp(fstman_status, "stopped") == 0)
        return 0;

    ALOGD("Stopping FST Manager");
    /* when invoked from netd, use different property because of different
       selinux permissions */
    if (softap_mode)
        property_set(FSTMAN_START_PROP_NAME, "false");
    else
        property_set("ctl.stop", FSTMAN_NAME);
    sched_yield();

    while (count-- > 0) {
        if (property_get(FSTMAN_PROP_NAME, fstman_status, NULL) &&
            strcmp(fstman_status, "stopped") == 0)
                return 0;
        usleep(100000);
    }

    ALOGE("Failed to stop fstman");
    return -1;
}
