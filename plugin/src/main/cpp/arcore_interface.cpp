/*************************************************************************/
/*  arcore_interface.cpp                                                 */
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

#include "arcore_interface.h"

#include <core/Godot.hpp>
#include <gen/MainLoop.hpp>
#include <gen/CameraServer.hpp>
#include <gen/OS.hpp>

// TODO remove these temporary includes once we add our GL_TEXTURE_EXTERNAL_OES support to our drivers
#ifndef GLES3_INCLUDE_H
#include <GLES3/gl3.h>
#include <gen/VisualServer.hpp>
#else
#include GLES3_INCLUDE_H
#endif

/**
	@author Bastiaan Olij <mux213@gmail.com>, Robert Hofstra <robert.hofstra@knowlogy.nl>, Fredia Huya-Kouadio <fhuyakou@gmail.com>
	ARCore interface between Android and Godot
**/

ARCoreInterface* ARCoreInterface::singleton_instance = nullptr;

ARCoreInterface* ARCoreInterface::get_singleton_instance() {
	if (singleton_instance == nullptr) {
		singleton_instance = new ARCoreInterface();
	}
	return singleton_instance;
}

void ARCoreInterface::delete_singleton_instance() {
	delete singleton_instance;
	singleton_instance = nullptr;
}

String ARCoreInterface::get_name() const {
	return "ARCore";
}

int ARCoreInterface::get_capabilities() const {
	return ARVRInterface::ARVR_MONO + ARVRInterface::ARVR_AR;
}

int ARCoreInterface::get_camera_feed_id() {
	if (feed.is_valid()) {
		return feed->get_id();
	} else {
		return 0;
	}
}

void ARCoreInterface::_register_methods() {
	register_method("_resume", &ARCoreInterface::_resume);
	register_method("_pause", &ARCoreInterface::_pause);
}

bool ARCoreInterface::is_initialized() const {
	// if we're in the process of initialising we treat this as initialised...
	return (init_status != NOT_INITIALISED) && (init_status != INITIALISE_FAILED);
}

ARVRInterface::Tracking_status ARCoreInterface::get_tracking_status() const {
	return tracking_state;
}

void ARCoreInterface::_resume() {
	if (init_status == INITIALISED && ar_session != nullptr) {
		ArStatus status = ArSession_resume(ar_session);
		if (status != AR_SUCCESS) {
			Godot::print("Godot ARCore: Failed to resume.");

			// TODO quit? how?
		}
	}
}

void ARCoreInterface::_pause() {
	if (ar_session != nullptr) {
		ArSession_pause(ar_session);
	}
}

void ARCoreInterface::notification(int p_what) {
	// Needs testing, this should now be called

	ARVRServer *arvr_server = ARVRServer::get_singleton();
	ERR_FAIL_NULL(arvr_server);

	switch (p_what) {
		case MainLoop::NOTIFICATION_APP_RESUMED: {
			if (is_initialized()) {
				_resume();

				if (init_status == INITIALISE_FAILED) {
					arvr_server->clear_primary_interface_if(this);
				}
			}
		}; break;
		case MainLoop::NOTIFICATION_APP_PAUSED:
			if (is_initialized()) {
				_pause();
			}
			break;
		default:
			break;
	}
}

