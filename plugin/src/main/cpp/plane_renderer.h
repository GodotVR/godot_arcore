#ifndef ARCORE_PLUGING_PLANE_RENDERER
#define ARCORE_PLUGING_PLANE_RENDERER

#include "include/arcore_c_api.h"
#include <godot_cpp/classes/csg_polygon3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

namespace arcore_plugin {
class PlaneRenderer {
public:
	PlaneRenderer();
	~PlaneRenderer();

	void initialize();

	void clear();
	void process(ArSession &p_ar_session);

private:
	godot::Node *m_planes_node{ nullptr };

	std::unordered_map<size_t, godot::CSGPolygon3D *> m_plane_nodes;

	void remove_plane_node(const size_t &plane_guid);

	void update_or_create_plane_node(const size_t &plane_guid, ArSession *ar_session, ArPlane *ar_plane, const std::vector<float> &boundary);
};
} // namespace arcore_plugin

#endif // ARCORE_PLUGING_PLANE_RENDERER
