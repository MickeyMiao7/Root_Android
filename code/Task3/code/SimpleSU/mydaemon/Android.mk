LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := mydaemon
LOCAL_SRC_FILES := mydaemonsu.c ../socket_util/socket_util.c
include $(BUILD_EXECUTABLE)
