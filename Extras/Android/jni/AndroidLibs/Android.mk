ANDROID_LIB_PATH := $(call my-dir)

include ${ANDROID_LIB_PATH}/uuid/Android_Static.mk

LOCAL_PATH := $(ANDROID_LIB_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := AndroidLibs
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_STATIC_LIBRARIES := AndroidUUID
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_SRC_FILES := rand_r.c fileio.c
include $(BUILD_STATIC_LIBRARY)