bool ARCoreInterface::initialize() {
	// TODO we may want to check for status PAUZED and just call resume (if we decide to implement that)

	ARVRServer *arvr_server = ARVRServer::get_singleton();
	ERR_FAIL_NULL_V(arvr_server, false);

	if (init_status == INITIALISE_FAILED) {
		// if we fully failed last time, don't try again..
		return false;
	} else if (init_status == NOT_INITIALISED) {
		Godot::print("Godot ARCore: Initialising...");
		init_status = START_INITIALISE;

		// create our camera feed
		if (feed.is_null()) {
			Godot::print("Godot ARCore: Creating camera feed...");

			feed.instance();
			feed->_set_name("ARCore");
			feed->set_active(true);

			CameraServer *cs = CameraServer::get_singleton();
			if (cs != nullptr) {
				cs->add_feed(feed);

				Godot::print("Godot ARCore: Feed " + String::num_int64(feed->get_id()) + " added");
			}
		}

		if (ar_session == nullptr) {

			Godot::print("Godot ARCore: Getting environment");

			// get some android things
			JNIEnv *env = godot::android_api->godot_android_get_env();

			jobject context = godot::android_api->godot_android_get_activity();
			if (context == nullptr) {
				Godot::print("Godot ARCore: Couldn't get context");
				init_status = INITIALISE_FAILED; // don't try again.
				return false;
			}

			Godot::print("Godot ARCore: Create ArSession");

			if (ArSession_create(env, context, &ar_session) != AR_SUCCESS || ar_session == nullptr) {
				Godot::print("Godot ARCore: ARCore couldn't be created.");
				init_status = INITIALISE_FAILED; // don't try again.
				return false;
			}

			Godot::print("Godot ARCore: Create ArFrame.");

			ArFrame_create(ar_session, &ar_frame);
			if (ar_frame == nullptr) {
				Godot::print("Godot ARCore: Frame couldn't be created.");

				ArSession_destroy(ar_session);
				ar_session = nullptr;

				init_status = INITIALISE_FAILED; // don't try again.
				return false;
			}

			// Get our size, make sure we have these in portrait
			Size2 size = OS::get_singleton()->get_window_size();
			if (size.x > size.y) {
				width = size.y;
				height = size.x;
			} else {
				width = size.x;
				height = size.y;
			}

			// Trigger display rotation
			display_rotation = -1;

			Godot::print("Godot ARCore: Initialised.");
			init_status = INITIALISED;
		}

		// and call resume for the first time to complete this
		_resume();

		if (init_status != INITIALISE_FAILED) {
			// make this our primary interface
			arvr_server->set_primary_interface(this);

			// make sure our feed is marked as active if we already have one...
			if (feed != nullptr) {
				feed->set_active(true);
			}
		}
	}

	return is_initialized();
}

void ARCoreInterface::uninitialize() {
	if (is_initialized()) {
		// TODO we may want to call ArSession_pauze here and introduce a new status PAUZED
		// then move cleanup to our destruct.

		make_anchors_stale();
		remove_stale_anchors();

		if (ar_session != nullptr) {
			ArSession_destroy(ar_session);
			ArFrame_destroy(ar_frame);

			ar_session = nullptr;
			ar_frame = nullptr;
		}

		if (feed.is_valid()) {
			feed->set_active(false);

			CameraServer *cs = CameraServer::get_singleton();
			if (cs != nullptr) {
				cs->remove_feed(feed);
			}
			feed.unref();
			camera_texture_id = 0;
		}

		init_status = NOT_INITIALISED;
	}
}

Size2 ARCoreInterface::get_render_targetsize() {
	Size2 target_size = OS::get_singleton()->get_window_size();
	return target_size;
}

bool ARCoreInterface::is_stereo() {
	return false;
}

Transform ARCoreInterface::get_transform_for_eye(ARVRInterface::Eyes p_eye, const Transform &p_cam_transform) {
	Transform transform_for_eye;

	ARVRServer *arvr_server = ARVRServer::get_singleton();
	ERR_FAIL_NULL_V(arvr_server, transform_for_eye);

	if (init_status == INITIALISED) {
		float world_scale = arvr_server->get_world_scale();

		// just scale our origin point of our transform, note that we really shouldn't be using world_scale in ARKit but....
		transform_for_eye = view;
		transform_for_eye.origin *= world_scale;

		transform_for_eye = p_cam_transform * (arvr_server->get_reference_frame()) * transform_for_eye;
	} else {
		// huh? well just return what we got....
		transform_for_eye = p_cam_transform;
	}

	return transform_for_eye;
}

