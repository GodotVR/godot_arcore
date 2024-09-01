//
// Created by luca on 18.08.24.
//

#include "arcore_interface.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include "utils.h"
#include "arcore_wrapper.h"

using namespace godot;

void ARCoreInterface::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_resume"), &ARCoreInterface::_resume);
    ClassDB::bind_method(D_METHOD("_pause"), &ARCoreInterface::_pause);
    ClassDB::bind_method(D_METHOD("start"), &ARCoreInterface::start);
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

    // This is just a default and will be changed by ARCore
    m_projection.set_perspective(60.0, 1.0, m_z_near, m_z_far, false);
}

ARCoreInterface::~ARCoreInterface() {
    // remove_all_anchors();

    if (is_initialized()) {
        uninitialize();
    }
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
            Size2 size = DisplayServer::get_singleton()->screen_get_size();

            if (size.x > size.y) {
                m_screen_width = size.y;
                m_screen_height = size.x;
            } else {
                m_screen_width = size.x;
                m_screen_height = size.y;
            }

            // Trigger a display rotation
            m_display_orientation = Orientation::UNKNOWN;

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

        // m_background_renderer.initialize(m_screen_width, m_screen_height);

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

        // m_background_renderer.uninitialize();

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

XRInterface::TrackingStatus ARCoreInterface::_get_tracking_status() const {
    return m_tracking_status;
}

StringName ARCoreInterface::_get_name() const {
    StringName name("ARCoreInterface");
    return name;
}

uint32_t ARCoreInterface::_get_capabilities() const {
    return XR_MONO + XR_AR;
}

int32_t ARCoreInterface::_get_camera_feed_id() const {
    //if (m_background_renderer.getFeed().is_valid()) {
    //    return m_background_renderer.getFeed()->get_id();
    //} else {
        return 0;
    //}
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

Transform3D ARCoreInterface::_get_transform_for_view(uint32_t p_view,
                                                     const Transform3D &p_cam_transform) {
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
        transform_for_eye = p_cam_transform;
    }

    return transform_for_eye;
}

PackedFloat64Array ARCoreInterface::_get_projection_for_view(uint32_t p_view, double p_aspect,
                                                             double p_z_near, double p_z_far) {
    PackedFloat64Array arr;
    arr.resize(16); // 4x4 matrix

    // Remember our near and far, we'll use it next frame
    m_z_near = p_z_near;
    m_z_far = p_z_far;

    real_t *p = (real_t *)&m_projection.columns;

    for(int i = 0; i < 16; i++) {
        arr[i] = p[i];
    }

    return arr;
}


void ARCoreInterface::configureSession() {
    ArConfig* ar_config = nullptr;
    ArConfig_create(m_ar_session, &ar_config);

    ArConfig_setDepthMode(m_ar_session, ar_config, AR_DEPTH_MODE_AUTOMATIC);
    ArConfig_setPlaneFindingMode(m_ar_session, ar_config, AR_PLANE_FINDING_MODE_HORIZONTAL);

    ERR_FAIL_NULL(ar_config);
    ERR_FAIL_COND(ArSession_configure(m_ar_session, ar_config) == AR_SUCCESS);
    ArConfig_destroy(ar_config);
}

void ARCoreInterface::_post_draw_viewport(const RID &p_render_target, const Rect2 &p_screen_rect)
{
    // We must have a valid render target
    ERR_FAIL_COND(!p_render_target.is_valid());

    // Because we are rendering to our device we must use our main viewport!
    ERR_FAIL_COND(p_screen_rect == Rect2());

    Rect2 src_rect(0.0f, 0.0f, 1.0f, 1.0f);
    Rect2 dst_rect = p_screen_rect;

    add_blit(p_render_target, src_rect, dst_rect, false, 0, false, Vector2(), 0, 0, 0.0, 1.0);
}

void ARCoreInterface::_process() {
    if (m_init_status != INITIALISED) {
        return;
    } else if (m_ar_session == nullptr) {
        return;
    }

    ArSession_setDisplayGeometry(m_ar_session, (int32_t)(Orientation::ROTATION_0), m_screen_height, m_screen_width);
    ArSession_setCameraTextureName(m_ar_session, _get_camera_feed_id());

    if (ArSession_update(m_ar_session, m_ar_frame) != AR_SUCCESS) {
        ALOGW("ARCorePlugin: OnDrawFrame ARSession_update error");
    }

    ArCamera *ar_camera;
    ArFrame_acquireCamera(m_ar_session, m_ar_frame, &ar_camera);

    // TODO: Getting the math right here (probably without glm dependency?)
    // Right-handed column major, right?

    float view_mat[16];

    // https://developers.google.com/ar/reference/c/group/ar-camera#arcamera_getviewmatrix
    ArCamera_getViewMatrix(m_ar_session, ar_camera, view_mat);

    // https://docs.godotengine.org/en/stable/classes/class_transform3d.html

    ALOGV("view_mat = \n"
          "[ %f, %f, %f, %f ]\n"
          "[ %f, %f, %f, %f ]\n"
          "[ %f, %f, %f, %f ]\n"
          "[ %f, %f, %f, %f ]",
          view_mat[0], view_mat[1], view_mat[2], view_mat[3],
          view_mat[4], view_mat[5], view_mat[6], view_mat[7],
          view_mat[8], view_mat[9], view_mat[10], view_mat[11],
          view_mat[12], view_mat[13], view_mat[14], view_mat[15]);

    m_view = Transform3D(
            Vector3(view_mat[0], view_mat[1], view_mat[2]),
            Vector3(view_mat[4], view_mat[5], view_mat[6]),
            Vector3(view_mat[8], view_mat[9], view_mat[10]),
            Vector3(view_mat[12], view_mat[13], view_mat[14])
    );

    m_view.invert();

    if (m_head.is_valid()) {
        // Set our head position, note in real space, reference frame and world scale is applied later
        m_head->set_pose("default", m_view, Vector3(), Vector3(), XRPose::XR_TRACKING_CONFIDENCE_HIGH);
    }

    // === Get planes
    /*ArTrackableList* ar_trackable_list = nullptr;
    ArTrackableList_create(m_ar_session, &ar_trackable_list);
    ArFrame_getUpdatedTrackables(m_ar_session, m_ar_frame, AR_TRACKABLE_PLANE, ar_trackable_list);

    int32_t image_list_size;
    ArTrackableList_getSize(m_ar_session, ar_trackable_list, &image_list_size);

    for (int i = 0; i < image_list_size; ++i) {
        ArTrackable* ar_trackable = nullptr;
        ArTrackableType trackable_type;
        ArTrackableList_acquireItem(m_ar_session, ar_trackable_list, i,
                                    &ar_trackable);
        ArTrackable_getType(m_ar_session, ar_trackable, &trackable_type);
        if (trackable_type == AR_TRACKABLE_PLANE) {
            ArPlane* plane = ArAsPlane(ar_trackable);
            ALOGV("Trackable %d is a Plane", i);
        }

        ArTrackable_release(ar_trackable);
    }*/
    // ===

    //m_background_renderer.process(*m_ar_session, *m_ar_frame, m_enable_depth_estimation);

    ArTrackingState camera_m_tracking_state;
    ArCamera_getTrackingState(m_ar_session, ar_camera, &camera_m_tracking_state);

    switch (camera_m_tracking_state) {
        case AR_TRACKING_STATE_TRACKING:
            m_tracking_status = XRInterface::XR_NORMAL_TRACKING;
            break;
        case AR_TRACKING_STATE_PAUSED:
            m_tracking_status = XRInterface::XR_NOT_TRACKING;
            break;
        case AR_TRACKING_STATE_STOPPED:
            m_tracking_status = XRInterface::XR_NOT_TRACKING;
            break;
        default:
            m_tracking_status = XRInterface::XR_UNKNOWN_TRACKING;
            break;
    }

    ArCamera_release(ar_camera);

    // If the camera isn't tracking don't bother rendering other objects.
    if (camera_m_tracking_state != AR_TRACKING_STATE_TRACKING) {
        return;
    }
}

void ARCoreInterface::_pause() {
    godot::UtilityFunctions::print("ARCoreInterface::_pause");
    ALOGV("ARCoreInterface::_pause");
}

void ARCoreInterface::_resume() {
    if (m_init_status == INITIALISED && m_ar_session != nullptr) {
        ArStatus status = ArSession_resume(m_ar_session);

        if(status != AR_SUCCESS) {
            ALOGE("ARCorePlugin: Failed to resume");
            // TODO: Quit or end the ARCore session gracefully
        }
    }
}

void ARCoreInterface::start() {
    ALOGV("ARCoreInterface::start()");
}

void ARCoreInterface::handleScreenOrientationChange() {

}
