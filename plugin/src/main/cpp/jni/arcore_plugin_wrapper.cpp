#include "arcore_plugin_wrapper.h"

#include "../android_util.h"
#include <godot_cpp/godot.hpp>

jobject ARCorePluginWrapper::arcore_plugin_instance = nullptr;

ARCorePluginWrapper::ARCorePluginWrapper() {}

ARCorePluginWrapper::~ARCorePluginWrapper() {}

void ARCorePluginWrapper::initialize_wrapper(JNIEnv *env, jobject arcore_plugin) {
    arcore_plugin_instance = env->NewGlobalRef(arcore_plugin);
    ALOG_ASSERT(arcore_plugin_instance, "Invalid jobject value.");

    jclass arcore_plugin_class = env->GetObjectClass(arcore_plugin_instance);
    ALOG_ASSERT(arcore_plugin_class, "Invalid jclass value.");
}

void ARCorePluginWrapper::uninitialize_wrapper(JNIEnv *env) {
    if (arcore_plugin_instance) {
        env->DeleteGlobalRef(arcore_plugin_instance);

        arcore_plugin_instance = nullptr;
    }
}