CameraMatrix ARCoreInterface::get_projection_for_eye(ARVRInterface::Eyes p_eye, real_t p_aspect, real_t p_z_near, real_t p_z_far) {
	// Remember our near and far, we'll use it next frame
	z_near = p_z_near;
	z_far = p_z_far;

	return projection;
}

void ARCoreInterface::commit_for_eye(ARVRInterface::Eyes p_eye, RID p_render_target, const Rect2 &p_screen_rect) {
	// We must have a valid render target
	ERR_FAIL_COND(!p_render_target.is_valid());

	// Because we are rendering to our device we must use our main viewport!
	ERR_FAIL_COND(p_screen_rect == Rect2());

	// get the size of our screen
	godot_rect2 screen_rect;
	godot::api->godot_rect2_new(&screen_rect, p_screen_rect.position.x, p_screen_rect.position.y, p_screen_rect.size.width, p_screen_rect.size.height);

	godot_rid render_target_rid = p_render_target._get_godot_rid();
	godot::arvr_api->godot_arvr_blit(p_eye, &render_target_rid, &screen_rect);
}

// Positions of the quad vertices in clip space (X, Y).
const GLfloat kVertices[] = {
	//	-1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f,
	0.0f,
	0.0f,
	1.0f,
	0.0f,
	0.0f,
	1.0f,
	1.0f,
	1.0f,
};

void ARCoreInterface::make_anchors_stale() {
	for (const auto &anchor: anchors) {
		anchor.second->stale = true;
	}
}

void ARCoreInterface::remove_stale_anchors() {
	for (auto it = anchors.cbegin(); it != anchors.cend();) {
		ArPlane *ar_plane = it->first;
		anchor_map *am = it->second;
		if (am->stale) {
			it = anchors.erase(it);

			ARVRServer::get_singleton()->remove_tracker(am->tracker);
			delete am->tracker;
			delete am;
			ArTrackable_release(ArAsTrackable(ar_plane));
		} else {
			++it;
		}
	}
}

