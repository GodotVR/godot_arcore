/*************************************************************************/
/*  arcore_interface.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2023 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2023 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef ARCORE_INTERFACE_H
#define ARCORE_INTERFACE_H

#include <map>

#include "godot_cpp/classes/xr_interface_extension.hpp"
#include "godot_cpp/classes/xr_positional_tracker.hpp"
#include "godot_cpp/classes/camera_feed.hpp"
#include "godot_cpp/variant/projection.hpp"

#include "include/arcore_c_api.h"

/**
	@author Bastiaan Olij <mux213@gmail.com>, Robert Hofstra <robert.hofstra@knowlogy.nl>
	ARCore interface between Android and Godot
**/

namespace godot {

class ARCoreInterface : public XRInterfaceExtension {
	GDCLASS(ARCoreInterface, XRInterfaceExtension);

public:
	enum InitStatus {
		NOT_INITIALISED, // We're not initialised
		START_INITIALISE, // We just started our initialise process
		INITIALISED, // Yeah! we are up and running
		INITIALISE_FAILED // We failed to initialise
	};

	static void _bind_methods();

	ARCoreInterface();
	virtual ~ARCoreInterface();

	virtual XRInterface::TrackingStatus _get_tracking_status() const override;

	void _resume();

	void _pause();

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

private:
	InitStatus init_status;

	ArSession *ar_session;
	ArFrame *ar_frame;
	int width;
	int height;
	int display_rotation;
	uint camera_texture_id;
	uint last_anchor_id;

	Ref<CameraFeed> feed;

	Ref<XRPositionalTracker> head;
	Transform3D view;
	Projection projection;
	float z_near, z_far;
	bool have_display_transform;

	struct anchor_map {
		Ref<XRPositionalTracker> tracker;
		bool stale;
	};

	TrackingStatus tracking_state;

	std::map<ArPlane *, anchor_map *> anchors;

	void make_anchors_stale();

	void remove_stale_anchors();
};

} // namespace godot

#endif /* !ARCORE_INTERFACE_H */
