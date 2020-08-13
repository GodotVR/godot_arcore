extends Spatial

var angle = 0;

func _add_log(text):
	$Log.text = $Log.text + text + "\n"

# Called when the node enters the scene tree for the first time.
func _ready():
	var result = $ARCore.initialize()
	if result != "Ok":
		print(result)
		_add_log(result)
	else:
		print("Successfully initialised ARCore")
		_add_log("Successfully initialised ARCore")

func _process(delta):
	angle += delta
	$Box.transform.basis = Basis(Vector3(1.0, 1.0, 1.0).normalized(), angle)

	var info_text = "FPS: " + str(Engine.get_frames_per_second()) + "\n"
	info_text += $ARCore.get_tracking_status() + "\n"

	$Info.text = info_text

func _on_Button_pressed():
	get_tree().quit()

func _on_ARCore_anchor_added(anchor):
	_add_log("New anchor: " + anchor.name)

func _on_ARCore_anchor_removed(anchor):
	_add_log("Removing anchor: " + anchor.name)
