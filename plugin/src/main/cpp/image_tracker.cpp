#include "image_tracker.h"

#include "include/arcore_c_api.h"
#include <godot_cpp/classes/image.hpp>

#include "utils.h"
#include "ar_utils.h"
using namespace arcore_plugin;
using namespace godot;


ImageTracker::ImageTracker(){
}

ImageTracker::~ImageTracker(){
    if(image_db != nullptr){
        ArAugmentedImageDatabase_destroy(image_db);
    }

    for(godot::HashMap<godot::String, ImageFrameTrackingData*>::Iterator itt = name_to_data.begin();
                itt != name_to_data.end(); ++itt){
        memfree(itt->value);
    };
}

void ImageTracker::initialize(ArSession *p_ar_session){
    ALOGV("Godot ARCore: Initializing ImageTracker\n");
    //ArAugmentedImageDatabase_create(p_ar_session, &image_db);
}

void ImageTracker::createFreshDatabase(ArSession *p_ar_session){
    //if(image_db != nullptr){
    //    ArAugmentedImageDatabase_destroy(image_db);
    //    image_db = nullptr;
    //}
    ALOGV("Godot ARCore: ImageTracker: Creating fresh database\n");
    ArAugmentedImageDatabase_create(p_ar_session, &image_db);
}

void ImageTracker::serializeDatabase(ArSession *p_ar_session, uint8_t **r_byte_array, int64_t *r_size){
    ArAugmentedImageDatabase_serialize(p_ar_session, image_db, r_byte_array, r_size);
}

void ImageTracker::loadSerializedDatabase(ArSession *p_ar_session, const uint8_t *r_byte_array, int64_t r_size){
    //if(image_db != nullptr){
    //    ArAugmentedImageDatabase_destroy(image_db);
    //    image_db = nullptr;
    //}
    ArAugmentedImageDatabase_deserialize(p_ar_session, r_byte_array, r_size, &image_db);
}

void ImageTracker::getImageTransformMatrix(ArSession *p_ar_session, const godot::String &p_img_name, float **r_mat4){
    *r_mat4 = name_to_data[p_img_name]->mat4;
}

bool ImageTracker::getImageTrackingStatus(ArSession *p_ar_session, const godot::String &p_img_name){
    return name_to_data[p_img_name]->validTracking;
}


void ImageTracker::process(ArSession &p_ar_session, ArFrame &p_ar_frame){
    ArTrackableList* updated_image_list = nullptr;
    ArTrackableList_create(&p_ar_session, &updated_image_list);
    ArFrame_getUpdatedTrackables(
			&p_ar_session, &p_ar_frame, AR_TRACKABLE_AUGMENTED_IMAGE, updated_image_list);

    int32_t image_list_size;
    ArTrackableList_getSize(&p_ar_session, updated_image_list, &image_list_size);
    int32_t totImg;
    ArAugmentedImageDatabase_getNumImages(&p_ar_session, image_db, &totImg);
    //ALOGV("Godot ARCore: Currently tracking a total of %d/%d images\n", image_list_size, totImg);
    for (int i = 0; i < image_list_size; ++i) {
        //ALOGV("Godot ARCore: tracked image Loop iter\n");
        ArTrackable* ar_trackable = nullptr;
        ArTrackableList_acquireItem(&p_ar_session, updated_image_list, i,
                                    &ar_trackable);
        ArAugmentedImage* image = ArAsAugmentedImage(ar_trackable);

        ArTrackingState tracking_state;
        ArTrackable_getTrackingState(&p_ar_session, ar_trackable, &tracking_state);

        int image_index;
        ArAugmentedImage_getIndex(&p_ar_session, image, &image_index);

        char* name;
        ArAugmentedImage_acquireName(&p_ar_session, image, &name);
        godot::String godotName = godot::String(name);
        ALOGV("Godot ARCore: Checking Tracking status for image %s\n", name);

        if (tracking_state == AR_TRACKING_STATE_TRACKING) {
            ALOGV("Godot ARCore: Tracked Image index %d is being tracked\n", image_index);
            ScopedArPose scopedArPose(&p_ar_session);
            ArAugmentedImage_getCenterPose(&p_ar_session, image,
                                        scopedArPose.GetArPose());

            ArAnchor* image_anchor = nullptr;
            ArTrackable_acquireNewAnchor(
					&p_ar_session, ar_trackable, scopedArPose.GetArPose(), &image_anchor);

            
            {
                ScopedArPose scopedArPose(&p_ar_session);
                ArAnchor_getPose(&p_ar_session, image_anchor, scopedArPose.GetArPose());

                ArPose_getMatrix(&p_ar_session, scopedArPose.GetArPose(), name_to_data[godotName]->mat4);
            
                godot::Vector3 pos = to_godot_position(name_to_data[godotName]->mat4);
                ALOGV("Godot ARCore: Position of tracked image %s updated to (%f, %f, %f)\n", name, pos.x, pos.y, pos.z);
            }

            name_to_data[godotName]->validTracking = true;
            
            // first time detecting this image
            if(name_to_data[godotName]->godotTracker.is_null()){
                name_to_data[godotName]->godotTracker.instantiate();
                name_to_data[godotName]->godotTracker->set_tracker_name(godotName);
                name_to_data[godotName]->godotTracker->set_tracker_type(godot::XRServer::TRACKER_ANCHOR);
                name_to_data[godotName]->godotTracker->set_tracker_desc(godot::String("Augmented Image tracked via ARCore plugin, image name is ") + godotName);
                godot::XRServer::get_singleton()->add_tracker(name_to_data[godotName]->godotTracker);
            }

            godot::Vector3 zero_vec = godot::Vector3(0,0,0);
            name_to_data[godotName]->godotTracker->set_pose("root", to_godot_transform(name_to_data[godotName]->mat4), 
                                                                zero_vec, zero_vec, godot::XRPose::TrackingConfidence::XR_TRACKING_CONFIDENCE_NONE);

        }
        else{
            name_to_data[godotName]->validTracking = false;
            if(name_to_data[godotName]->godotTracker.is_valid()){
                name_to_data[godotName]->godotTracker->invalidate_pose("root");
            }
            if(tracking_state == AR_TRACKING_STATE_PAUSED){
                ALOGV("Godot ARCore: Tracked Image %s has tracking PAUSED\n", name);
            }
            else if(tracking_state == AR_TRACKING_STATE_STOPPED){
                ALOGV("Godot ARCore: Tracked Image %s has tracking STOPPED\n", name);
            } else {
                ALOGV("Godot ARCore: Tracked Image %s has tracking status %d\n", name, tracking_state);
            }
            
            //img_to_anchor[image_index] = nullptr;
        }
        ArString_release(name);
    }
}


