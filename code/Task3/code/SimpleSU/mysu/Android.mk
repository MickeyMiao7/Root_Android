LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := mysu
LOCAL_SRC_FILES := ./mysu.c ../socket_util/socket_util.c
include $(BUILD_EXECUTABLE)
