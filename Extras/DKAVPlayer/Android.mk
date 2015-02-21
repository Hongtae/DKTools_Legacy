DKAVPLAYER_ROOT := $(call my-dir)

#include ${DKAVPLAYER_ROOT}/../../DK/Android.mk
include ${DKAVPLAYER_ROOT}/FFmpeg/Android.mk

LOCAL_PATH := $(DKAVPLAYER_ROOT)
include $(CLEAR_VARS)

LOCAL_MODULE := DKAVPlayer
LOCAL_CPPFLAGS := -std=c++11 -fvisibility=hidden -DNDEBUG=1 -D_NDEBUG=1
LOCAL_CPPFLAGS += -Wno-multichar -Wno-enum-compare
LOCAL_CPPFLAGS += -D__STDC_CONSTANT_MACROS
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_STATIC_LIBRARIES := DK
LOCAL_SHARED_LIBRARIES := FFmpeg

LOCAL_C_INCLUDES := \
	${LOCAL_PATH}/../../DK/include \
	${LOCAL_PATH}/FFmpeg/include

LOCAL_SRC_FILES := \
	DKAVMediaBuffer.cpp \
	DKAVMediaPlayer.cpp \
	DKAVMediaRenderer.cpp \
	DKAVMediaStream.cpp

include $(BUILD_STATIC_LIBRARY)
