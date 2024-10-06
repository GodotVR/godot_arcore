#ifndef ARCORE_PLUGING_INSTANCES_RENDERER
#define ARCORE_PLUGING_INSTANCES_RENDERER

#include "include/arcore_c_api.h"
#include <godot_cpp/classes/csg_polygon3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

namespace arcore_plugin {
class InstancesRenderer {
public:
	InstancesRenderer();
	~InstancesRenderer();

	void clear();
	void process(ArSession &p_ar_session);

	void set_node_images_mapping(const godot::Dictionary &in_nodeImagesMap);

private:
	// godot::Node* m_instances_node {nullptr};

	std::unordered_map<size_t, godot::Node3D *> m_instances_nodes;

	godot::Dictionary m_nodeImagesMap;
};
} // namespace arcore_plugin

#endif // ARCORE_PLUGING_INSTANCES_RENDERER
