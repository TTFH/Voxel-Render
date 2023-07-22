#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aTexCoord;
layout(location = 3) in vec3 aOffset;

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

void main() {
	//vec3 stretch = vec3(1.5f, sqrt(3), 1.0f);
	//vec3 offset = vec3(1.5 * aOffset.x, sqrt(3) * aOffset.y + mod(aOffset.x, 2) * 0.5 * sqrt(3), aOffset.z);

	vec3 aPos2 = vec3(aPos.x, aPos.z, -aPos.y);
	vec3 stretch = vec3(1.5f, 1.0f, sqrt(3.0f));
	vec3 offset = vec3(1.5f * aOffset.x, aOffset.y, sqrt(3.0f) * aOffset.z + mod(aOffset.x, 2) * 0.5f * sqrt(3.0f));

	vec4 pos = position * rotation * vec4((aPos2 + offset) / stretch, 1.0f);
	vec4 currentPos = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);
	gl_ClipDistance[0] = dot(currentPos, clip_plane);
	gl_Position = camera * currentPos;

	vec4 local_normal = rotation * vec4(aNormal, 1.0f);
	normal = normalize((world_rot * vec4(local_normal.x, local_normal.z, -local_normal.y, 1.0f)).xyz);

	tex_coord = aTexCoord;
	fragPosLight = lightProjection * currentPos;
}
