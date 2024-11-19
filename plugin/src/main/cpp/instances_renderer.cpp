#include "instances_renderer.h"
#include "utils.h"
#include <godot_cpp/classes/camera_server.hpp>

using namespace arcore_plugin;
using namespace godot;

InstancesRenderer::InstancesRenderer() {
}

InstancesRenderer::~InstancesRenderer() {
}

void InstancesRenderer::process(ArSession &p_ar_session) {
}

void InstancesRenderer::clear() {
}

void InstancesRenderer::set_node_images_mapping(const Dictionary &in_nodeImagesMap) {
	clear();
	m_nodeImagesMap = in_nodeImagesMap;
}
