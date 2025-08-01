extends Control

signal onScreenTouchDown(pos: Vector2)

var isTouchDown: bool = false


func _on_gui_input(event: InputEvent) -> void:
	#print("Control detected event")
	if event is InputEventScreenTouch and event.pressed == true:
		if not isTouchDown:
			onScreenTouchDown.emit(event.position)
		isTouchDown = true
	else:
		isTouchDown = false
