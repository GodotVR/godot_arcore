#include "plane_tracker.h"

#include "ar_utils.h"
#include "utils.h"

namespace arcore_plugin{

PlaneTracker::PlaneTracker(){}
PlaneTracker::~PlaneTracker(){
    // TODO: release all trackers
    typedef godot::HashMap<uint32_t, PlaneFrameData*> map;

    for(map::Iterator itt = plane_data_map.begin(); itt != plane_data_map.end(); ++itt){
        memfree(itt->value);
    }
}

void PlaneTracker::initialize(ArSession *p_ar_session){

}

uint64_t logical_plane_hash(const ArPlane *ar_plane){
	// ArPlanes are long lived object
	// i.e. the pointer to one (logical) plane shouldn't change
	return reinterpret_cast<uint64_t>(ar_plane);
}


uint64_t physical_plane_hash(float *boundary, int32_t n_points) {
	// The data associated with one logical plane (which describes the physical plane)
	// might change during the planes lifetime, and thus this hash too
	uint64_t hash = 0;
    for (int32_t i =0; i < n_points; i++) {
        float elem = boundary[i];
        
		hash ^= godot::hash_djb2_one_float(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
	return hash;
}




// Function to get the boundary polygon of an ArPlane
void get_plane_boundary(ArSession *p_ar_session, ArPlane *p_ar_plane, float **r_elems, int32_t *r_n) {
	ArPlane_getPolygonSize(p_ar_session, p_ar_plane, r_n);
	*r_elems = (float*) memalloc((*r_n)*sizeof(float));
	ArPlane_getPolygon(p_ar_session, p_ar_plane, *r_elems);
}



void PlaneTracker::process(ArSession *p_ar_session, ArFrame *p_ar_frame){

    // Update and render planes.
	ArTrackableList *plane_list = nullptr;
	ArTrackableList_create(p_ar_session, &plane_list);
	ERR_FAIL_NULL(plane_list);

	ArSession_getAllTrackables(p_ar_session, AR_TRACKABLE_PLANE, plane_list);

	int32_t plane_list_size = 0;
	ArTrackableList_getSize(p_ar_session, plane_list, &plane_list_size);

	godot::HashSet<uint64_t> detected_hashes;
	ALOGV("Godot ARCore: Detected %d trackable planes\n", plane_list_size);
	for (int i = 0; i < plane_list_size; ++i) {
		ArTrackable *ar_trackable = nullptr;
		ArTrackableList_acquireItem(p_ar_session, plane_list, i, &ar_trackable);
		ArPlane *ar_plane = ArAsPlane(ar_trackable);
		
		ArTrackingState out_m_tracking_status;
		ArTrackable_getTrackingState(p_ar_session, ar_trackable,
				&out_m_tracking_status);

        uint64_t plane_hash = logical_plane_hash(ar_plane);
        
		detected_hashes.insert(plane_hash);

		uint32_t plane_id = 0;
		if(logicalPlaneHashToId.has(plane_hash)){
			plane_id = logicalPlaneHashToId[plane_hash];
		} else {
			ALOGV("Brand new plane, setting it up...");
			plane_id = trackNewPlane(plane_hash);
			ALOGV("Registered plane hash %ld with id %d", plane_hash, plane_id);
		}
		PlaneFrameData* plane_data = plane_data_map[plane_id];

        ArPlane *parent_plane;
        ArPlane_acquireSubsumedBy(p_ar_session, ar_plane, &parent_plane);
        if (parent_plane != nullptr) {
			ALOGV("PLANE SUBSUMED");
			uint32_t parent_hash = logical_plane_hash(parent_plane);
			
			uint32_t parent_id = 0;
			// I am not sure this can happen, but handle it anyway
			if(!logicalPlaneHashToId.has(parent_hash)){
				parent_id = trackNewPlane(parent_hash);
			} else {
				parent_id = logicalPlaneHashToId[parent_hash];
			}
			ALOGV("Plane %d is being subsumed by plane %d\n", plane_id, parent_id);

			plane_data->parent_plane_id = parent_id;

			ArTrackable_release(ArAsTrackable(parent_plane));
			ArTrackable_release(ar_trackable);
			continue;
		}

		ALOGV("Plane %d not subsumed, need to update its data", plane_id);

		// Not subsumed, its own parent
		// Set it every process call in case subsumed planes get unsubsumed (dunno if that is possible but...)
		plane_data->parent_plane_id = plane_id;


		if (out_m_tracking_status != ArTrackingState::AR_TRACKING_STATE_TRACKING) {
			ALOGV("Godot ARCore: plane not being tracked");
            ArTrackable_release(ar_trackable);
			continue;
		}

		float* boundary; int32_t n_vals;
        get_plane_boundary(p_ar_session, ar_plane, &boundary, &n_vals);
        uint64_t new_boundary_hash = physical_plane_hash(boundary, n_vals);
		if(new_boundary_hash != plane_data->boundary_hash){
			ALOGV("Updating boundary, arcore boundary has %d values", n_vals);
			godot::Array arr = plane_data->boundary_points;
			
			arr.clear(); arr.resize(n_vals/2);
			for(int pt = 0; pt < n_vals/2; pt++){
				arr[pt] = godot::Vector3(boundary[pt*2], 0, boundary[pt*2 + 1]);
			}
		}

		{
			ALOGV("Updating transform");
			ScopedArPose scoped_pose(p_ar_session);
			ArPlane_getCenterPose(p_ar_session, ar_plane, scoped_pose.GetArPose());
			ArPose_getMatrix(p_ar_session, scoped_pose.GetArPose(), plane_data->mat4);
		}
		
		ArTrackable_release(ar_trackable);
	}

	godot::List<uint64_t> allTrackedHashes;
	typedef godot::HashMap<uint64_t, uint32_t> map;
	for(map::Iterator itt = logicalPlaneHashToId.begin(); itt != logicalPlaneHashToId.end(); ++itt){
		if( !detected_hashes.has(itt->key)){
			handleLostPlane(itt->key);
		}
	}



}

void PlaneTracker::reset(){
	typedef godot::HashMap<uint64_t, uint32_t> map;
	for(map::Iterator itt = logicalPlaneHashToId.begin(); itt != logicalPlaneHashToId.end(); ++itt){
		handleLostPlane(itt->key);
	}
}


void PlaneTracker::handleLostPlane(uint64_t plane_hash){
	// TODO release godot::ArPositionalTracker
	uint32_t plane_id = logicalPlaneHashToId[plane_hash];
	memfree(plane_data_map[plane_id]);
	//! If a plane that subsumes others gets lost but the planes that it subsumes are not lost
	//! Big problems, cross fingers
	plane_data_map.erase(plane_id);
	logicalPlaneHashToId.erase(plane_hash);
}

uint32_t PlaneTracker::trackNewPlane(uint64_t plane_hash){
    static uint32_t next_plane_id = 0;
	
	// TODO instantiate new godot::ArPositionalTracker
	uint32_t plane_id = next_plane_id;
	next_plane_id++;
	logicalPlaneHashToId.insert(plane_hash, plane_id);
	
	PlaneFrameData *data = (PlaneFrameData*) memalloc(sizeof(PlaneFrameData));
	
	data->boundary_hash = 0;
	data->boundary_points = godot::Array();
	data->parent_plane_id = UINT32_MAX;
	
	plane_data_map.insert(plane_id, data);
	return plane_id;
}



}