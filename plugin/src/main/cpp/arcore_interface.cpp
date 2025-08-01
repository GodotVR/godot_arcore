#include "ar_utils.h"
#include "arcore_interface.h"
#include "arcore_wrapper.h"
#include "utils.h"
#include <godot_cpp/classes/camera_server.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ARCoreInterface::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_resume"), &ARCoreInterface::_resume);
	ClassDB::bind_method(D_METHOD("_pause"), &ARCoreInterface::_pause);
	ClassDB::bind_method(D_METHOD("start"), &ARCoreInterface::start);
	ClassDB::bind_method(D_METHOD("enable_depth_estimation"), &ARCoreInterface::enable_depth_estimation);
	ClassDB::bind_method(D_METHOD("is_depth_supported"), &ARCoreInterface::is_depth_supported);
	ClassDB::bind_method(D_METHOD("show_depth_map"), &ARCoreInterface::show_depth_map);
	ClassDB::bind_method(D_METHOD("set_max_depth_meters"), &ARCoreInterface::set_max_depth_meters);
	ClassDB::bind_method(D_METHOD("enable_vertical_plane_detection"), &ARCoreInterface::enable_vertical_plane_detection);
	ClassDB::bind_method(D_METHOD("enable_horizontal_plane_detection"), &ARCoreInterface::enable_horizontal_plane_detection);
	ClassDB::bind_method(D_METHOD("enable_instant_placement"), &ARCoreInterface::enable_instant_placement);
	ClassDB::bind_method(D_METHOD("get_camera_feed"), &ARCoreInterface::get_camera_feed);
	ClassDB::bind_method(D_METHOD("getHitPose"), &ARCoreInterface::getHitPose);
	ClassDB::bind_method(D_METHOD("getHitRayPose"), &ARCoreInterface::getHitRayPose);
	ClassDB::bind_method(D_METHOD("enable_images_detection"), &ARCoreInterface::enable_images_detection);
	ClassDB::bind_method(D_METHOD("set_node_images_mapping"), &ARCoreInterface::set_node_images_mapping);
	ClassDB::bind_method(D_METHOD("enable_point_cloud_detection"), &ARCoreInterface::enable_point_cloud_detection);
	ClassDB::bind_method(D_METHOD("switch_orientation"), &ARCoreInterface::switch_orientation);
	ClassDB::bind_method(D_METHOD("enable_light_estimation"), &ARCoreInterface::enable_light_estimation);
	ClassDB::bind_method(D_METHOD("get_light_color_correction"), &ARCoreInterface::get_light_color_correction);
	ClassDB::bind_method(D_METHOD("get_light_main_hdr_direction"), &ARCoreInterface::get_light_main_hdr_direction);
	ClassDB::bind_method(D_METHOD("get_light_main_hdr_intensity"), &ARCoreInterface::get_light_main_hdr_intensity);
	ClassDB::bind_method(D_METHOD("image_tracker_database_create"), &ARCoreInterface::image_tracker_database_create);
	ClassDB::bind_method(D_METHOD("image_tracker_database_add_image"), &ARCoreInterface::image_tracker_database_add_image);
	ClassDB::bind_method(D_METHOD("image_tracker_database_get_serialized"), &ARCoreInterface::image_tracker_database_get_serialized);
	ClassDB::bind_method(D_METHOD("image_tracker_database_load"), &ARCoreInterface::image_tracker_database_load);
	ClassDB::bind_method(D_METHOD("image_tracker_get_image_tracking_status"), &ARCoreInterface::image_tracker_get_image_tracking_status);
	ClassDB::bind_method(D_METHOD("image_tracker_get_tracked_transform"), &ARCoreInterface::image_tracker_get_tracked_transform);
	ClassDB::bind_method(D_METHOD("plane_tracker_get_all_plane_ids"), &ARCoreInterface::plane_tracker_get_all_plane_ids);
	ClassDB::bind_method(D_METHOD("plane_tracker_get_plane_boundary"), &ARCoreInterface::plane_tracker_get_plane_boundary);
	ClassDB::bind_method(D_METHOD("plane_tracker_get_plane_transform"), &ARCoreInterface::plane_tracker_get_plane_transform);
	ClassDB::bind_method(D_METHOD("plane_tracker_get_plane_parent"), &ARCoreInterface::plane_tracker_get_plane_parent);

	ClassDB::bind_method(D_METHOD("getNear"), &ARCoreInterface::getNear);
	ClassDB::bind_method(D_METHOD("getFar"), &ARCoreInterface::getFar);
}



