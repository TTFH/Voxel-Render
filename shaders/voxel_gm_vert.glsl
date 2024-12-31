#version 410 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aTexCoord;

uniform mat4 camera;
uniform float scale;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 world_pos;
uniform mat4 world_rot;
uniform mat4 lightMatrix;

out vec3 vNormal;
out float vTexCoord;
out vec4 vFragPosLight;

void main() {
	vec4 pos = position * rotation * vec4(aPosition, 1.0f);
	vec4 worldPosition = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);

	vec4 normal = rotation * vec4(aNormal, 1.0f);
	normal = world_rot * vec4(normal.x, normal.z, -normal.y, 1.0f);

	vNormal = normalize(normal.xyz);
	vTexCoord = aTexCoord;
	vFragPosLight = lightMatrix * worldPosition;
	gl_Position = camera * worldPosition;
}
