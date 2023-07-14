#version 420 core
layout(location = 0) in vec2 aPos;

uniform mat4 camera;
uniform mat4 position;

out vec4 world_pos;

void main() {
	world_pos = camera * position * vec4(aPos.x, 0.05f, aPos.y, 1.0f);
	gl_Position = world_pos;
}
