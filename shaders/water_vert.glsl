#version 420 core
layout(location = 0) in vec2 aPos;

// Water AABB if not a quad
uniform vec2 min;
uniform vec2 max;
uniform vec3 lightpos;
uniform vec3 camera_pos;
uniform mat4 camera;
uniform mat4 position;

out vec2 uv;
out vec3 to_camera;
out vec3 from_light;
out vec4 clip_space;

float tiling = 6.0;

void main() {
	vec4 world_pos = position * vec4(aPos.x, 0.05f, aPos.y, 1.0f);
	uv = tiling * (aPos - min) / (max - min);
	to_camera = normalize(camera_pos - world_pos.xyz);
	from_light = normalize(world_pos.xyz - lightpos);
	clip_space = camera * world_pos;
	gl_Position = clip_space;
}
