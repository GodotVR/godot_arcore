@tool
extends EditorPlugin

# A class member to hold the editor export plugin during its lifecycle.
var export_plugin : AndroidExportPlugin

func _enter_tree():
	# Initialization of the plugin goes here.
	export_plugin = AndroidExportPlugin.new()
	add_export_plugin(export_plugin)

	# Add an autoload for the actual interface
	add_autoload_singleton(
			"ARCoreInterfaceInstance",
			"res://addons/ARCorePlugin/ARCoreInterface.gd")


func _exit_tree():
	# Clean-up of the plugin goes here.
	remove_export_plugin(export_plugin)
	export_plugin = null

	# TODO: Check if the singleton must be removed
	# remove_autoload_singleton("ARCorePlugin")


class AndroidExportPlugin extends EditorExportPlugin:
	# TODO: Update to your plugin's name.
	var _plugin_name = "ARCorePlugin"

	func _supports_platform(platform):
		if platform is EditorExportPlatformAndroid:
			return true
		return false

	func _get_android_libraries(platform, debug):
		if debug:
			return PackedStringArray([_plugin_name + "/bin/debug/" + _plugin_name + "-debug.aar"])
		else:
			return PackedStringArray([_plugin_name + "/bin/release/" + _plugin_name + "-release.aar"])

	func _get_android_dependencies(platform, debug):
		if debug:
			return PackedStringArray(["com.google.ar:core:1.44.0", "androidx.appcompat:appcompat:1.4.0"])
		else:
			return PackedStringArray(["com.google.ar:core:1.44.0", "androidx.appcompat:appcompat:1.4.0"])

	func _get_name():
		return _plugin_name
