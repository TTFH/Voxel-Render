#version 420 core
layout(location = 0) in vec2 aPos;

uniform vec2 offset;

out vec2 uv0;
out vec2 texCoord;

void main() {
	gl_Position = vec4(aPos / 2.0 + offset, 0.0, 1.0);
	uv0 = 2.0 * aPos -1.0; // -1 to 1
	texCoord = aPos; // 0 to 1
}
