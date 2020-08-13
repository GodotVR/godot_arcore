/**
* Created by Fredia Huya-Kouadio.
*/

#include <core/GodotGlobal.hpp>
#include <gen/ARVRServer.hpp>
#include <arcore_interface.h>
#include <core/Godot.hpp>
#include <common.h>
#include "godot_arcore.h"

void GDN_EXPORT godot_arcore_gdnative_init(godot_gdnative_init_options *options) {
    godot::Godot::gdnative_init(options);
    ALOG_ASSERT(godot::android_api->version.major >= 1 && godot::android_api->version.minor >= 2,
                "GDNative Android API version should be >= 1.2");
}

void GDN_EXPORT godot_arcore_gdnative_singleton() {
  godot::ARVRServer::get_singleton()->add_interface(ARCoreInterface::get_singleton_instance());
}

void GDN_EXPORT godot_arcore_nativescript_init(void *handle) {
  godot::Godot::nativescript_init(handle);

  godot::register_class<ARCoreInterface>();
}

void GDN_EXPORT godot_arcore_nativescript_terminate(void *handle) {
  godot::Godot::nativescript_terminate(handle);
}

void GDN_EXPORT godot_arcore_gdnative_terminate(godot_gdnative_terminate_options *options) {
  godot::Godot::gdnative_terminate(options);
}

////////////////////////////////////////////////////////////////////////
// !BAS! Implementing this here for now so we can get it to work
// but this should move into godot-cpp at some point

void *godot_arvr_constructor(godot_object *p_instance) {
	// we need to return the pointer to the wrapper object for our instance here

	ARCoreInterface *data = godot::detail::get_wrapper<ARCoreInterface>(p_instance);
	return data;
}

void godot_arvr_destructor(void *p_data) {
	// we can ignore this here

}

godot_string godot_arvr_get_name(const void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	godot::String arvr_name = data->get_name();
	godot_string ret_name;
	api->godot_string_new_copy(&ret_name, (godot_string *)&arvr_name);

	return ret_name;
}

godot_int godot_arvr_get_capabilities(const void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	return (godot_int) data->get_capabilities();
};

godot_bool godot_arvr_get_anchor_detection_is_enabled(const void *p_data) {
	godot_bool ret;

	ret = false; // does not apply here

	return ret;
};

void godot_arvr_set_anchor_detection_is_enabled(void *p_data, bool p_enable){
	// we ignore this, not supported in this interface!
};

godot_bool godot_arvr_is_stereo(const void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	return (godot_bool) data->is_stereo();
};

godot_bool godot_arvr_is_initialized(const void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	return (godot_bool) data->is_initialized();
};

godot_bool godot_arvr_initialize(void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	return (godot_bool) data->initialize();
};

void godot_arvr_uninitialize(void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	data->uninitialize();
};

godot_vector2 godot_arvr_get_render_targetsize(const void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	Size2 arvr_size = data->get_render_targetsize();

	godot_vector2 ret_size;
	api->godot_vector2_new(&ret_size, arvr_size.x, arvr_size.y);

	return ret_size;
};

godot_transform godot_arvr_get_transform_for_eye(void *p_data, godot_int p_eye, godot_transform *p_cam_transform) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	godot::Transform *cam_transform = (godot::Transform *)&p_cam_transform;
	godot::Transform transform = data->get_transform_for_eye((ARVRInterface::Eyes) p_eye, *cam_transform);

	godot_transform *ret_transform = (godot_transform*)&transform;
	return *ret_transform;
};

void godot_arvr_fill_projection_for_eye(void *p_data, godot_real *p_projection, godot_int p_eye, godot_real p_aspect, godot_real p_z_near, godot_real p_z_far) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	CameraMatrix cm = data->get_projection_for_eye((ARVRInterface::Eyes) p_eye, p_aspect, p_z_near, p_z_far);
	memcpy(p_projection, &cm, sizeof(godot_real[4][4]));
};

void godot_arvr_commit_for_eye(void *p_data, godot_int p_eye, godot_rid *p_render_target, godot_rect2 *p_screen_rect) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	godot::RID *rid = (godot::RID *) p_render_target;
	godot::Rect2 *rect = (godot::Rect2 *) p_screen_rect;
	data->commit_for_eye((ARVRInterface::Eyes) p_eye, *rid, *rect);
};

void godot_arvr_process(void *p_data) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	data->process();
};

int godot_arvr_get_external_texture_for_eye(void *p_data, int p_eye) {
	// ARCoreInterface *data = (ARCoreInterface *)p_data;

	// !BAS! not exposed ?! should probably be...

	return 0;
};

void godot_arvr_notification(void *p_data, int p_what) {
	ARCoreInterface *data = (ARCoreInterface *)p_data;

	data->notification(p_what);
};

const godot_arvr_interface_gdnative arvr_interface_struct = {
	{ GODOTVR_API_MAJOR, GODOTVR_API_MINOR }, godot_arvr_constructor,
	godot_arvr_destructor, godot_arvr_get_name, godot_arvr_get_capabilities,
	godot_arvr_get_anchor_detection_is_enabled,
	godot_arvr_set_anchor_detection_is_enabled, godot_arvr_is_stereo,
	godot_arvr_is_initialized, godot_arvr_initialize, godot_arvr_uninitialize,
	godot_arvr_get_render_targetsize, godot_arvr_get_transform_for_eye,
	godot_arvr_fill_projection_for_eye, godot_arvr_commit_for_eye,
	godot_arvr_process,
	// only available in Godot 3.2+
	godot_arvr_get_external_texture_for_eye, godot_arvr_notification
};
