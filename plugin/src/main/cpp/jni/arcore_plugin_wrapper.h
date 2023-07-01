#ifndef ARCORE_PLUGIN_WRAPPER_H
#define ARCORE_PLUGIN_WRAPPER_H

#include <jni.h>

class ARCorePluginWrapper {
public:
    static void initialize_wrapper(JNIEnv *env, jobject arcore_plugin);

    static void uninitialize_wrapper(JNIEnv *env);


private:
    ARCorePluginWrapper();
    ~ARCorePluginWrapper();

    static jobject arcore_plugin_instance;
};

#endif // ARCORE_PLUGIN_WRAPPER_H
