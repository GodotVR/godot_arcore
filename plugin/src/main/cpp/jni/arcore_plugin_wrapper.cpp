#include "arcore_plugin_wrapper.h"

#include "../android_util.h"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/os.hpp>

jobject ARCorePluginWrapper::arcore_plugin_instance = nullptr;

ARCorePluginWrapper::ARCorePluginWrapper() {
    _get_surface = nullptr;
    _is_activity_resumed = nullptr;
    _get_display_rotation = nullptr;
}

ARCorePluginWrapper::~ARCorePluginWrapper() {}

void ARCorePluginWrapper::initialize_wrapper(JNIEnv *p_env, jobject p_activity, jobject p_godot_instance, jobject p_arcore_plugin) {
    arcore_plugin_instance = p_env->NewGlobalRef(p_arcore_plugin);
    ALOG_ASSERT(arcore_plugin_instance, "Invalid jobject value.");

    jclass arcore_plugin_class = p_env->GetObjectClass(arcore_plugin_instance);
    ALOG_ASSERT(arcore_plugin_class, "Invalid jclass value.");

    godot_instance = p_env->NewGlobalRef(p_godot_instance);
    activity = p_env->NewGlobalRef(p_activity);

    // get info about our Godot class so we can get pointers and stuff...
    godot_class = p_env->FindClass("org/godotengine/godot/Godot");
    if (godot_class) {
        godot_class = (jclass)p_env->NewGlobalRef(godot_class);
    } else {
        // this is a pretty serious fail.. bail... pointers will stay 0
        return;
    }
    activity_class = p_env->FindClass("android/app/Activity");
    if (activity_class) {
        activity_class = (jclass)p_env->NewGlobalRef(activity_class);
    } else {
        // this is a pretty serious fail.. bail... pointers will stay 0
        return;
    }

    // get some Godot method pointers...
    _get_surface = p_env->GetMethodID(godot_class, "getSurface", "()Landroid/view/Surface;");
    _is_activity_resumed = p_env->GetMethodID(godot_class, "isActivityResumed", "()Z");
    _get_display_rotation = p_env->GetMethodID(godot_class, "getDisplayRotation", "()I");
}

void ARCorePluginWrapper::uninitialize_wrapper(JNIEnv *env) {
    if (arcore_plugin_instance) {
        env->DeleteGlobalRef(arcore_plugin_instance);

        arcore_plugin_instance = nullptr;

        env->DeleteGlobalRef(godot_instance);
        env->DeleteGlobalRef(godot_class);
        env->DeleteGlobalRef(activity);
        env->DeleteGlobalRef(activity_class);
    }
}

jobject ARCorePluginWrapper::get_activity() {
    return activity;
}

jobject ARCorePluginWrapper::get_surface(JNIEnv *p_env) {
    if (_get_surface) {
        return p_env->CallObjectMethod(godot_instance, _get_surface);
    } else {
        return nullptr;
    }
}

bool ARCorePluginWrapper::is_activity_resumed(JNIEnv *p_env) {
    if (_is_activity_resumed) {
        return p_env->CallBooleanMethod(godot_instance, _is_activity_resumed);
    } else {
        return false;
    }
}

int ARCorePluginWrapper::get_display_rotation(JNIEnv *p_env) {
    if (_get_display_rotation) {
        return p_env->CallIntMethod(godot_instance, _get_display_rotation);
    } else {
        return 0;
    }
}
