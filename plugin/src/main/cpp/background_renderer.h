#ifndef ARCORE_PLUGING_BACKGROUND_RENDERER
#define ARCORE_PLUGING_BACKGROUND_RENDERER

#include "godot_cpp/classes/camera_feed.hpp"
#include "include/arcore_c_api.h"

namespace arcore_plugin {
class BackgroundRenderer {
public:
	BackgroundRenderer();
	~BackgroundRenderer();

	void initialize(int p_width, int p_height);

	void uninitialize();

	void process(ArSession &p_ar_session, const ArFrame &p_ar_frame, bool p_enable_depth_estimation);

	int64_t getTextureId() const { return m_camera_texture_id; }

	godot::Ref<godot::CameraFeed> getFeed() const { return m_feed; }

private:
	godot::Ref<godot::CameraFeed> m_feed;

	int64_t m_camera_texture_id;
};
} // namespace arcore_plugin
#endif // ARCORE_PLUGING_BACKGROUND_RENDERER
