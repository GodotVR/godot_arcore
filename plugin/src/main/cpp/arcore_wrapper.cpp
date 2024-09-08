//
// Created by luca on 20.08.24.
//

#include "arcore_wrapper.h"
#include "utils.h"

JNIEnv *ARCoreWrapper::env = nullptr;
// Is this used?
jobject ARCoreWrapper::arcore_plugin_instance = nullptr;
jobject ARCoreWrapper::godot_instance = nullptr;
jobject ARCoreWrapper::activity = nullptr;
jclass ARCoreWrapper::godot_class = nullptr;
jclass ARCoreWrapper::activity_class = nullptr;

ARCoreWrapper::ARCoreWrapper() {}

ARCoreWrapper::~ARCoreWrapper() {}

void ARCoreWrapper::initialize_environment(JNIEnv *p_env, jobject p_activity) {
	env = p_env;

	activity = p_env->NewGlobalRef(p_activity);

	// Get info about our Godot class to get pointers
	godot_class = p_env->FindClass("org/godotengine/godot/Godot");

	if (godot_class) {
		godot_class = (jclass)p_env->NewGlobalRef(godot_class);
	} else {
		ALOGE("ARCorePlugin: Can't find org/godotengine/godot/Godot");
		return;
	}

	activity_class = p_env->FindClass("android/app/Activity");

	if (activity_class) {
		activity_class = (jclass)p_env->NewGlobalRef(activity_class);
	} else {
		ALOGE("ARCorePlugin: Can't find android/app/Activity");
		return;
	}
}

void ARCoreWrapper::uninitialize_environment(JNIEnv *env) {
	if (arcore_plugin_instance) {
		ALOGV("ARCorePlugin: ARCore instance found.");
		env->DeleteGlobalRef(arcore_plugin_instance);

		arcore_plugin_instance = nullptr;

		env->DeleteGlobalRef(godot_instance);
		env->DeleteGlobalRef(godot_class);
		env->DeleteGlobalRef(activity);
		env->DeleteGlobalRef(activity_class);
	}
}

JNIEnv *ARCoreWrapper::get_env() {
	return env;
}

jobject ARCoreWrapper::get_godot_class() {
	return godot_class;
}

jobject ARCoreWrapper::get_activity() {
	return activity;
}

jobject ARCoreWrapper::get_global_context() {
	jclass activityThread = env->FindClass("android/app/ActivityThread");
	jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
	jobject activityThreadObj = env->CallStaticObjectMethod(activityThread, currentActivityThread);

	jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
	jobject context = env->CallObjectMethod(activityThreadObj, getApplication);
	return context;
}
