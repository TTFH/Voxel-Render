#version 410 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 size;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 world_pos;
uniform mat4 world_rot;

out vec2 tex_coord;

void main() {
	tex_coord = aTexCoord;
	vec4 currentPos = position * rotation * vec4(aPos * size, 0.0f, 1.0f);
	gl_Position = camera * world_pos * world_rot * currentPos;
}
