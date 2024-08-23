#version 410 core
uniform sampler2D diffuse;
uniform vec3 tint_color;
uniform vec2 uv_min;
uniform vec2 uv_max;
uniform int tex_rot;

in vec2 tex_coord;

out vec4 FragColor;

void main() {
	vec2 uv = tex_coord;
	if (tex_rot == 90)
		uv = vec2(1.0f - uv.y, uv.x);
	else if (tex_rot == 180)
		uv = vec2(1.0f - uv.x, 1.0f - uv.y);
	else if (tex_rot == 270)
		uv = vec2(uv.y, 1.0f - uv.x);

	uv = uv_min + (uv_max - uv_min) * uv;
	vec4 color = texture(diffuse, uv) * vec4(tint_color, 1.0f);
	//if (color.a < 0.1f) discard;
	FragColor = color;
}
