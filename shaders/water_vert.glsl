#version 420 core
layout(location = 0) in vec2 aPos;

uniform mat4 camera;
uniform mat4 position;

out vec3 normal;

void main() {
	gl_Position = camera * position * vec4(aPos.x, 0.05f, aPos.y, 1.0f);
	normal = vec3(0.0f, 1.0f, 0.0f);
}
