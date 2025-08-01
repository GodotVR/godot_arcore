shader_type sky;

uniform samplerExternalOES camera_y : source_color, repeat_disable;
uniform samplerExternalOES camera_CbCr : source_color, repeat_disable;

uniform vec2 viewport_size = vec2(512,512);
uniform vec2 feed_transform_basis_x = vec2(1,0);
uniform vec2 feed_transform_basis_y = vec2(0,1);
uniform vec2 feed_transform_origin = vec2(0,0);

void sky() {
	if (AT_CUBEMAP_PASS){
		COLOR = vec3(0);
	}
	else{
		vec2 uv = FRAGCOORD.xy / viewport_size;

		// Very hacky, from trial and error
		uv.y = 1.0 - uv.y;
		mat2 feed_transform_basis = mat2(feed_transform_basis_x, feed_transform_basis_y);
		uv = feed_transform_origin + feed_transform_basis * uv;
		//uv = (inverse(feed_transform) * vec4(uv,0,1)).xy;

		// If the camera has YCbCr, use the commented block below
		// vec3 color;
		// color.r = texture(camera_y, uv).r;
		// color.gb = texture(camera_CbCr, uv).rg - vec2(0.5, 0.5);

		// // YCbCr -> SRGB conversion
		// // Using BT.709 which is the standard for HDTV
		// color.rgb = mat3(
		// 					vec3(1.00000, 1.00000, 1.00000),
		// 					vec3(0.00000, -0.18732, 1.85560),
		// 					vec3(1.57481, -0.46813, 0.00000)) *
		// 			color.rgb;

		COLOR.rgb = texture(camera_y, uv).rgb;
		//COLOR.rgb += vec3(uv-0.5,0);
		// Because of background canvas
		//COLOR = pow(COLOR, vec4(2.2));
	}
}