ARCoreInterface::ARCoreInterface() {
	m_init_status = NOT_INITIALISED;
	m_ar_session = nullptr;
	m_ar_frame = nullptr;
	m_screen_width = 1;
	m_screen_height = 1;
	m_display_orientation = Orientation::UNKNOWN;
	m_z_near = 0.01;
	m_z_far = 1000.0;
	m_enable_depth_estimation = false;
	m_enable_vertical_plane_detection = false;
	m_enable_horizontal_plane_detection = false;
	m_enable_instant_placement = false;
	m_enable_images_detection = false;
	m_enable_point_cloud_detection = false;
	m_enable_light_estimation = false;

	// This is just a default and will be changed by ARCore
	m_projection.set_perspective(60.0, 1.0, m_z_near, m_z_far, false);
}

ARCoreInterface::~ARCoreInterface() {
	// remove_all_anchors();

	if (is_initialized()) {
		uninitialize();
	}
}

XRInterface::TrackingStatus ARCoreInterface::_get_tracking_status() const {
	return m_tracking_status;
}

void ARCoreInterface::start() {
	ALOGV("ARCoreInterface::start()");
	printf("CSTM: ARCoreInterface::start()");
}

void ARCoreInterface::_resume() {
	if (m_init_status == INITIALISED && m_ar_session != nullptr) {
		ArStatus status = ArSession_resume(m_ar_session);

		if (status != AR_SUCCESS) {
			ALOGE("ARCorePlugin: Failed to resume");
			// TODO: Quit or end the ARCore session gracefully
		}
	}
}

void ARCoreInterface::_pause() {
	godot::UtilityFunctions::print("ARCoreInterface::_pause");
	ALOGV("ARCoreInterface::_pause");
	if (m_ar_session != nullptr) {
		ArSession_pause(m_ar_session);
	}
}

StringName ARCoreInterface::_get_name() const {
	StringName name("ARCore");
	return name;
}

uint32_t ARCoreInterface::_get_capabilities() const {
	return XR_MONO + XR_AR;
}

int32_t ARCoreInterface::_get_camera_feed_id() const {
	if (m_background_renderer.getFeed().is_valid()) {
		return m_background_renderer.getFeed()->get_id();
	} else {
		return 0;
	}
}

void ARCoreInterface::enable_depth_estimation(bool p_enable) {
	ALOGW("Godot ARCore: Requestiing depth estimation");
	printf("CSTM: Godot ARCore: Requestiing depth estimation");

	if(!is_depth_supported()){
		ALOGW("Godot ARCore: Requested depth estimation, but is not supported, aborting");
		printf("CSTM: Godot ARCore: Requested depth estimation, but is not supported, aborting");
		return;
	}
	m_enable_depth_estimation = p_enable;
	configureSession();
}

bool ARCoreInterface::is_depth_supported() {
	int32_t is_supported = 0;
	ArSession_isDepthModeSupported(m_ar_session, AR_DEPTH_MODE_AUTOMATIC,
			&is_supported);
	return is_supported;
}

void ARCoreInterface::show_depth_map(bool p_enable) {
	if (m_enable_depth_estimation) {
		godot::Ref<godot::CameraFeed> feed = m_background_renderer.getFeed();
		if (feed.is_valid()) {
			// feed->set_should_display_depthmap(p_enable);
		}
	} else {
		ALOGW("Godot ARCore: show_depth_map called but depth estimation is not enabled");
	}
}

void ARCoreInterface::set_max_depth_meters(float p_max_depth_meters) {
	if (m_enable_depth_estimation) {
		godot::Ref<godot::CameraFeed> feed = m_background_renderer.getFeed();
		if (feed.is_valid()) {
			// feed->set_max_depth_meters(p_max_depth_meters);
		}
	} else {
		ALOGW("Godot ARCore: set_max_depth_meters called but depth estimation is not enabled");
	}
}

void ARCoreInterface::enable_vertical_plane_detection(bool p_enable) {
	m_enable_vertical_plane_detection = p_enable;
	configureSession();
}

void ARCoreInterface::enable_horizontal_plane_detection(bool p_enable) {
	m_enable_horizontal_plane_detection = p_enable;
	configureSession();
}

void ARCoreInterface::enable_instant_placement(bool p_enable) {
	m_enable_instant_placement = p_enable;
	configureSession();
}

void ARCoreInterface::enable_images_detection(bool p_enable) {
	m_enable_images_detection = p_enable;
	configureSession();
}

