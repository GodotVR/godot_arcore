#include "plane_renderer.h"

#include <functional>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

#include "ar_utils.h"
#include "utils.h"

using namespace arcore_plugin;
using namespace godot;

PlaneRenderer::PlaneRenderer() {
}

PlaneRenderer::~PlaneRenderer() {
}

void PlaneRenderer::initialize() {
}

size_t hash_vector(const std::vector<float> &vec) {
	size_t hash = 0;
	for (const auto &element : vec) {
		// Combine the hash value of each element using std::hash
		hash ^= std::hash<float>{}(element) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
	return hash;
}

// Function to get the boundary polygon of an ArPlane
std::vector<float> get_plane_boundary(ArSession *ar_session, ArPlane *ar_plane) {
	int32_t polygon_size;
	ArPlane_getPolygonSize(ar_session, ar_plane, &polygon_size);
	std::vector<float> polygon(polygon_size);
	ArPlane_getPolygon(ar_session, ar_plane, polygon.data());
	return polygon;
}

godot::Quaternion get_plane_rotation(ArSession *ar_session, ArPose *plane_pose) {
	float plane_pose_raw[7] = { 0.f };
	ArPose_getPoseRaw(ar_session, plane_pose, plane_pose_raw);
	godot::Quaternion plane_quaternion(plane_pose_raw[3], plane_pose_raw[0],
			plane_pose_raw[1], plane_pose_raw[2]);
	// Get normal vector, normal is defined to be positive Y-position in local
	// frame.
	return plane_quaternion;
}

godot::Vector3 get_plane_normal(ArSession *ar_session, ArPose *plane_pose) {
	// Get normal vector, normal is defined to be positive Y-position in local
	// frame.
	godot::Quaternion quat = get_plane_rotation(ar_session, plane_pose);
	return godot::Vector3(0.f, 1.f, 0.f).rotated(quat.get_axis(), quat.get_angle());
}

// Function to convert the boundary points to Godot vertices
Array convert_boundary_to_vertices(const std::vector<float> &boundary) {
	Array vertices;
	for (size_t i = 0; i < boundary.size(); i += 2) {
		Vector2 vertex(boundary[i], boundary[i + 1]);
		vertices.append(vertex);
	}
	return vertices;
}

// // Function to create a polygon mesh from vertices
// Ref<ArrayMesh> create_polygon_mesh(const Array& vertices) {
//     Ref<ArrayMesh> mesh = memnew(ArrayMesh);
//     Array arrays;
//     arrays.resize(Mesh::ARRAY_MAX);
//     arrays[Mesh::ARRAY_VERTEX] = vertices;
//     mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
//     return mesh;
// }

// // Function to create a MeshInstance3D and set the mesh
// MeshInstance3D* create_mesh_instance(const Ref<Mesh>& mesh) {
//     MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
//     mesh_instance->set_mesh(mesh);
//     return mesh_instance;
// }

CSGPolygon3D *create_csg_polygon(const Array &vertices) {
	CSGPolygon3D *csg_polygon = memnew(CSGPolygon3D);
	csg_polygon->set_polygon(vertices);
	csg_polygon->set_depth(0.01f); // Set the depth of the polygon extrusion

	return csg_polygon;
}

// Function to remove the CSGPolygon3D node of a lost plane
void PlaneRenderer::remove_plane_node(const size_t &plane_guid) {
	if (m_plane_nodes.find(plane_guid) != m_plane_nodes.end()) {
		// Plane node found, remove it from the scene
		CSGPolygon3D *polygon_node = m_plane_nodes[plane_guid];
		polygon_node->queue_free();
		m_plane_nodes.erase(plane_guid);
	}
}

// Function to update the CSGPolygon3D node of a tracked plane
void PlaneRenderer::update_or_create_plane_node(const size_t &plane_guid, ArSession *ar_session, ArPlane *ar_plane, const std::vector<float> &boundary) {
	Array vertices = convert_boundary_to_vertices(boundary);
	ScopedArPose scopedArPose(ar_session);
	if (m_plane_nodes.find(plane_guid) != m_plane_nodes.end()) {
		// Plane node found, update its position and shape
		CSGPolygon3D *polygon_node = m_plane_nodes[plane_guid];

		polygon_node->set_polygon(vertices);

		ArPlane_getCenterPose(ar_session, ar_plane, scopedArPose.GetArPose());

		float mat4[16];
		ArPose_getMatrix(ar_session, scopedArPose.GetArPose(), mat4);
		Transform3D transform;

		// CSGPloygons are aligned on the XY axis while ARPlanes are aligned on the XZ axis
		transform.rotate(Vector3(1.f, 0.f, 0.f), 1.57f);
		transform = to_godot_transform(mat4) * transform;

		polygon_node->set_transform(transform);
	} else {
		CSGPolygon3D *polygon_node = create_csg_polygon(vertices);

		// Get the center pose of the plane
		ArPlane_getCenterPose(ar_session, ar_plane, scopedArPose.GetArPose());
		float mat4[16];
		ArPose_getMatrix(ar_session, scopedArPose.GetArPose(), mat4);
		Transform3D transform;

		// CSGPloygons are aligned on the XY axis while ARPlanes are aligned on the XZ axis
		transform.rotate(Vector3(1.f, 0.f, 0.f), 1.57f);
		transform = to_godot_transform(mat4) * transform;

		polygon_node->set_transform(transform);

		m_planes_node->add_child(polygon_node);
		m_plane_nodes[plane_guid] = polygon_node;
	}
}

Node *get_root_node() {
	// Get the SceneTree singleton
	SceneTree *scene_tree = (SceneTree *)Engine::get_singleton()->get_main_loop();

	// Get the current scene
	Node *current_scene = scene_tree->get_current_scene();

	return current_scene;
}

// void PlaneRenderer::clear()
// {
//     if (m_planes_node == nullptr) {
//         return;
//     }

//     while(m_planes_node->get_child_count() != 0)
//     {
//         Node* node = m_planes_node->get_child(0);
//         if (node)
//         {
//             m_planes_node->remove_child(node);
//             node->queue_free();
//         }
//     }
// }

size_t get_uid(ArSession *ar_session, const std::vector<float> &boundary) {
	size_t boundary_hash = hash_vector(boundary);
	return boundary_hash;
}

void PlaneRenderer::process(ArSession &p_ar_session) {
	if (m_planes_node == nullptr) {
		m_planes_node = memnew(Node);
		m_planes_node->set_name("ARPlanes_node");
		get_root_node()->add_child(m_planes_node);
	}

	// Update and render planes.
	ArTrackableList *plane_list = nullptr;
	ArTrackableList_create(&p_ar_session, &plane_list);
	ERR_FAIL_NULL(plane_list);

	ArTrackableType plane_tracked_type = AR_TRACKABLE_PLANE;
	ArSession_getAllTrackables(&p_ar_session, plane_tracked_type, plane_list);

	int32_t plane_list_size = 0;
	ArTrackableList_getSize(&p_ar_session, plane_list, &plane_list_size);

	std::vector<size_t> detected_guid;
	for (int i = 0; i < plane_list_size; ++i) {
		ArTrackable *ar_trackable = nullptr;
		ArTrackableList_acquireItem(&p_ar_session, plane_list, i, &ar_trackable);
		ArPlane *ar_plane = ArAsPlane(ar_trackable);
		ArTrackingState out_m_tracking_status;
		ArTrackable_getTrackingState(&p_ar_session, ar_trackable,
				&out_m_tracking_status);

		ArPlane *subsume_plane;
		ArPlane_acquireSubsumedBy(&p_ar_session, ar_plane, &subsume_plane);
		if (subsume_plane != nullptr) {
			ArTrackable_release(ArAsTrackable(subsume_plane));
			ArTrackable_release(ar_trackable);
			continue;
		}

		if (ArTrackingState::AR_TRACKING_STATE_TRACKING != out_m_tracking_status) {
			ArTrackable_release(ar_trackable);
			continue;
		}

		std::vector<float> boundary = get_plane_boundary(&p_ar_session, ar_plane);
		size_t plane_guid = get_uid(&p_ar_session, boundary);
		detected_guid.push_back(plane_guid);

		update_or_create_plane_node(plane_guid, &p_ar_session, ar_plane, boundary);

		ArTrackable_release(ar_trackable);
	}

	// Removing all lost planes
	std::vector<size_t> to_remove;
	using vec = std::unordered_map<size_t, godot::CSGPolygon3D *>;
	for (vec::const_iterator it = m_plane_nodes.cbegin(); it != m_plane_nodes.cend(); ++it) {
		size_t uid = it->first;
		if (std::find(detected_guid.begin(), detected_guid.end(), uid) == detected_guid.end()) {
			to_remove.push_back(uid);
		}
	}
	for (int i = 0; i < to_remove.size(); ++i) {
		Node *node = m_plane_nodes[to_remove[i]];
		if (node) {
			node->queue_free();
		}
		m_plane_nodes.erase(to_remove[i]);
	}

	ArTrackableList_destroy(plane_list); //! Maxime: Might need to change that
}

void PlaneRenderer::clear() {
	if (get_root_node() && m_planes_node != nullptr) {
		get_root_node()->remove_child(m_planes_node);
		memdelete(m_planes_node);
		m_planes_node = nullptr;
	}
}
