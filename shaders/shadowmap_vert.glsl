#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec3 aOffset;

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
	if (size.x != 0.0f)
		currentPos = position * rotation * vec4(aPos * size, 10.0f);

	// Voxel
    if (scale > 0.0f) {
	    vec4 pos = position * rotation * vec4(aPos, 1.0f);
	    currentPos = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);

		// Hexagons
		/*vec3 aPos2 = vec3(aPos.x, aPos.z, -aPos.y);
		vec3 stretch = vec3(1.5f, 1.0f, sqrt(3.0f));
		vec3 offset = vec3(1.5f * aOffset.x, aOffset.y, sqrt(3.0f) * aOffset.z + mod(aOffset.x, 2) * 0.5f * sqrt(3.0f));
		vec4 pos = position * rotation * vec4((aPos2 + offset) / stretch, 1.0f);
		currentPos = world_pos * world_rot * vec4(pos.x, pos.z, -pos.y, 10.0f / scale);*/
    }

	gl_Position = lightProjection * currentPos;
}