void ARCoreInterface::enable_point_cloud_detection(bool p_enable) {
	m_enable_point_cloud_detection = p_enable;
	configureSession();
}

void ARCoreInterface::switch_orientation(bool p_vertical) {
	m_vertical_orientation = p_vertical;
	configureSession();
}

void ARCoreInterface::enable_light_estimation(bool p_enable) {
	m_enable_light_estimation = true;
	configureSession();
}

Vector4 ARCoreInterface::get_light_color_correction() {
	Vector4 res(1.f, 1.f, 1.f, 1.f);

	if (m_enable_light_estimation) {
		res = m_color_correction;
	} else {
		ALOGW("Godot ARCore: get_light_color_correction called but light estimation is not enabled");
	}

	return res;
}

Vector3 ARCoreInterface::get_light_main_hdr_direction() {
	Vector3 res(0.f, 0.f, 0.f);

	if (m_enable_light_estimation) {
		res = m_light_env_hdr_main_light_direction;
	} else {
		ALOGW("Godot ARCore: get_light_color_correction called but light estimation is not enabled");
	}

	return res;
}

Vector3 ARCoreInterface::get_light_main_hdr_intensity() {
	Vector3 res(0.f, 0.f, 0.f);

	if (m_enable_light_estimation) {
		res = m_light_env_hdr_main_light_intensity;
	} else {
		ALOGW("Godot ARCore: get_light_color_correction called but light estimation is not enabled");
	}

	return res;
}

godot::Ref<godot::CameraFeed> ARCoreInterface::get_camera_feed() {
	if(m_init_status != INITIALISED) return godot::Ref<godot::CameraFeed>();
	return m_background_renderer.getFeed();
}

Transform3D ARCoreInterface::getHitPose(float p_pixel_x, float p_pixel_y) {
	Transform3D res;

	
	ALOGV("Godot ARCore: Casting screen ray...\n");

	ArHitResultList *hit_result_list = nullptr;
	ArHitResultList_create(m_ar_session, &hit_result_list);
	ArFrame_hitTest(m_ar_session, m_ar_frame, p_pixel_x, p_pixel_y, hit_result_list);

	int32_t size = 0;
	ArHitResultList_getSize(m_ar_session, hit_result_list, &size);
	float currentMinDistance = std::numeric_limits<float>::max();

	ALOGV("Hitlist contains %d results\n", size);

	if (size > 0){
		ArHitResult *hit_result = nullptr;
		ArHitResult_create(m_ar_session, &hit_result);
		ArHitResultList_getItem(m_ar_session, hit_result_list, 0, hit_result);
		arcore_plugin::ScopedArPose scopedArPose(m_ar_session);
		ArHitResult_getHitPose(m_ar_session, hit_result, scopedArPose.GetArPose());
		float distance = std::numeric_limits<float>::max();
		ArHitResult_getDistance(m_ar_session, hit_result, &distance);

		if (distance < currentMinDistance) {
			float mat4[16];
			ArPose_getMatrix(m_ar_session, scopedArPose.GetArPose(), mat4);
			res = to_godot_transform(mat4);
		}
	
		ArHitResult_destroy(hit_result);
	}
	ArHitResultList_destroy(hit_result_list);


	return res;
}

Transform3D ARCoreInterface::getHitRayPose(const Vector3 &p_origin, const Vector3 &p_direction) {
	Transform3D res;

	float ray_origin_3[3]{ p_origin.x, p_origin.y, p_origin.z };
	float ray_direction_3[3]{ p_origin.x, p_origin.y, p_origin.z };
	ArHitResultList *hit_result_list = nullptr;
	ArHitResultList_create(m_ar_session, &hit_result_list);
	ArFrame_hitTestRay(m_ar_session, m_ar_frame, ray_origin_3, ray_direction_3, hit_result_list);
	
	ALOGV("Godot ARCore: Casting ray...\n");
	
	int32_t size = 0;
	ArHitResultList_getSize(m_ar_session, hit_result_list, &size);
	float currentMinDistance = std::numeric_limits<float>::max();

	ALOGV("Hitlist contains %d results\n", size);

	if(size > 0) {
		ArHitResult *hit_result = nullptr;
		ArHitResult_create(m_ar_session, &hit_result);
		// Only get first result, which is the closest per definition
		ArHitResultList_getItem(m_ar_session, hit_result_list, 0, hit_result);

		arcore_plugin::ScopedArPose scopedArPose(m_ar_session);
		ArHitResult_getHitPose(m_ar_session, hit_result, scopedArPose.GetArPose());
		float mat4[16];
		ArPose_getMatrix(m_ar_session, scopedArPose.GetArPose(), mat4);
		Transform3D trsf = to_godot_transform(mat4);
		Vector3 target = trsf.get_origin();
		float distance = (target - p_origin).length();

		if (distance < currentMinDistance) {
			res = trsf;
		}

		ArHitResult_destroy(hit_result);
	}
	ArHitResultList_destroy(hit_result_list);

	return res;
}

