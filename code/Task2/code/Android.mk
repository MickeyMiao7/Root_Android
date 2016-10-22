LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := compiled_binary
LOCAL_SRC_FILES := wrapper.c
include $(BUILD_EXECUTABLE) 
