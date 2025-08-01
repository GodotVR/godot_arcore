#ifndef ARCORE_PLUGING_INSTANCES_RENDERER
#define ARCORE_PLUGING_INSTANCES_RENDERER

#include "include/arcore_c_api.h"
#include <godot_cpp/classes/csg_polygon3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include "image_tracker.h"

namespace arcore_plugin {


// Tracks ArAnchors (https://developers.google.com/ar/reference/c/group/ar-anchor)
// And maps Godot Nodes to them, taking care of transforms
class InstancesRenderer {
public:
	InstancesRenderer();
	~InstancesRenderer();

	void clear();
	void process(ArSession &p_ar_session);

	void set_node_images_mapping(ArSession *p_ar_session, const godot::Dictionary &in_nodeImagesMap);

	void initalize(ImageTracker *image_tracker){m_image_tracker = image_tracker;}

private:
	ImageTracker *m_image_tracker;

	//godot::Node* m_instances_node {nullptr};

	godot::HashMap<godot::String, godot::Node3D *> m_instances_nodes;

	godot::Dictionary m_nodeImagesMap;
};
} // namespace arcore_plugin

#endif // ARCORE_PLUGING_INSTANCES_RENDERER
