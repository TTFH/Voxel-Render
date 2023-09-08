#version 410 core
layout(location = 0) in vec3 aPos;
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
uniform vec4 clip_plane;
uniform mat4 lightProjection;

out vec3 normal;
out float tex_coord;
out vec4 fragPosLight;

const float threehalf = sqrt(3);
const float twoplustwo = 5;

vec3 getHexPos() {
	vec3 pos;
	if (side == 0) { // CUBE
		pos = aPos + aOffset;
	} else if (side == 1) { // TOP
		vec3 stretch = vec3(threehalf, sqrt(3), 1.0f);
		vec3 offset = vec3(threehalf * aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.x, 2) * 0.5 * sqrt(3), aOffset.z);
		pos = (aPos + offset) / stretch;
	} else if (side == 2) { // FRONT
		vec3 aPos2 = vec3(aPos.x, aPos.z, -aPos.y);
		vec3 stretch = vec3(threehalf, 1.0f, sqrt(3.0f));
		vec3 offset = vec3(threehalf * aOffset.x, aOffset.y, sqrt(3) * aOffset.z + mod(aOffset.x, 2) * 0.5 * sqrt(3));
		pos = (aPos2 + offset) / stretch;
	} else if (side == 3) { // SIDE
		vec3 aPos3 = vec3(aPos.z, -aPos.y, aPos.x);
		vec3 stretch = vec3(1.0f, sqrt(3.0f), threehalf);
		vec3 offset = vec3(aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.z, 2) * 0.5 * sqrt(3), threehalf * aOffset.z);
		pos = (aPos3 + offset) / stretch;
	}
	return pos;
}

vec3 getHexNormal() {
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
	vec4 pos = position * rotation * vec4(getHexPos(), 1.0f);
	vec4 currentPos = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);

	gl_ClipDistance[0] = dot(currentPos, clip_plane);
	gl_Position = camera * currentPos;

	vec4 local_normal = rotation * vec4(getHexNormal(), 1.0f);
	normal = normalize((world_rot * vec4(local_normal.x, local_normal.z, -local_normal.y, 1.0f)).xyz);

	tex_coord = aTexCoord;
	fragPosLight = lightProjection * currentPos;
}