void ARCoreInterface::set_node_images_mapping(const Dictionary &in_nodeImagesMap) {
	ALOGV("Godot ARCore: Requested image tracking of %ld images\n", in_nodeImagesMap.size());
	if (m_enable_images_detection) {
		m_instances_renderer.set_node_images_mapping(m_ar_session, in_nodeImagesMap);

		// dunno if this is required, put this when debugging
		configureSession();

	} else {
		ALOGW("Godot ARCore: set_node_images_mapping called but images detection is not enabled");
	}
}



void ARCoreInterface::image_tracker_database_create(){
	ALOGV("Creating new image tracker database and updating session config\n");
	m_image_tracker.createFreshDatabase(m_ar_session);
	configureSession();
}

void ARCoreInterface::image_tracker_database_load(const godot::PackedByteArray &db){
	m_image_tracker.loadSerializedDatabase(m_ar_session, db.ptr(), db.size());
	configureSession();
}

godot::PackedByteArray ARCoreInterface::image_tracker_database_get_serialized(){
	uint8_t *data;
	int64_t size;
	m_image_tracker.serializeDatabase(m_ar_session, &data, &size);

	// TODO look into this
	// copy :( is there another way to send this in godot world??
	// ArString release is expected, but is it needed?? https://developers.google.com/ar/reference/c/group/ar-augmented-image-database#araugmentedimagedatabase_serialize
	// Also, would using a PackedInt64 be faster?
	PackedByteArray pba;
	pba.resize(size);
	for(int64_t i = 0; i < size; i++) pba.set(i,data[i]);
	ArByteArray_release(data);
	return pba;
}

void ARCoreInterface::image_tracker_database_add_image(godot::Ref<godot::Image> img, const godot::String &img_identifier, float img_physical_width){
	m_image_tracker.registerImageForTracking(m_ar_session, img.ptr(), img_identifier, img_physical_width);
	configureSession();
}

bool ARCoreInterface::image_tracker_get_image_tracking_status(const godot::String &img_identifier){
	return m_image_tracker.getImageTrackingStatus(m_ar_session, img_identifier);
}

godot::Transform3D ARCoreInterface::image_tracker_get_tracked_transform(const godot::String &img_identifier){
	float *mat;
	m_image_tracker.getImageTransformMatrix(m_ar_session, img_identifier, &mat);
	return to_godot_transform(mat);
}


godot::Array ARCoreInterface::plane_tracker_get_all_plane_ids(){
	return m_plane_tracker.getAllAvailablePlaneIds();
}
godot::Array ARCoreInterface::plane_tracker_get_plane_boundary(int plane_id){
	return m_plane_tracker.getPlaneBoundary(plane_id);
}
godot::Transform3D ARCoreInterface::plane_tracker_get_plane_transform(int plane_id){
	float *mat4;
	m_plane_tracker.getPlaneTransformMatrix(plane_id, &mat4);
	return to_godot_transform(mat4);
}
int ARCoreInterface::plane_tracker_get_plane_parent(int plane_id){
	return m_plane_tracker.getPlaneParentId(plane_id);
}


bool ARCoreInterface::_is_initialized() const {
	// If we are in the process of initialising, we treat it as initialised
	return (m_init_status != NOT_INITIALISED) && (m_init_status != INITIALISE_FAILED);
}

