# Copyright 2006 The Android Open Source Project

# Setting LOCAL_PATH will mess up all-subdir-makefiles, so do it beforehand.
legacy_modules := power uevent wifi qemu qemu_tracing

SAVE_MAKEFILES := $(call all-named-subdir-makefiles,$(legacy_modules))
LEGACY_AUDIO_MAKEFILES := $(call all-named-subdir-makefiles,audio)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libcutils liblog

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_CFLAGS  += -DQEMU_HARDWARE -Wno-unused-parameter -Wno-gnu-designator

ifeq ($(KERNEL_HAS_FINIT_MODULE), false)
LOCAL_CFLAGS += -DNO_FINIT_MODULE
endif

ifdef USES_TI_MAC80211
ifneq ($(wildcard external/libnl),)
LOCAL_SHARED_LIBRARIES += libnl
LOCAL_C_INCLUDES += external/libnl/include
else
LOCAL_STATIC_LIBRARIES := libnl_2
LOCAL_C_INCLUDES += external/libnl-headers
endif
endif

QEMU_HARDWARE := true

LOCAL_SHARED_LIBRARIES += libdl

include $(SAVE_MAKEFILES)

LOCAL_MODULE:= libhardware_legacy

include $(BUILD_SHARED_LIBRARY)

# static library for librpc
include $(CLEAR_VARS)

LOCAL_MODULE:= libpower

LOCAL_SRC_FILES += power/power.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)

# shared library for various HALs
include $(CLEAR_VARS)

LOCAL_MODULE := libpower

LOCAL_SRC_FILES := power/power.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_SHARED_LIBRARY)

# legacy_audio builds it's own set of libraries that aren't linked into
# hardware_legacy
include $(LEGACY_AUDIO_MAKEFILES)
