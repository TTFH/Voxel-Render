#version 410 core
layout(location = 0) in vec3 aPosition;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;

void main() {
	vec4 worldPosition = position * rotation * vec4(aPosition, 1.0f);
	gl_Position = camera * worldPosition;
}
