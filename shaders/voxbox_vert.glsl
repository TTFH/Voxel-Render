#version 410 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform vec3 size;
uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 lightMatrix;

out vec3 vNormal;
out vec4 vFragPosLight;

void main() {
	vec4 worldPosition = position * rotation * vec4(aPosition * size, 10.0f);
	vec4 normal = rotation * vec4(aNormal, 1.0f);
	vNormal = normalize(normal.xyz);
	vFragPosLight = lightMatrix * worldPosition;
	gl_Position = camera * worldPosition;
}
