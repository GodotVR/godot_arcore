#ifndef ARCORE_PLUGING_POINT_CLOUD_RENDERER
#define ARCORE_PLUGING_POINT_CLOUD_RENDERER

#include "include/arcore_c_api.h"
#include <godot_cpp/classes/csg_polygon3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

namespace arcore_plugin {
class PointCloudRenderer {
public:
	PointCloudRenderer();
	~PointCloudRenderer();

	void clear();
	void process(ArSession &p_ar_session);

private:
	// godot::Node* m_point_cloud_node {nullptr};
};
} // namespace arcore_plugin

#endif // ARCORE_PLUGING_POINT_CLOUD_RENDERER
