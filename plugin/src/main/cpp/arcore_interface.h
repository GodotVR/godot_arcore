//
// Created by luca on 18.08.24.
//

#ifndef ARCOREPLUGIN_ARCORE_INTERFACE_H
#define ARCOREPLUGIN_ARCORE_INTERFACE_H

#include "include/arcore_c_api.h"
#include <godot_cpp/classes/xr_interface_extension.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>

namespace godot {
    class ARCoreInterface : public XRInterfaceExtension {
        GDCLASS(ARCoreInterface, XRInterfaceExtension);

    protected:
        static void _bind_methods();

    public:
        enum InitStatus {
            NOT_INITIALISED,
            START_INITIALISE,
            INITIALISED,
            INITIALISE_FAILED
        };

        ARCoreInterface();
        ~ARCoreInterface();
        void _resume();
        void _pause();
        void start();

        // Method overrides from XRInterface
        virtual XRInterface::TrackingStatus _get_tracking_status() const override;

        virtual StringName _get_name() const override;

        virtual uint32_t _get_capabilities() const override;

        virtual int32_t _get_camera_feed_id() const override;

        virtual bool _is_initialized() const override;

        virtual bool _initialize() override;

        virtual void _uninitialize() override;

        virtual Vector2 _get_render_target_size() override;

        virtual uint32_t _get_view_count() override;

        virtual Transform3D _get_camera_transform() override;

        virtual Transform3D _get_transform_for_view(uint32_t p_view, const Transform3D &p_cam_transform) override;

        virtual PackedFloat64Array _get_projection_for_view(uint32_t p_view, double p_aspect, double p_z_near, double p_z_far) override;

        virtual void _post_draw_viewport(const RID &p_render_target, const Rect2 &p_screen_rect) override;

        virtual void _process() override;

    private:
        enum class Orientation: int32_t {
            UNKNOWN = -1, // Not initialized
            ROTATION_0 = 0, // Portrait mode
            ROTATION_90 = 1, // Counter-clockwise rotation of 90 degrees
            ROTATION_180 = 2, // Upside down
            ROTATION_270 = 3 // Counter-clockwise rotation of 270 degrees
        };

        InitStatus m_init_status;
        ArSession *m_ar_session;
        ArFrame *m_ar_frame;
        int m_screen_width;
        int m_screen_height;
        Orientation m_display_orientation;

        Ref<XRPositionalTracker> m_head;
        Transform3D m_view;
        Projection m_projection;
        float m_z_near;
        float m_z_far;

        TrackingStatus m_tracking_status;

        void configureSession();

        void handleScreenOrientationChange();
    };
};

#endif //ARCOREPLUGIN_ARCORE_INTERFACE_H
