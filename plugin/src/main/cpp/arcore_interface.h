//
// Created by Patrick on 07.09.2023.
// Modified by luca on 18.08.24.
// Modified by Maxime on 08.09.24.
//

#ifndef ARCOREPLUGIN_ARCORE_INTERFACE_H
#define ARCOREPLUGIN_ARCORE_INTERFACE_H

#include "godot_cpp/variant/projection.hpp"
#include "include/arcore_c_api.h"
#include <godot_cpp/classes/xr_interface_extension.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>

#include "background_renderer.h"
#include "instances_renderer.h"
#include "plane_renderer.h"
#include "point_cloud_renderer.h"

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
	virtual ~ARCoreInterface();
	void start();
	void _resume();
	void _pause();

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

	virtual void notification(int p_what);

	void enable_depth_estimation(bool p_enable);
	bool is_depth_supported();
	void show_depth_map(bool p_enable);
	void set_max_depth_meters(float p_max_depth_meters);
	void enable_vertical_plane_detection(bool p_enable);
	void enable_horizontal_plane_detection(bool p_enable);
	void enable_instant_placement(bool p_enable);
	godot::Transform3D getHitPose(float p_pixel_x, float p_pixel_y, float p_approximate_distance_meters);
	godot::Transform3D getHitRayPose(const godot::Vector3 &p_origin, const godot::Vector3 &p_direction);
	void enable_images_detection(bool p_enable);
	void set_node_images_mapping(const godot::Dictionary &in_nodeImagesMap);
	void enable_point_cloud_detection(bool p_enable);
	void switch_orientation(bool p_vertical);
	void enable_light_estimation(bool p_enable);
	godot::Vector4 get_light_color_correction();
	godot::Vector3 get_light_main_hdr_direction();
	godot::Vector3 get_light_main_hdr_intensity();

	float getNear() { return m_z_near; }
	float getFar() { return m_z_far; }

private:
	enum class Orientation : int32_t {
		UNKNOWN = -1, // Not initialized
		ROTATION_0 = 0, // Portrait mode
		ROTATION_90 = 1, // Counter-clockwise rotation of 90 degrees
		ROTATION_180 = 2, // Upside down
		ROTATION_270 = 3 // Counter-clockwise rotation of 270 degrees
	};

	arcore_plugin::BackgroundRenderer m_background_renderer;
	arcore_plugin::PlaneRenderer m_plane_renderer;
	arcore_plugin::InstancesRenderer m_instances_renderer;
	arcore_plugin::PointCloudRenderer m_point_cloud_renderer;

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

	bool m_enable_depth_estimation;
	bool m_enable_vertical_plane_detection;
	bool m_enable_horizontal_plane_detection;
	bool m_enable_instant_placement;
	bool m_enable_images_detection;
	bool m_enable_point_cloud_detection;
	bool m_enable_light_estimation;
	bool m_vertical_orientation;
	godot::Vector4 m_color_correction;
	godot::Vector3 m_light_env_hdr_main_light_direction;
	godot::Vector3 m_light_env_hdr_main_light_intensity;

	void configureSession();

	void handleScreenOrientationChange();

	void estimateLight();
};
}; //namespace godot

#endif //ARCOREPLUGIN_ARCORE_INTERFACE_H
