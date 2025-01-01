#version 410 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 size;
uniform vec2 uv_min;
uniform vec2 uv_max;
uniform int tex_rot;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 world_pos;
uniform mat4 world_rot;

out vec2 vTexCoord;

void main() {
	vec2 uv = aTexCoord;
	if (tex_rot == 90)
		uv = vec2(1.0f - uv.y, uv.x);
	else if (tex_rot == 180)
		uv = vec2(1.0f - uv.x, 1.0f - uv.y);
	else if (tex_rot == 270)
		uv = vec2(uv.y, 1.0f - uv.x);
	uv = uv_min + (uv_max - uv_min) * uv;
	vTexCoord = uv;

	vec4 currentPos = position * rotation * vec4(aPosition * size, 0.0f, 1.0f);
	gl_Position = camera * world_pos * world_rot * currentPos;
}
