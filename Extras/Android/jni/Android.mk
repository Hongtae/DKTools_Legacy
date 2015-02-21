DKLIB_JNI_PATH := $(call my-dir)

DKLIB_ROOT := $(DKLIB_JNI_PATH)/../../..
DK_PATH := $(DKLIB_ROOT)/DK
PYDK_PATH := $(DKLIB_ROOT)/PyDK
PYTHON_ROOT := $(DKLIB_JNI_PATH)/Python

include $(DKLIB_JNI_PATH)/AndroidLibs/Android.mk
include $(DK_PATH)/Android.mk
include $(PYTHON_ROOT)/Android.mk
include $(PYDK_PATH)/Android.mk

LOCAL_PATH := $(DKLIB_JNI_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := PyDKAndroid

LOCAL_CPPFLAGS := -std=c++11 -fvisibility=hidden -mfpu=neon
LOCAL_CPPFLAGS += -DNDEBUG=1 -D_NDEBUG=1 -DDK_STATIC=1 -DPYDK_STATIC=1
LOCAL_CPPFLAGS += -Wno-multichar -Wno-enum-compare
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_STATIC_LIBRARIES := DK
LOCAL_STATIC_LIBRARIES += PyDK
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

LOCAL_C_INCLUDES := \
	$(DK_PATH) \
	$(PYDK_PATH)/src

LOCAL_SRC_FILES := \
	PyDKAndroid/JNIOperationQueue.cpp \
	PyDKAndroid/DKApplicationImpl.cpp \
	PyDKAndroid/DKOpenGLImpl.cpp \
	PyDKAndroid/DKWindowImpl.cpp \
	PyDKAndroid/PyDKAndroid.cpp

include $(BUILD_SHARED_LIBRARY)
