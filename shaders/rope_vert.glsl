#version 410 core
layout(location = 0) in vec3 aPos;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;

void main() {
	gl_Position = camera * position * rotation * vec4(aPos, 1.0f);
}
