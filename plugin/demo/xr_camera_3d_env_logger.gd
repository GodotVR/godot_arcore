extends XRCamera3D

func _process(delta: float) -> void:
	var skyMat: ShaderMaterial = environment.sky.sky_material
	print("Cam id in skymaterialshader textures: ", skyMat.get_shader_parameter("camera_y").camera_feed_id)
