#version 420 core
layout(location = 0) in vec2 aPos;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;

out vec3 normal;

void main() {
	gl_Position = camera * position * rotation * vec4(aPos.x, 0.02f, aPos.y, 1.0f);
	normal = (rotation * vec4(0.0f, 1.0f, 0.0f, 1.0f)).xyz;
}
