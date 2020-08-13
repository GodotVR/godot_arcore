/*************************************************************************/
/*  godot_arcore.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/**
* Created by Fredia Huya-Kouadio.
*/

#include <core/GodotGlobal.hpp>
#include <gen/ARVRServer.hpp>
#include <arcore_interface.h>
#include <core/Godot.hpp>
#include <common.h>
#include "godot_arcore.h"

const godot_gdnative_ext_arvr_1_2_api_struct * arvr12_api = nullptr;
const godot_gdnative_ext_android_1_2_api_struct * android12_api = nullptr;

void GDN_EXPORT godot_arcore_gdnative_init(godot_gdnative_init_options *options) {
	ALOGV("godot_arcore_gdnative_init");
	godot::Godot::gdnative_init(options);

	// our additions are in a follow up API, this needs to move into godot-cpp with us just checking whether 1.2 API pointers are set
	ALOG_ASSERT(godot::android_api != nullptr, "GDNative Android API missing");
	ALOG_ASSERT(godot::android_api->next != nullptr, "GDNative Android 1.2 API missing");
	android12_api = (const godot_gdnative_ext_android_1_2_api_struct *)godot::android_api->next;
	ALOG_ASSERT(android12_api->type == GDNATIVE_EXT_ANDROID && android12_api->version.major == 1 && android12_api->version.minor == 2, "GDNative Android API version should be >= 1.2");

	ALOG_ASSERT(godot::arvr_api != nullptr, "GDNative ARVR API missing");
	ALOG_ASSERT(godot::arvr_api->next != nullptr, "GDNative ARVR 1.2 API missing");
	arvr12_api = (const godot_gdnative_ext_arvr_1_2_api_struct *)godot::arvr_api->next;
	ALOG_ASSERT(arvr12_api->type == GDNATIVE_EXT_ARVR && arvr12_api->version.major == 1 && arvr12_api->version.minor == 2, "GDNative ARVR API version should be >= 1.2");
}

void GDN_EXPORT godot_arcore_gdnative_singleton() {
	ALOGV("godot_arcore_gdnative_singleton");

	// This currently doesn't work because we can't access our ARVR server singleton yet.
	// Our ARVRServer exists, but `get_singleton` isn't actually calling that method but
	// is obtaining the singleton from a registry, and this registry hasn't had the server
	// added to it yet.
	if (android12_api != nullptr && arvr12_api != nullptr) {
		godot::ARVRServer *arvr_server = godot::ARVRServer::get_singleton();
		ALOG_ASSERT(arvr_server != nullptr, "Can't access the ARVR server");

		Ref<ARCoreInterface> arcore_interface = ARCoreInterface::get_singleton_reference();
		ALOG_ASSERT(arcore_interface.is_valid(), "Couldn't instantiate ARCORE Interface");
		arvr_server->add_interface(arcore_interface);
	} else {
		godot::Godot::print("Missing Godot APIs for ARCore");
	}
}

void GDN_EXPORT godot_arcore_nativescript_init(void *handle) {
	ALOGV("godot_arcore_nativescript_init");

	godot::Godot::nativescript_init(handle);

	if (android12_api != nullptr && arvr12_api != nullptr) {
		godot::register_class<ARCoreInterface>();
	}
}

void GDN_EXPORT godot_arcore_nativescript_terminate(void *handle) {
	ALOGV("godot_arcore_nativescript_terminate");
	ARCoreInterface::delete_singleton_reference();
	godot::Godot::nativescript_terminate(handle);
}

void GDN_EXPORT godot_arcore_gdnative_terminate(godot_gdnative_terminate_options *options) {
	ALOGV("godot_arcore_gdnative_terminate");
	godot::Godot::gdnative_terminate(options);
}

////////////////////////////////////////////////////////////////////////
// !BAS! Implementing this here for now so we can get it to work
// but this should move into godot-cpp at some point

void *godot_arvr_constructor(godot_object *p_instance) {
	// we need to return the pointer to the wrapper object for our instance here

	// !BAS! This crashes and it's probably because we don't store the binding until after this is called!
	// ARCoreInterface *data = godot::detail::get_wrapper<ARCoreInterface>(p_instance);

	ARCoreInterface *data = ARCoreInterface::get_singleton_instance();
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

	godot::Transform *cam_transform = (godot::Transform *)p_cam_transform;
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
