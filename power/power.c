/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <hardware_legacy/power.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#define LOG_TAG "power"
#include <utils/Log.h>
#ifdef QCOM_HARDWARE
#include <cutils/properties.h>
#define DMMDEBUG 0
#endif

#include "qemu.h"
#ifdef QEMU_POWER
#include "power_qemu.h"
#endif

enum {
    ACQUIRE_PARTIAL_WAKE_LOCK = 0,
    RELEASE_WAKE_LOCK,
    REQUEST_STATE,
    OUR_FD_COUNT
};

const char * const OLD_PATHS[] = {
    "/sys/android_power/acquire_partial_wake_lock",
    "/sys/android_power/release_wake_lock",
    "/sys/android_power/request_state"
};

const char * const NEW_PATHS[] = {
    "/sys/power/wake_lock",
    "/sys/power/wake_unlock",
    "/sys/power/state"
};

const char * const AUTO_OFF_TIMEOUT_DEV = "/sys/android_power/auto_off_timeout";

//XXX static pthread_once_t g_initialized = THREAD_ONCE_INIT;
static int g_initialized = 0;
static int g_fds[OUR_FD_COUNT];
static int g_error = 1;

static const char *off_state = "mem";
static const char *on_state = "on";

static int64_t systemTime()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000LL + t.tv_nsec;
}

static int
open_file_descriptors(const char * const paths[])
{
    int i;
    for (i=0; i<OUR_FD_COUNT; i++) {
        int fd = open(paths[i], O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "fatal error opening \"%s\"\n", paths[i]);
            g_error = errno;
            return -1;
        }
        g_fds[i] = fd;
    }

    g_error = 0;
    return 0;
}

static inline void
initialize_fds(void)
{
    // XXX: should be this:
    //pthread_once(&g_initialized, open_file_descriptors);
    // XXX: not this:
    if (g_initialized == 0) {
        if(open_file_descriptors(NEW_PATHS) < 0) {
            open_file_descriptors(OLD_PATHS);
            on_state = "wake";
            off_state = "standby";
        }
        g_initialized = 1;
    }
}

int
acquire_wake_lock(int lock, const char* id)
{
    initialize_fds();

//    LOGI("acquire_wake_lock lock=%d id='%s'\n", lock, id);

    if (g_error) return g_error;

    int fd;

    if (lock == PARTIAL_WAKE_LOCK) {
        fd = g_fds[ACQUIRE_PARTIAL_WAKE_LOCK];
    }
    else {
        return EINVAL;
    }

    return write(fd, id, strlen(id));
}

int
release_wake_lock(const char* id)
{
    initialize_fds();

//    LOGI("release_wake_lock id='%s'\n", id);

    if (g_error) return g_error;

    ssize_t len = write(g_fds[RELEASE_WAKE_LOCK], id, strlen(id));
    return len >= 0;
}

int
set_last_user_activity_timeout(int64_t delay)
{
//    LOGI("set_last_user_activity_timeout delay=%d\n", ((int)(delay)));

    int fd = open(AUTO_OFF_TIMEOUT_DEV, O_RDWR);
    if (fd >= 0) {
        char buf[32];
        ssize_t len;
        len = snprintf(buf, sizeof(buf), "%d", ((int)(delay)));
        buf[sizeof(buf) - 1] = '\0';
        len = write(fd, buf, len);
        close(fd);
        return 0;
    } else {
        return errno;
    }
}

int
set_screen_state(int on)
{
    QEMU_FALLBACK(set_screen_state(on));

    LOGI("*** set_screen_state %d", on);

    initialize_fds();

    //LOGI("go_to_sleep eventTime=%lld now=%lld g_error=%s\n", eventTime,
      //      systemTime(), strerror(g_error));

    if (g_error)
        goto failure;

    char buf[32];
    int len;
    if(on)
        len = snprintf(buf, sizeof(buf), "%s", on_state);
    else
        len = snprintf(buf, sizeof(buf), "%s", off_state);

    buf[sizeof(buf) - 1] = '\0';
    len = write(g_fds[REQUEST_STATE], buf, len);
    if(len < 0) {
    failure:
        LOGE("Failed setting last user activity: g_error=%d\n", g_error);
    }
    return 0;
}

#ifdef QCOM_HARDWARE
/************************************
 If 'state' equals 1
    1. physical hotplug unstable memory
    2. logical hotplug unstable memory

 If 'state' equals 0
    1. logical remove unstable memory
    2. physical remove unstable memory
************************************/