int ImageTracker::registerImageForTracking(ArSession *p_ar_session, godot::Image *p_img, const godot::String &p_img_name, float p_phys_img_width){

    // TODO put this in background thread as said here
    // https://developers.google.com/ar/reference/c/group/ar-augmented-image-database#araugmentedimagedatabase_addimage

    int32_t w = p_img->get_width(), h = p_img->get_height();
    ALOGV("Godot ARCore: Registering image as %s (%dx%d)\n", p_img_name.ascii().ptr(), w, h);
    ALOGV("Godot ARCore: Session null: %d   - Image DB null: %d\n", p_ar_session == nullptr, image_db == nullptr);
    uint8_t* grayscale = (uint8_t*) memalloc(w*h*sizeof(uint8_t));

    
    for(int j = 0; j < h; j++){
    for(int i = 0; i < w; i++){
        Color pixc = p_img->get_pixel(i, j);
        grayscale[i+j*w] = uint8_t(CLAMP( 255.0*(0.299*pixc.r + 0.587*pixc.g + 0.115*pixc.b), 0, 255));
    }}

    

    int32_t outidx;

    ArStatus stts;
    if(p_phys_img_width > 0.0) stts = ArAugmentedImageDatabase_addImageWithPhysicalSize(
        p_ar_session, image_db, p_img_name.ascii().ptr(), grayscale, w, h, w, p_phys_img_width, &outidx);
    else stts = ArAugmentedImageDatabase_addImage(
        p_ar_session, image_db, p_img_name.ascii().ptr(), grayscale, w, h, w, &outidx);

    ImageFrameTrackingData* data = (ImageFrameTrackingData*) memalloc(sizeof(ImageFrameTrackingData));
    name_to_data.insert(p_img_name, data);

    memfree(grayscale);

    if(stts == AR_SUCCESS){
        ALOGV("Godot ARCore: Registered tracked image, id is %d\n", outidx);
        return outidx;
    }

    if(stts == AR_ERROR_INVALID_ARGUMENT){
        ALOGE("Godot ARCore: Invalid arguments in addImage (stride != width?)");
    }

    if(stts == AR_ERROR_IMAGE_INSUFFICIENT_QUALITY){
        ALOGE("Godot ARCore: Cannot track image %s , lacking trackable features", p_img_name.ascii().ptr());
    }

    return stts;

}
