#ifndef ARCORE_PLUGIN_PLANE_TRACKER
#define ARCORE_PLUGIN_PLANE_TRACKER

#include "include/arcore_c_api.h"
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>

namespace arcore_plugin {

typedef size_t PlaneId;

typedef struct {
    float mat4[16];
    godot::Array boundary_points; // array of vec3 with y=0
    
    uint64_t boundary_hash;

// In ArCore, a plane might get subsumed by another bigger, aligned plane, see https://developers.google.com/ar/reference/c/group/ar-plane#arplane_acquiresubsumedby
// When that happens, both planes keep being tracked as two different entities, but have the same data
// To more easily handle these cases, each (logical) plane has a unique ID and an associated parent ID
// If a plane is not subsumed by any other plane, its parent ID is its own ID, otherwise it is the ID of the plane that subsumes it
    uint32_t parent_plane_id;
} PlaneFrameData;


class PlaneTracker {
public:
    PlaneTracker();
    ~PlaneTracker();

    void initialize(ArSession *p_ar_session);
    void process(ArSession *p_ar_session, ArFrame *p_ar_frame);
    void reset();
    

    godot::Array getAllAvailablePlaneIds(){
        // maybe this array should be computed in process and cached?
        godot::Array arr;
        typedef godot::HashMap<uint64_t, uint32_t> map;
        for(map::Iterator itt = logicalPlaneHashToId.begin(); itt != logicalPlaneHashToId.end(); ++itt){
            //  maybe don't include id of subsumed planes?
            arr.append(itt->value);
        }
        return arr;
    }
    
    void getPlaneTransformMatrix(uint32_t p_plane_id, float **r_mat4) const{
        if(!plane_data_map.has(p_plane_id)){
            *r_mat4 = nullptr;
            return;
        } 
        
        uint32_t parent_plane_id = plane_data_map[p_plane_id]->parent_plane_id;
        if(parent_plane_id != p_plane_id){
            *r_mat4 = plane_data_map[parent_plane_id]->mat4;
            return;
        }

        *r_mat4 = plane_data_map[p_plane_id]->mat4;
    }
    godot::Array getPlaneBoundary(uint32_t p_plane_id) const{
        if(!plane_data_map.has(p_plane_id)) return godot::Array(); 
        
        uint32_t parent_plane_id = plane_data_map[p_plane_id]->parent_plane_id;
        if(parent_plane_id != p_plane_id)
            return plane_data_map[parent_plane_id]->boundary_points;
        
        return plane_data_map[p_plane_id]->boundary_points;
    }
    uint32_t getPlaneParentId(uint32_t p_plane_id) const {
        if(plane_data_map.has(p_plane_id)) return plane_data_map[p_plane_id]->parent_plane_id;
        return UINT32_MAX;
    }
    

private:

godot::HashMap<uint64_t, uint32_t> logicalPlaneHashToId;
godot::HashMap<uint32_t, PlaneFrameData*> plane_data_map;


uint32_t trackNewPlane(uint64_t plane_hash);
void handleLostPlane(uint64_t plane_hash);

};
}


#endif