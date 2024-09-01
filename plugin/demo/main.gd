extends Node3D

# TODO: Update to match your plugin's name
var _plugin_name = "ARCorePlugin"
var ARCorePlugin

func _ready():
	if Engine.has_singleton(_plugin_name):
		ARCorePlugin = Engine.get_singleton(_plugin_name)

		# This sets up access to the environment and godot classes
		ARCorePlugin.initializeEnvironment()
	else:
		printerr("Couldn't find plugin " + _plugin_name)

func _process(delta):
	# Display the XRCamera position we get from ARCore
	# print($XROrigin3D.position)
	# print($XROrigin3D/XRCamera3D.fov)
	$CanvasLayer/Control/VBoxContainer/MarginContainer2/VBoxContainer/CameraPositionLabel.text = "XRCamera3D.position: " + str($XROrigin3D/XRCamera3D.position)
	$CanvasLayer/Control/VBoxContainer/MarginContainer2/VBoxContainer/TrackingState.text = "Tracking State: " + get_tracking_state()

func _on_start_ar_button_pressed():
	print("ARCoreInterfaceInstance")

	# This should be named "ARCoreInterface" but there is a name clash with the registered class "ARCoreInterface"
	ARCoreInterfaceInstance.start()

func get_tracking_state() -> String:
	var status = ARCoreInterfaceInstance.get_tracking_status()
	
	if status == XRInterface.XR_NOT_TRACKING:
		return "Not Tracking"
	elif status == XRInterface.XR_NORMAL_TRACKING:
		return "Normal Tracking"
	else:
		return "Unknown Tracking"
	#if ARCoreInterfaceInstance.get_tracking_status() == 0:
	#	return "Not Tracking"
	#else:
	#	return "Unknown State"