bool ARCoreInterface::_initialize() {
	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V(xr_server, false);

	if (m_init_status == INITIALISE_FAILED) {
		// If the interface failed to initialise, don't try again
		return false;
	} else if (m_init_status == NOT_INITIALISED) {
		m_init_status = START_INITIALISE;

		if (m_ar_session == nullptr) {
			// Get the android environment
			JNIEnv *env = ARCoreWrapper::get_env();

			jobject context = ARCoreWrapper::get_global_context();

			if (context == nullptr) {
				ALOGE("ARCorePlugin: Couldn't get global context");
				m_init_status = INITIALISE_FAILED;
				return false;
			}

			if (ArSession_create(env, context, &m_ar_session) != AR_SUCCESS || m_ar_session == nullptr) {
				ALOGE("ARCorePlugin: Session couldn't be created");
				m_init_status = INITIALISE_FAILED;
				return false;
			}

			ArFrame_create(m_ar_session, &m_ar_frame);

			if (m_ar_frame == nullptr) {
				ALOGE("ARCorePlugin: Frame couldn't be created");

				ArSession_destroy(m_ar_session);
				m_ar_session = nullptr;

				m_init_status = INITIALISE_FAILED; // don't try initialising again
				return false;
			}

			// Get the screen size
			Size2 screen_size = DisplayServer::get_singleton()->screen_get_size();

			if(screen_size.y > screen_size.x){
				// hope we are on a phone
				m_display_orientation = Orientation::ROTATION_0;
		
				m_screen_height = screen_size.x;
				m_screen_width = screen_size.y;
			} else {
				m_display_orientation = Orientation::ROTATION_90;
		
				m_screen_width = screen_size.x;
				m_screen_height = screen_size.y;
			}
			

			// Trigger a display rotation
			m_display_orientation = Orientation::ROTATION_0;


			// Everything is initialised
			m_init_status = INITIALISED;
		}

		// Now call resume to complete the initialization
		_resume();

		if (m_init_status != INITIALISE_FAILED) {
			// The camera operates as a head and we need to create a tracker for that
			m_head.instantiate();
			m_head->set_tracker_type(XRServer::TRACKER_HEAD);
			m_head->set_tracker_name("head");
			m_head->set_tracker_desc("AR Device");
			xr_server->add_tracker(m_head);

			// Make ARCore the primary interface
			xr_server->set_primary_interface(this);
		}

		m_background_renderer.initialize(m_screen_width, m_screen_height);
		m_image_tracker.initialize(m_ar_session);
		m_instances_renderer.initalize(&m_image_tracker);


		configureSession();
	}

	return is_initialized();
}

void ARCoreInterface::_uninitialize() {
	if (_is_initialized()) {
		XRServer *xr_server = XRServer::get_singleton();
		ERR_FAIL_NULL(xr_server);

		if (m_ar_session != nullptr) {
			ArSession_destroy(m_ar_session);
			ArFrame_destroy(m_ar_frame);

			m_ar_session = nullptr;
			m_ar_frame = nullptr;
		}

		m_background_renderer.uninitialize();

		if (m_head.is_valid()) {
			xr_server->remove_tracker(m_head);
			m_head.unref();
		}

		m_init_status = NOT_INITIALISED;
	}
}

Vector2 ARCoreInterface::_get_render_target_size() {
	Vector2 target_size = DisplayServer::get_singleton()->screen_get_size();
	return target_size;
}

uint32_t ARCoreInterface::_get_view_count() {
	return 1;
}

Transform3D ARCoreInterface::_get_camera_transform() {
	Transform3D transform_for_eye;

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V(xr_server, Transform3D());

	if (m_init_status == INITIALISED) {
		float world_scale = xr_server->get_world_scale();

		// just scale our origin point of our transform, note that we really shouldn't be using world_scale in ARKit but....
		transform_for_eye = m_view;
		transform_for_eye.origin *= world_scale;

		transform_for_eye = xr_server->get_reference_frame() * transform_for_eye;
	}

	return transform_for_eye;
}

Transform3D ARCoreInterface::_get_transform_for_view(uint32_t p_view, const Transform3D &p_cam_transform) {
	Transform3D transform_for_eye;

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V(xr_server, Transform3D());

	if (m_init_status == INITIALISED) {
		float world_scale = xr_server->get_world_scale();

		// just scale our origin point of our transform, note that we really shouldn't be using world_scale in ARKit but....
		transform_for_eye = m_view;
		transform_for_eye.origin *= world_scale;

		transform_for_eye = p_cam_transform * (xr_server->get_reference_frame()) * transform_for_eye;
	} else {
		// huh? well just return what we got....
		transform_for_eye = p_cam_transform;
	}

	return transform_for_eye;
}

PackedFloat64Array ARCoreInterface::_get_projection_for_view(uint32_t p_view, double p_aspect, double p_m_z_near, double p_z_far) {
	PackedFloat64Array arr;
	arr.resize(16); // 4x4 matrix

	// Remember our near and far, we'll use it next frame
	m_z_near = p_m_z_near;
	m_z_far = p_z_far;

	real_t *p = (real_t *)&m_projection.columns;
	for (int i = 0; i < 16; i++) {
		arr[i] = p[i];
	}

	return arr;
}

