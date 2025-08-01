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

	static int display_orientation;
	static int display_width;
	static int display_height;
	static bool viewport_changed;

public:
	ARCoreWrapper();
	~ARCoreWrapper();

	static JNIEnv *get_env();
	static jobject get_godot_class();
	static jobject get_activity();
	static jobject get_global_context();

	static int get_display_orientation();
	static int get_display_width();
	static int get_display_height();
	static bool has_viewport_changed();
	static void set_viewport_changed(bool changed);

	static void on_display_geometry_changed(int orientation, int width, int height);
};

#endif //ARCOREPLUGIN_ARCORE_WRAPPER_H
