#version 410 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aTexCoord;
layout(location = 3) in vec3 aOffset;

uniform int side;
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

const float three_halves = sqrt(3);
const float two_plus_two = 5;

vec3 getHexPos(int side) {
	vec3 pos = aPosition + aOffset;
	if (side == 1) { // TOP
		vec3 stretch = vec3(three_halves, sqrt(3), 1.0f);
		vec3 offset = vec3(three_halves * aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.x, 2) * 0.5 * sqrt(3), aOffset.z);
		offset += vec3(0.25f, -0.5 * sqrt(3), 0);
		pos = (aPosition + offset) / stretch;
	} else if (side == 2) { // FRONT
		vec3 aPosition2 = vec3(aPosition.x, aPosition.z, -aPosition.y);
		vec3 stretch = vec3(three_halves, 1.0f, sqrt(3.0f));
		vec3 offset = vec3(three_halves * aOffset.x, aOffset.y, sqrt(3) * aOffset.z + mod(aOffset.x, 2) * 0.5 * sqrt(3));
		offset += vec3(0.25f, 0, 0.5 * sqrt(3));
		pos = (aPosition2 + offset) / stretch;
	} else if (side == 3) { // SIDE
		vec3 aPosition3 = vec3(aPosition.z, -aPosition.y, aPosition.x);
		vec3 stretch = vec3(1.0f, sqrt(3.0f), three_halves);
		vec3 offset = vec3(aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.z, 2) * 0.5 * sqrt(3), three_halves * aOffset.z);
		offset += vec3(0, 0.5 * sqrt(3), 0.25f);
		pos = (aPosition3 + offset) / stretch;
	}
	return pos;
}

vec3 getHexNormal(int side) {
	vec3 normal;
	if (side == 0 || side == 1)
		normal = aNormal;
	else if (side == 2)
		normal = vec3(aNormal.x, aNormal.z, -aNormal.y);
	else if (side == 3)
		normal = vec3(aNormal.z, -aNormal.y, aNormal.x);
	return normal;
}

void main() {
	vec4 pos = position * rotation * vec4(getHexPos(side), 1.0f);
	vec4 worldPosition = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);

	vec4 local_normal = rotation * vec4(getHexNormal(side), 1.0f);
	vec4 normal = world_rot * vec4(local_normal.x, local_normal.z, -local_normal.y, 1.0f);
	
	vNormal = normalize(normal.xyz);
	vTexCoord = aTexCoord;
	vFragPosLight = lightMatrix * worldPosition;
	gl_Position = camera * worldPosition;
}