void ARCoreInterface::_post_draw_viewport(const RID &p_render_target, const Rect2 &p_screen_rect) {
	// We must have a valid render target
	ERR_FAIL_COND(!p_render_target.is_valid());

	// Because we are rendering to our device we must use our main viewport!
	ERR_FAIL_COND(p_screen_rect == Rect2());

	Rect2 src_rect(0.0f, 0.0f, 1.0f, 1.0f);
	Rect2 dst_rect = p_screen_rect;

	add_blit(p_render_target, src_rect, dst_rect, false, 0, false, Vector2(), 0, 0, 0.0, 1.0);
}

void ARCoreInterface::estimateLight() {
	// Get light estimation value.
	ArLightEstimate *ar_light_estimate;
	ArLightEstimateState ar_light_estimate_state;
	ArLightEstimate_create(m_ar_session, &ar_light_estimate);

	ArFrame_getLightEstimate(m_ar_session, m_ar_frame, ar_light_estimate);
	ArLightEstimate_getState(m_ar_session, ar_light_estimate,
			&ar_light_estimate_state);

	// Set light intensity to default. Intensity value ranges from 0.0f to 1.0f.
	// The first three components are color scaling factors.
	// The last one is the average pixel intensity in gamma space.
	float color_correction[4] = { 1.f, 1.f, 1.f, 1.f };
	float light_env_hdr_main_light_direction[3] = { 1.f, 1.f, 1.f };
	float light_env_hdr_main_light_intensity[3] = { 1.f, 1.f, 1.f };
	if (ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID) {
		ArLightEstimate_getColorCorrection(m_ar_session, ar_light_estimate,
				color_correction);
		m_color_correction = Vector4(m_color_correction[0], m_color_correction[1], m_color_correction[2], m_color_correction[3]);

		ArLightEstimate_getEnvironmentalHdrMainLightDirection(m_ar_session, ar_light_estimate, light_env_hdr_main_light_direction);
		m_light_env_hdr_main_light_direction = Vector3(m_light_env_hdr_main_light_direction[0], m_light_env_hdr_main_light_direction[1], m_light_env_hdr_main_light_direction[2]);

		ArLightEstimate_getEnvironmentalHdrMainLightIntensity(m_ar_session, ar_light_estimate, light_env_hdr_main_light_intensity);
		m_light_env_hdr_main_light_intensity = Vector3(m_light_env_hdr_main_light_intensity[0], m_light_env_hdr_main_light_intensity[1], m_light_env_hdr_main_light_intensity[2]);
	}

	ArLightEstimate_destroy(ar_light_estimate);
	ar_light_estimate = nullptr;
}

godot::XRInterface::TrackingStatus ar_tracking_state_to_tracking_status(const ArSession &p_ar_session, const ArCamera &p_ar_camera, const ArTrackingState &p_camera_tracking_state) {
	godot::XRInterface::TrackingStatus trackingStatus;
	switch (p_camera_tracking_state) {
		case AR_TRACKING_STATE_TRACKING:
			trackingStatus = XRInterface::XR_NORMAL_TRACKING;
			break;
		case AR_TRACKING_STATE_PAUSED:
			// lets find out why..
			ArTrackingFailureReason camera_tracking_failure_reason;
			ArCamera_getTrackingFailureReason(&p_ar_session, &p_ar_camera, &camera_tracking_failure_reason);
			switch (camera_tracking_failure_reason) {
				case AR_TRACKING_FAILURE_REASON_NONE:
					ALOGE("Godot ARCore: AR_TRACKING_FAILURE_REASON_NONE");
					trackingStatus = XRInterface::XR_UNKNOWN_TRACKING;
					break;
				case AR_TRACKING_FAILURE_REASON_BAD_STATE:
					ALOGE("Godot ARCore: AR_TRACKING_FAILURE_REASON_BAD_STATE");
					trackingStatus = XRInterface::XR_INSUFFICIENT_FEATURES; // @TODO add bad state to XRInterface
					break;
				case AR_TRACKING_FAILURE_REASON_INSUFFICIENT_LIGHT:
					ALOGE("Godot ARCore: AR_TRACKING_FAILURE_REASON_INSUFFICIENT_LIGHT");
					trackingStatus = XRInterface::XR_INSUFFICIENT_FEATURES; // @TODO add insufficient light to XRInterface
					break;
				case AR_TRACKING_FAILURE_REASON_EXCESSIVE_MOTION:
					ALOGE("Godot ARCore: AR_TRACKING_FAILURE_REASON_EXCESSIVE_MOTION");
					trackingStatus = XRInterface::XR_EXCESSIVE_MOTION;
					break;
				case AR_TRACKING_FAILURE_REASON_INSUFFICIENT_FEATURES:
					ALOGE("Godot ARCore: AR_TRACKING_FAILURE_REASON_INSUFFICIENT_FEATURES");
					trackingStatus = XRInterface::XR_INSUFFICIENT_FEATURES;
					break;
				case AR_TRACKING_FAILURE_REASON_CAMERA_UNAVAILABLE:
					ALOGE("Godot ARCore: AR_TRACKING_FAILURE_REASON_CAMERA_UNAVAILABLE");
					trackingStatus = XRInterface::XR_INSUFFICIENT_FEATURES; // @TODO add no camera to XRInterface
					break;
				default:
					ALOGE("Godot ARCore: XR_UNKNOWN_TRACKING");
					trackingStatus = XRInterface::XR_UNKNOWN_TRACKING;
					break;
			};

			break;
		case AR_TRACKING_STATE_STOPPED:
			ALOGE("Godot ARCore: AR_TRACKING_STATE_STOPPED");
			trackingStatus = XRInterface::XR_NOT_TRACKING;
			break;
		default:
			ALOGE("Godot ARCore: XR_UNKNOWN_TRACKING");
			trackingStatus = XRInterface::XR_UNKNOWN_TRACKING;
			break;
	};

	return trackingStatus;
}

