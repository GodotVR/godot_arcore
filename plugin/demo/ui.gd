extends Control

signal estimate_depthmap_toggled(toggled)
signal show_depthmap_toggled(toggled)
signal vertical_planes_toggled(toggled)
signal horizontal_planes_toggled(toggled)
signal instant_placement_toggled(toggled)
signal images_detection_toggled(toggled)
signal light_estimation_toggled(toggled)
signal point_cloud_detection_toggled(toggled)
signal switch_orientation(vertical)
signal far_changed(value)

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass


func _on_estimate_depthmap_toggled(toggled_on):
	estimate_depthmap_toggled.emit(toggled_on)


func _on_schow_depthmap_toggled(toggled_on):
	show_depthmap_toggled.emit(toggled_on)


func _on_far_slider_value_changed(value):
	$Scale/FarValue.text = str(value)
	far_changed.emit(value)


func _on_vertical_plane_detection_toggled(toggled_on):
	vertical_planes_toggled.emit(toggled_on)


func _on_horizontal_plane_detection_toggled(toggled_on):
	horizontal_planes_toggled.emit(toggled_on)


func _on_instant_placement_toggled(toggled_on):
	instant_placement_toggled.emit(toggled_on)


func _on_light_estimation_toggled(toggled_on):
	light_estimation_toggled.emit(toggled_on)


func _on_images_detection_toggled(toggled_on):
	images_detection_toggled.emit(toggled_on)


func _on_point_cloud_detection_toggled(toggled_on):
	point_cloud_detection_toggled.emit(toggled_on)


func _on_vertical_display_toggled(toggled_on):
	switch_orientation.emit(toggled_on)
