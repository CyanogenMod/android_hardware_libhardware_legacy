# Copyright 2006 The Android Open Source Project
ifeq ($(BOARD_HAVE_SQN_WIMAX),true)

LOCAL_SRC_FILES += wimax/wimax.c

LOCAL_SHARED_LIBRARIES += libnetutils

endif
