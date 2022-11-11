#version 420 core
layout(location = 0) in vec3 aPos;

uniform vec3 size;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 world_pos;
uniform mat4 world_rot;
uniform float scale;
uniform mat4 lightProjection;

void main() {
	// Mesh
	vec4 currentPos = position * rotation * vec4(aPos, 1.0f);

	// Voxbox
	//vec4 currentPos = position * rotation * vec4(aPos * size, 10.0f);

	// Greedy Mesh Voxel
    if (scale > 0.0f) {
	    vec4 pos = position * rotation * vec4(aPos, 1.0f);
	    currentPos = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);
    }

	gl_Position = lightProjection * currentPos;
}
