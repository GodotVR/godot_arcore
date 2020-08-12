/**
* Created by Fredia Huya-Kouadio.
*/

#ifndef COMMON_H
#define COMMON_H

#include <android/log.h>

#define DEBUG 1
#define LOG_TAG "GodotARCore"

#define ALOG_ASSERT(_cond, ...) \
	if (!(_cond)) __android_log_assert("conditional", LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

#endif // COMMON_H
