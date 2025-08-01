extends Node
class_name ImageTrackedComponent

signal onActivated
signal onDeactivated

var target: Node3D

@export var trackedImage: Image
@export var trackedTexture: Texture2D

@export_group("Transform")
@export var position_offset: Vector3 = Vector3(0,0,0)

@export var maxDistanceRender: float = 2

@export var timeToDeactivate: float = 5

var ttl_activation: float = -1

enum ROTATION_MIX {ABSOLUTE, RELATIVE}

@export var x_rotation_mix: ROTATION_MIX = ROTATION_MIX.RELATIVE
@export var x_rotation_val: float = 0.0

@export var y_rotation_mix: ROTATION_MIX = ROTATION_MIX.RELATIVE
@export var y_rotation_val: float = 0.0

@export var z_rotation_mix: ROTATION_MIX = ROTATION_MIX.RELATIVE
@export var z_rotation_val: float = 0.0

func _ready():
	target = get_parent()
	var image_name: String = ""
	if trackedImage == null:
		if trackedTexture == null:
			push_error("ImageTrackedComponent needs either an image or a texture")
		trackedImage = trackedTexture.get_image()
	

	ARCoreInterfaceInstance.image_tracker_database_add_image(trackedImage, image_name)

func is_active():
	return ttl_activation > 0

func _process(delta: float):
	
	var validTracking = ARCoreInterfaceInstance.image_tracker_get_tracking_status(trackedImage.resource_name)
	if validTracking:
		if ttl_activation < 0:
			onActivated.emit()
		ttl_activation = timeToDeactivate
		target.visible = true
	else:
		if ttl_activation > 0:
			ttl_activation -= delta
			if ttl_activation < 0:
				onDeactivated.emit()
				target.visible = false
		return


	var trackedTrsfrm: Transform3D = ARCoreInterfaceInstance.image_tracker_get_tracked_transform(trackedImage.resource_name)
	target.position = trackedTrsfrm.origin + position_offset
	
	
	var cam = get_viewport().get_camera_3d()
	var distToCam = target.global_position.distance_to(cam.global_position)
	
	if distToCam > maxDistanceRender:
		target.visible = false
		return
	else:
		target.visible = true
	
	
	var imgRot = trackedTrsfrm.basis.get_euler()
	
	var rot_x = x_rotation_val
	if x_rotation_mix == ROTATION_MIX.RELATIVE:
		rot_x += imgRot.x
	target.rotation.x = rot_x
	
	
	var rot_y = y_rotation_val
	if y_rotation_mix == ROTATION_MIX.RELATIVE:
		rot_y += imgRot.y
	target.rotation.y = rot_y
	
	
	var rot_z = z_rotation_val
	if z_rotation_mix == ROTATION_MIX.RELATIVE:
		rot_z += imgRot.z
	target.rotation.z = rot_z
