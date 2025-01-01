#version 410 core
layout(location = 0) in vec3 aPosition;

uniform mat4 vpMatrix;

out vec3 vTexCoord;

void main() {
	vec4 pos = vpMatrix * vec4(aPosition, 1.0f);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	vTexCoord = vec3(aPosition.x, aPosition.y, -aPosition.z);
}