void ARCoreInterface::process() {
	if (init_status != INITIALISED) {
		// not yet initialised so....
		return;
	} else if ((ar_session == nullptr) or (feed.is_null())) {
		// don't have a session yet so...
		return;
	}

	// check display rotation
	int new_display_rotation = godot::android_api->godot_android_get_display_rotation();
	if (new_display_rotation != display_rotation) {
		display_rotation = new_display_rotation;
		if ((display_rotation == 1) || (display_rotation == 3)) {
			ArSession_setDisplayGeometry(ar_session, display_rotation, height, width);
		} else {
			ArSession_setDisplayGeometry(ar_session, display_rotation, width, height);
		}
		have_display_transform = false;

		Godot::print("Godot ARCore: Window orientation changes to " + String::num_int64(display_rotation) + " (" + String::num_int64(width) + ", " + String::num_int64(height) + ")");
	}

	// setup our camera texture
	if (camera_texture_id == 0) {
		// The size here isn't actually used, ARCore will manage it, but set it just in case
		// Also this is a YCbCr texture, not RGB, should probably add a format for that some day :)
		feed->_set_external(width, height);
		RID camera_texture = feed->get_texture(CameraServer::FEED_RGBA_IMAGE);
		godot_rid camera_texture_rid = camera_texture._get_godot_rid();
		camera_texture_id = godot::arvr_api->godot_arvr_get_texid(&camera_texture_rid);

		Godot::print("Godot ARCore: Created: " + String::num_int64(camera_texture_id));
	}

	// Have ARCore grab a camera frame, load it into our texture object and do its funky SLAM logic
	ArSession_setCameraTextureName(ar_session, camera_texture_id);

	// Update session to get current frame and render camera background.
	if (ArSession_update(ar_session, ar_frame) != AR_SUCCESS) {
		Godot::print("Godot ARCore: OnDrawFrame ArSession_update error");
	}

	ArCamera *ar_camera;
	ArFrame_acquireCamera(ar_session, ar_frame, &ar_camera);

	// process our view matrix
	float view_mat[4][4];
	ArCamera_getViewMatrix(ar_session, ar_camera, (float *)view_mat);

	// TODO: We may need to adjust this based on orientation
	view.basis.elements[0].x = view_mat[0][0];
	view.basis.elements[1].x = view_mat[0][1];
	view.basis.elements[2].x = view_mat[0][2];
	view.basis.elements[0].y = view_mat[1][0];
	view.basis.elements[1].y = view_mat[1][1];
	view.basis.elements[2].y = view_mat[1][2];
	view.basis.elements[0].z = view_mat[2][0];
	view.basis.elements[1].z = view_mat[2][1];
	view.basis.elements[2].z = view_mat[2][2];
	view.origin.x = view_mat[3][0];
	view.origin.y = view_mat[3][1];
	view.origin.z = view_mat[3][2];
	// invert our view matrix
	view.invert();

	// process our projection matrix
	float projection_mat[4][4];
	ArCamera_getProjectionMatrix(ar_session, ar_camera, z_near, z_far, (float *)projection_mat);

	projection.matrix[0][0] = projection_mat[0][0];
	projection.matrix[1][0] = projection_mat[1][0];
	projection.matrix[2][0] = projection_mat[2][0];
	projection.matrix[3][0] = projection_mat[3][0];
	projection.matrix[0][1] = projection_mat[0][1];
	projection.matrix[1][1] = projection_mat[1][1];
	projection.matrix[2][1] = projection_mat[2][1];
	projection.matrix[3][1] = projection_mat[3][1];
	projection.matrix[0][2] = projection_mat[0][2];
	projection.matrix[1][2] = projection_mat[1][2];
	projection.matrix[2][2] = projection_mat[2][2];
	projection.matrix[3][2] = projection_mat[3][2];
	projection.matrix[0][3] = projection_mat[0][3];
	projection.matrix[1][3] = projection_mat[1][3];
	projection.matrix[2][3] = projection_mat[2][3];
	projection.matrix[3][3] = projection_mat[3][3];

	ArTrackingState camera_tracking_state;
	ArCamera_getTrackingState(ar_session, ar_camera, &camera_tracking_state);
	switch (camera_tracking_state) {
		case AR_TRACKING_STATE_TRACKING:
			tracking_state = ARVRInterface::ARVR_NORMAL_TRACKING;
			break;
		case AR_TRACKING_STATE_PAUSED:
			// lets find out why..
			ArTrackingFailureReason camera_tracking_failure_reason;
			ArCamera_getTrackingFailureReason(ar_session, ar_camera, &camera_tracking_failure_reason);
			switch (camera_tracking_failure_reason) {
				case AR_TRACKING_FAILURE_REASON_NONE:
					tracking_state = ARVRInterface::ARVR_UNKNOWN_TRACKING;
					break;
				case AR_TRACKING_FAILURE_REASON_BAD_STATE:
					tracking_state = ARVRInterface::ARVR_INSUFFICIENT_FEATURES; // @TODO add bad state to ARVRInterface
					break;
				case AR_TRACKING_FAILURE_REASON_INSUFFICIENT_LIGHT:
					tracking_state = ARVRInterface::ARVR_INSUFFICIENT_FEATURES; // @TODO add insufficient light to ARVRInterface
					break;
				case AR_TRACKING_FAILURE_REASON_EXCESSIVE_MOTION:
					tracking_state = ARVRInterface::ARVR_EXCESSIVE_MOTION;
					break;
				case AR_TRACKING_FAILURE_REASON_INSUFFICIENT_FEATURES:
					tracking_state = ARVRInterface::ARVR_INSUFFICIENT_FEATURES;
					break;
				case AR_TRACKING_FAILURE_REASON_CAMERA_UNAVAILABLE:
					tracking_state = ARVRInterface::ARVR_INSUFFICIENT_FEATURES; // @TODO add no camera to ARVRInterface
					break;
				default:
					tracking_state = ARVRInterface::ARVR_UNKNOWN_TRACKING;
					break;
			};

			break;
		case AR_TRACKING_STATE_STOPPED:
			tracking_state = ARVRInterface::ARVR_NOT_TRACKING;
			break;
		default:
			tracking_state = ARVRInterface::ARVR_UNKNOWN_TRACKING;
			break;
	}

	ArCamera_release(ar_camera);

	// If display rotation changed (also includes view size change), we need to
	// re-query the uv coordinates for the on-screen portion of the camera image.
	int32_t geometry_changed = 0;

	ArFrame_getDisplayGeometryChanged(ar_session, ar_frame, &geometry_changed);
	if (geometry_changed != 0 || !have_display_transform) {
		// update our transformed uvs
		float transformed_uvs[4 * 2];
		ArFrame_transformCoordinates2d(ar_session, ar_frame, AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES, 4, kVertices, AR_COORDINATES_2D_TEXTURE_NORMALIZED, transformed_uvs);
		have_display_transform = true;

		// got to convert these uvs. They seem weird in portrait mode..
		bool shift_x = false;
		bool shift_y = false;

		// -1.0 - 1.0 => 0.0 - 1.0
		for (int i = 0; i < 8; i += 2) {
			transformed_uvs[i] = transformed_uvs[i] * 2.0 - 1.0;
			shift_x = shift_x || (transformed_uvs[i] < -0.001);
			transformed_uvs[i + 1] = transformed_uvs[i + 1] * 2.0 - 1.0;
			shift_y = shift_y || (transformed_uvs[i + 1] < -0.001);
		}

		// do we need to shift anything?
		if (shift_x || shift_y) {
			for (int i = 0; i < 8; i += 2) {
				if (shift_x) transformed_uvs[i] += 1.0;
				if (shift_y) transformed_uvs[i + 1] += 1.0;
			}
		}

		// Convert transformed_uvs to our display transform
		Transform2D display_transform;
		display_transform.elements[0] = Vector2(transformed_uvs[2] - transformed_uvs[0], transformed_uvs[3] - transformed_uvs[1]);
		display_transform.elements[1] = Vector2(transformed_uvs[4] - transformed_uvs[0], transformed_uvs[5] - transformed_uvs[1]);
		display_transform.elements[2] = Vector2(transformed_uvs[0], transformed_uvs[1]);
		feed->set_transform(display_transform);
	}

	// mark anchors as stale
	make_anchors_stale();

	// Now need to handle our anchors and such....
	ArTrackableList *plane_list = nullptr;
	ArTrackableList_create(ar_session, &plane_list);
	if (plane_list != nullptr) {
		//@TODO possibly change this to using ArFrame_getUpdatedTrackables, but then need to figure out how we retire merged planes
		// can't say I find the documentation easy to follow here

		ArTrackableType plane_tracked_type = AR_TRACKABLE_PLANE;
		ArSession_getAllTrackables(ar_session, plane_tracked_type, plane_list);

		int32_t plane_list_size = 0;
		ArTrackableList_getSize(ar_session, plane_list, &plane_list_size);

		for (int i = 0; i < plane_list_size; i++) {
			// stealing this bit from the ARCore SDK....

			// Godot::print(String("Godot ARCore: checking plane ") + String::num_int64(i));

			// grab our trackable plane...
			ArTrackable *ar_trackable = nullptr;
			ArTrackableList_acquireItem(ar_session, plane_list, i, &ar_trackable);
			ArPlane *ar_plane = ArAsPlane(ar_trackable);

			ArTrackingState out_tracking_state;
			ArTrackable_getTrackingState(ar_session, ar_trackable, &out_tracking_state);
			if (out_tracking_state != ArTrackingState::AR_TRACKING_STATE_TRACKING) {
				Godot::print(String("Godot ARCore: not tracking plane ") + String::num_int64(i));
				continue;
			}

			// subsume this plane, I'm not sure what that means, we don't seem to use the result...
			ArPlane *subsume_plane;
			ArPlane_acquireSubsumedBy(ar_session, ar_plane, &subsume_plane);
			if (subsume_plane != nullptr) {
				Godot::print(String("Godot ARCore: can't subsume plane ") + String::num_int64(i));

				ArTrackable_release(ArAsTrackable(subsume_plane));
				continue;
			}

			// grabbing the tracking state again, not sure why...
			ArTrackingState plane_tracking_state;
			ArTrackable_getTrackingState(ar_session, ArAsTrackable(ar_plane), &plane_tracking_state);
			if (plane_tracking_state == ArTrackingState::AR_TRACKING_STATE_TRACKING) {
				// now we need to check if we have this as a tracking in Godot...
				anchor_map *am = nullptr;

				auto search = anchors.find(ar_plane);
				if (search != anchors.end()) {
					am = anchors[ar_plane];
					am->stale = false;

					// If this is an already observed trackable release it so it doesn't
					// leave an additional reference dangling.
					ArTrackable_release(ar_trackable);
				} else {
					// Godot::print(String("Godot ARCore: adding new plane ") + String::num_int64(i));

					am = new anchor_map;
					am->stale = false;

					// create our tracker
					am->tracker = new ARVRPositionalTracker;
					am->tracker->_set_name(String("Anchor ") + String::num_int64(last_anchor_id++));
					am->tracker->_set_type(ARVRServer::TRACKER_ANCHOR);

					ARVRServer::get_singleton()->add_tracker(am->tracker);

					anchors[ar_plane] = am;
				}

				if (am != nullptr) {
					///@TODO need to find a way to figure out something has chanced, we don't really want to update this every frame if nothing
					// has changed....

					// get center position of our plane
					float mat[4][4];
					ArPose *pose;
					ArPose_create(ar_session, nullptr, &pose);
					ArPlane_getCenterPose(ar_session, ar_plane, pose);
					ArPose_getMatrix(ar_session, pose, (float *)mat);
					// normal_vec_ = util::GetPlaneNormal(ar_session, *pose);

					Basis b;
					b.elements[0].x = mat[0][0];
					b.elements[1].x = mat[0][1];
					b.elements[2].x = mat[0][2];
					b.elements[0].y = mat[1][0];
					b.elements[1].y = mat[1][1];
					b.elements[2].y = mat[1][2];
					b.elements[0].z = mat[2][0];
					b.elements[1].z = mat[2][1];
					b.elements[2].z = mat[2][2];
					am->tracker->_set_orientation(b);

					am->tracker->_set_rw_position(Vector3(mat[3][0], mat[3][1], mat[3][2]));

					ArPose_destroy(pose);

					// TODO should now get the polygon data and build our mesh
				}
			} else {
				Godot::print(String("Godot ARCore: huh? I thought we were tracking plane ") + String::num_int64(i));
			}
		}

		ArTrackableList_destroy(plane_list);

		// now we remove our stale trackers..
		remove_stale_anchors();
	}
}

ARCoreInterface::ARCoreInterface() {
	ar_session = nullptr;
	ar_frame = nullptr;
	init_status = NOT_INITIALISED;
	width = 1;
	height = 1;
	display_rotation = 0;
	camera_texture_id = 0;
	last_anchor_id = 0;
	z_near = 0.01;
	z_far = 1000.0;
	have_display_transform = false;
	projection.set_perspective(60.0, 1.0, z_near, z_far, false); // this is just a default, will be changed by ARCore

	// !BAS! TODO once we have added this to Godot uncomment, this will not work without!!
	// set_interface(&arvr_interface_struct);
}

ARCoreInterface::~ARCoreInterface() {
	// remove_all_anchors();

	// and make sure we cleanup if we haven't already
	if (is_initialized()) {
		uninitialize();
	}
}
