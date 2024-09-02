@tool
extends Node

# This is the autoload that is set up when enabling the plugin
# It also communicates between ARCore and Godot
var arcore : ARCoreInterface

func _enter_tree():
	arcore = ARCoreInterface.new()

	if arcore:
		XRServer.add_interface(arcore)

func _exit_tree():
	if arcore:
		XRServer.remove_interface(arcore)
		arcore = null

func get_interface():
	return arcore

func start():
	if arcore.initialize():
		get_viewport().use_xr = true

func get_tracking_status():
	return arcore.get_tracking_status()