void ARCoreInterface::_process() {
	if (m_init_status != INITIALISED) {
		return;
	} else if (m_ar_session == nullptr) {
		// don't have a session yet so...
		return;
	}

	if(ARCoreWrapper::has_viewport_changed()){
		ALOGV("Godot ARCore Interface: detected viewport change");
		
		m_display_orientation = static_cast<Orientation>(ARCoreWrapper::get_display_orientation());
		m_screen_width = ARCoreWrapper::get_display_width();
		m_screen_height = ARCoreWrapper::get_display_height();
		
		ALOGV("Orientation: %d, screen dims: %d x %d\n", m_display_orientation, m_screen_width, m_screen_height);
		
		ArSession_setDisplayGeometry(m_ar_session, (int32_t)(m_display_orientation), m_screen_width, m_screen_height);
		
		ARCoreWrapper::set_viewport_changed(false);
	}
	ArSession_setCameraTextureName(m_ar_session, m_background_renderer.getTextureId());

	if (ArSession_update(m_ar_session, m_ar_frame) != AR_SUCCESS) {
		ALOGW("ARCorePlugin: OnDrawFrame ARSession_update error");
	}

	ArCamera *ar_camera;
	ArFrame_acquireCamera(m_ar_session, m_ar_frame, &ar_camera);

	float view_mat[16];

	// https://developers.google.com/ar/reference/c/group/ar-camera#arcamera_getviewmatrix
	ArCamera_getViewMatrix(m_ar_session, ar_camera, view_mat);

	// https://docs.godotengine.org/en/stable/classes/class_transform3d.html

	// ALOGV("view_mat = \n"
	//       "[ %f, %f, %f, %f ]\n"
	//       "[ %f, %f, %f, %f ]\n"
	//       "[ %f, %f, %f, %f ]\n"
	//       "[ %f, %f, %f, %f ]",
	//       view_mat[0], view_mat[1], view_mat[2], view_mat[3],
	//       view_mat[4], view_mat[5], view_mat[6], view_mat[7],
	//       view_mat[8], view_mat[9], view_mat[10], view_mat[11],
	//       view_mat[12], view_mat[13], view_mat[14], view_mat[15]);

	m_view = to_godot_transform(view_mat);
	// We need to adjust this based on orientation
	if (m_vertical_orientation) {
		m_view.rotate(godot::Vector3(0.f, 0.f, 1.f), 3.1415f * 0.5f);
	}

	m_view.invert();

	if (m_head.is_valid()) {
		// Set our head position, note in real space, reference frame and world scale is applied later
		m_head->set_pose("default", m_view, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);
	}

	float projection_mat[16];
	ArCamera_getProjectionMatrix(m_ar_session, ar_camera,
			/*near=*/0.1f, /*far=*/1000.f,
			projection_mat);

	m_projection = to_godot_projection(projection_mat);

	m_background_renderer.process(*m_ar_session, *m_ar_frame, m_enable_depth_estimation);

	ArTrackingState camera_m_tracking_status;
	ArCamera_getTrackingState(m_ar_session, ar_camera, &camera_m_tracking_status);

	m_tracking_status = ar_tracking_state_to_tracking_status(*m_ar_session, *ar_camera, camera_m_tracking_status);

	ArCamera_release(ar_camera);

	// If the camera isn't tracking don't bother rendering other objects.
	if (camera_m_tracking_status != AR_TRACKING_STATE_TRACKING) {
		return;
	}

	if (m_enable_light_estimation) {
		estimateLight();
	}

	if (m_enable_vertical_plane_detection || m_enable_horizontal_plane_detection) {
		m_plane_tracker.process(m_ar_session, m_ar_frame);
	} else {
		m_plane_tracker.reset();
	}

	if (m_enable_images_detection) {
		m_image_tracker.process(*m_ar_session, *m_ar_frame);
		m_instances_renderer.process(*m_ar_session);
	} else {
		m_instances_renderer.clear();
	}

	if (m_enable_point_cloud_detection) {
		m_point_cloud_renderer.process(*m_ar_session);
	} else {
		m_point_cloud_renderer.clear();
	}
}

