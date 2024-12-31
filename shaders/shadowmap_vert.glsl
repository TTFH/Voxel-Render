#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec3 aOffset;

uniform int side;
uniform vec3 size;
uniform float scale;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 world_pos;
uniform mat4 world_rot;
uniform mat4 lightMatrix;

vec3 getHexPos(int side) {
	vec3 pos;
	if (side == 0) { // CUBE
		pos = aPos + aOffset;
	} else if (side == 1) { // TOP
		vec3 stretch = vec3(1.5f, sqrt(3), 1.0f);
		vec3 offset = vec3(1.5 * aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.x, 2) * 0.5 * sqrt(3), aOffset.z);
		pos = (aPos + offset) / stretch;
	} else if (side == 2) { // FRONT
		vec3 aPos2 = vec3(aPos.x, aPos.z, -aPos.y);
		vec3 stretch = vec3(1.5f, 1.0f, sqrt(3.0f));
		vec3 offset = vec3(1.5 * aOffset.x, aOffset.y, sqrt(3) * aOffset.z + mod(aOffset.x, 2) * 0.5 * sqrt(3));
		pos = (aPos2 + offset) / stretch;
	} else if (side == 3) { // SIDE
		vec3 aPos3 = vec3(aPos.z, -aPos.y, aPos.x);
		vec3 stretch = vec3(1.0f, sqrt(3.0f), 1.5f);
		vec3 offset = vec3(aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.z, 2) * 0.5 * sqrt(3), 1.5 * aOffset.z);
		pos = (aPos3 + offset) / stretch;
	}
	return pos;
}

void main() {
	// Mesh
	vec4 worldPosition = position * rotation * vec4(aPos, 1.0f);

	// Voxbox
	if (size.x != 0.0f)
		worldPosition = position * rotation * vec4(aPos * size, 10.0f);

	// Voxel
    if (scale > 0.0f) {
	    vec4 pos = position * rotation * vec4(getHexPos(side), 1.0f);
	    worldPosition = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);
    }

	gl_Position = lightMatrix * worldPosition;
}
