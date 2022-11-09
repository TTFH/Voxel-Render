#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aTexCoord;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 world_pos;
uniform mat4 world_rot;
uniform float scale;

out vec3 normal;
out float tex_coord;

void main() {
	vec4 pos = position * rotation * vec4(aPos, 1.0f);
	gl_Position = camera * world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);

	vec4 local_normal = rotation * vec4(aNormal, 1.0f);
	normal = (world_rot * vec4(local_normal.x, local_normal.z, -local_normal.y, 1.0f)).xyz;

	tex_coord = aTexCoord;
}
