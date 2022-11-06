#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform vec3 size;

out vec3 normal;

void main() {
	gl_Position = camera * position * rotation * vec4(aPos * size, 10.0f);
	normal = (rotation * vec4(aNormal, 1.0f)).xyz;
}
