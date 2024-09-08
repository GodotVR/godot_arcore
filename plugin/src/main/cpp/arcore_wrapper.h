//
// Created by luca on 20.08.24.
//

#ifndef ARCOREPLUGIN_ARCORE_WRAPPER_H
#define ARCOREPLUGIN_ARCORE_WRAPPER_H

#include "utils.h"

class ARCoreWrapper {
public:
    static void initialize_environment(JNIEnv *env, jobject activity);
    static void uninitialize_environment(JNIEnv *env);
private:
    static JNIEnv *env;
    static jobject arcore_plugin_instance;
    static jobject godot_instance;
    static jobject activity;
    static jclass godot_class;
    static jclass activity_class;
public:
    ARCoreWrapper();
    ~ARCoreWrapper();

    static JNIEnv* get_env();
    static jobject get_godot_class();
    static jobject get_activity();
    static jobject get_global_context();
};

#endif //ARCOREPLUGIN_ARCORE_WRAPPER_H
