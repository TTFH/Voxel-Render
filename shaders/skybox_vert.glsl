#version 410 core
layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 texCoords;

void main() {
	vec4 pos = projection * view * vec4(aPos, 1.0f);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	texCoords = vec3(aPos.x, aPos.y, -aPos.z);
}
