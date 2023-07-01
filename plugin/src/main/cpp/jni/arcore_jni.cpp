#include <jni.h>

#include "jni_util.h"
#include "arcore_plugin_wrapper.h"

#undef JNI_PACKAGE_NAME
#define JNI_PACKAGE_NAME org_godotengine_plugin_arcore

#undef JNI_CLASS_NAME
#define JNI_CLASS_NAME ARCorePlugin

extern "C" {
    JNIEXPORT void JNICALL JNI_METHOD(initializeWrapper)(JNIEnv *env, jobject object) {
        ARCorePluginWrapper::initialize_wrapper(env, object);
    }

    JNIEXPORT void JNICALL JNI_METHOD(uninitializeWrapper)(JNIEnv *env, jobject) {
        ARCorePluginWrapper::uninitialize_wrapper(env);
    }
}
