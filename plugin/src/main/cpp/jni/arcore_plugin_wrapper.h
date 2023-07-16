#ifndef ARCORE_PLUGIN_WRAPPER_H
#define ARCORE_PLUGIN_WRAPPER_H

#include <jni.h>

class ARCorePluginWrapper {
public:
    static void initialize_wrapper(JNIEnv *env, jobject activity, jobject p_godot_instance, jobject arcore_plugin);

    static void uninitialize_wrapper(JNIEnv *env);

private:
    static jobject arcore_plugin_instance;
    static jobject godot_instance;
    static jobject activity;
    static jclass godot_class;
    static jclass activity_class;

    static jmethodID _get_surface;
    static jmethodID _is_activity_resumed;
    static jmethodID _get_display_rotation;

public:
    ARCorePluginWrapper();
    ~ARCorePluginWrapper();

    jobject get_activity();
    jobject get_surface(JNIEnv *p_env);
    bool is_activity_resumed(JNIEnv *p_env);
    int get_display_rotation(JNIEnv *p_env);
};

#endif // ARCORE_PLUGIN_WRAPPER_H