static int
logical(int state) {
    int fd_State=0;
    char str_movable_start_bytes[PROPERTY_VALUE_MAX], strBlock[32]="0";
    char pDirName[128]="/sys/devices/system/memory/memory", fDirName[128], strState[7]="/state";

    if(property_get("ro.dev.dmm.dpd.block", strBlock, "0") <= 0) {
        LOGE("Failed to property_get() block number:%s\n",strBlock);
        return -1;
    }
    else
        if (DMMDEBUG) LOGW("strBlock = %s\n", strBlock);

    sprintf(fDirName, "%s%s%s", pDirName, strBlock, strState);

    if(DMMDEBUG) LOGW("Directory Location = %s\n", fDirName);

    if((fd_State=open(fDirName, O_RDWR)) < 0) {
        LOGE("Failed to open %s: %d", fDirName, -errno);
        return -errno;
    }

    if(state == 0) {
        if(write(fd_State, "offline", strlen("offline")) == -1) {
            LOGE("Logical Remove of Unstable Memory: Failed (-%d)", errno);
            close(fd_State);
            return -errno;
        }
        if(DMMDEBUG) LOGW("Logical Remove of Unstable Memory: Succeded !");
    }
    else {
        if(write(fd_State, "online", strlen("online")) == -1) {
            LOGE("Logical Hotplug of Unstable Memory: Failed (-%d)", errno);
            close(fd_State);
            return -errno;
        }
        if(DMMDEBUG) LOGW("Logical Hotplug of Unstable Memory: Succeded !");
    }
    close(fd_State);
    return 0;
}

static int
physical(int state) {
    int fd_Physical=0;
    char str_movable_start_bytes[PROPERTY_VALUE_MAX];
    char probePath[128]="/sys/devices/system/memory/probe";
    char activePath[128]="/sys/devices/system/memory/active";
    char removePath[128]="/sys/devices/system/memory/remove";

    if(property_get("ro.dev.dmm.dpd.start_address", str_movable_start_bytes, "0") <= 0) {
        LOGE("Failed to property_get() movable start bytes:%s\n",str_movable_start_bytes);
        return -1;
    }
    else
        if(DMMDEBUG) LOGW("str_movable_start_bytes = %s\n", str_movable_start_bytes);

    if(state == 0) {
        if((fd_Physical=open(removePath, O_WRONLY)) < 0) {
            LOGE("Failed to open %s: %d", removePath, -errno);
            return -errno;
        }

        if(write(fd_Physical, str_movable_start_bytes, strlen(str_movable_start_bytes)) == -1) {
            LOGE("Physical Remove of Unstable Memory: Failed (-%d)", errno);
            LOGE("Writing %s to %s: Failed !", str_movable_start_bytes, removePath);
            close(fd_Physical);
            return -errno;
        }
        if(DMMDEBUG) LOGW("Physical Remove of Unstable Memory: Succeded !");
    }
    else {
        if((fd_Physical=open(probePath, O_WRONLY)) < 0) {
            LOGE("Failed to open %s: %d", probePath, -errno);
            return -errno;
        }

        if(write(fd_Physical, str_movable_start_bytes, strlen(str_movable_start_bytes)) == -1) {
            LOGE("Physical HotPlug (Probe) of Unstable Memory: Failed (-%d)", errno);
            LOGE("Writing %s to %s: Failed !", str_movable_start_bytes, probePath);
            close(fd_Physical);
            return -errno;
        }

        if((fd_Physical=open(activePath, O_WRONLY)) < 0) {
            LOGE("Failed to open %s: %d", activePath, -errno);
            return -errno;
        }

        if(write(fd_Physical, str_movable_start_bytes, strlen(str_movable_start_bytes)) == -1) {
            LOGE("Physical HotPlug (Active) of Unstable Memory: Failed (-%d)", errno);
            LOGE("Writing %s to %s: Failed !", str_movable_start_bytes, activePath);
            close(fd_Physical);
            return -errno;
        }
        if(DMMDEBUG) LOGW("Physical HotPlug of Unstable Memory: Succeded !");
    }
    close(fd_Physical);
    return 0;
}

int
set_unstable_memory_state(int state) {
    LOGW("UnstableMemory(%d)", state);

    if(state == 0) {
        if(logical(0) != 0) {
            return -1;
        }
        if(physical(0) != 0) {
            logical(1);
            return -1;
        }
    }
    else {
        if(physical(1) != 0) {
            return -1;
        }
        if(logical(1) != 0) {
            physical(0);
            return -1;
        }
    }
    return 0;
}
#endif
