extends ARVROrigin

signal anchor_added(anchor)
signal anchor_removed(anchor)

export (PackedScene) var anchor_scene = null

var arcore : ARVRInterface = null

func tracker_added(tracker_name, tracker_type, tracker_id):
	if tracker_type == ARVRServer.TRACKER_ANCHOR:
		var new_anchor = anchor_scene.instance()
		
		new_anchor.anchor_id = tracker_id
		new_anchor.set_name("anchor_" + str(tracker_id))
		
		add_child(new_anchor)
		emit_signal("anchor_added", new_anchor)

func tracker_removed(tracker_name, tracker_type, tracker_id):
	if tracker_type == ARVRServer.TRACKER_ANCHOR:
		var anchor = $ARVROrigin.get_node("anchor_" + str(tracker_id))
		if anchor:
			emit_signal("anchor_removed", anchor)
			anchor.queue_free()

func get_tracking_status() -> String:
	if !arcore:
		return "ARCore not initialized"
	
	var status = arcore.get_tracking_status() 
	if status == 4:
		return "Not tracking, move your device around for ARCore to detect features"
	elif status == 0:
		return "Tracking a-ok"
	elif status == 1:
		return "Insufficient tracking, moving your device to fast"
	elif status == 2:
		return "Insufficient features, move further away or improve lighting conditions"
	else:
		return "Unknown tracking status"

# Called when the node enters the scene tree for the first time.
func initialize() -> String:
	# register our signals
	ARVRServer.connect("tracker_added", self, "tracker_added")
	ARVRServer.connect("tracker_removed", self, "tracker_removed")
	
	arcore = ARVRServer.find_interface('ARCore')
	if !arcore:
		return "Failed to start ARCore"
	
	if !arcore.initialize():
		return "Failed to start ARCore"
	
	# this just means we started our initialisation process successfully
	get_viewport().arvr = true
	
	# assign our camera
	$ARVRCamera.environment.background_camera_feed_id = arcore.get_camera_feed_id()
	
	return "Ok"