void ARCoreInterface::notification(int p_what) {
	ALOGE("MCT Godot ARCore: notification");
	// Needs testing, this should now be called

	// XRServer *xr_server = XRServer::get_singleton();
	// ERR_FAIL_NULL(xr_server);

	switch (p_what) {
		case MainLoop::NOTIFICATION_APPLICATION_RESUMED: {
			if (is_initialized()) {
				_resume();

				if (m_init_status == INITIALISE_FAILED) {
					// if (xr_server->get_primary_interface() == this) {
					//     xr_server->set_primary_interface(Ref<XRInterface>());
					// }
				}
			}
		}; break;
		case MainLoop::NOTIFICATION_APPLICATION_PAUSED:
			if (is_initialized()) {
				_pause();
			}
			break;
		default:
			break;
	}
}

void ARCoreInterface::configureSession() {
	ArConfig *ar_config = nullptr;
	ArConfig_create(m_ar_session, &ar_config);
	if (is_depth_supported() && m_enable_depth_estimation) {
		ArConfig_setDepthMode(m_ar_session, ar_config, AR_DEPTH_MODE_AUTOMATIC);
	} else {
		ArConfig_setDepthMode(m_ar_session, ar_config, AR_DEPTH_MODE_DISABLED);
	}

	if (m_enable_instant_placement) {
		ArConfig_setInstantPlacementMode(m_ar_session, ar_config,
				AR_INSTANT_PLACEMENT_MODE_LOCAL_Y_UP);
	} else {
		ArConfig_setInstantPlacementMode(m_ar_session, ar_config,
				AR_INSTANT_PLACEMENT_MODE_DISABLED);
	}

	if (m_enable_vertical_plane_detection && m_enable_horizontal_plane_detection) {
		ArConfig_setPlaneFindingMode(m_ar_session, ar_config, AR_PLANE_FINDING_MODE_HORIZONTAL_AND_VERTICAL);
	} else if (m_enable_vertical_plane_detection) {
		ArConfig_setPlaneFindingMode(m_ar_session, ar_config, AR_PLANE_FINDING_MODE_VERTICAL);
	} else if (m_enable_horizontal_plane_detection) {
		ArConfig_setPlaneFindingMode(m_ar_session, ar_config, AR_PLANE_FINDING_MODE_HORIZONTAL);
	} else if (m_enable_light_estimation) {
		ArConfig_setLightEstimationMode(m_ar_session, ar_config, AR_LIGHT_ESTIMATION_MODE_ENVIRONMENTAL_HDR); // Warning: not available with front facing camera
	} else {
		ArConfig_setPlaneFindingMode(m_ar_session, ar_config, AR_PLANE_FINDING_MODE_DISABLED);
	}

	ALOGV("Godot ARCore: Session reconfigured, active image db is now %p\n", m_image_tracker.getActiveDatabase());
	ArConfig_setAugmentedImageDatabase(m_ar_session, ar_config, m_image_tracker.getActiveDatabase());

	ERR_FAIL_NULL(ar_config);
	ERR_FAIL_COND(ArSession_configure(m_ar_session, ar_config) != AR_SUCCESS);
	ArConfig_destroy(ar_config);
}
