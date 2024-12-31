#version 410 core
layout(location = 0) in vec2 aPosition;

uniform mat4 camera;
uniform mat4 position;

void main() {
	vec4 worldPosition = position * vec4(aPosition.x, 0.05f, aPosition.y, 1.0f);
	gl_Position = camera * worldPosition;
}
