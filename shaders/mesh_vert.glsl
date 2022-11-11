#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;

out vec3 model_pos;
out vec3 normal;
out vec2 tex_coord;

void main() {
	model_pos = vec3(position * rotation * vec4(aPos, 1.0f));
	normal = normalize(aNormal);
	tex_coord = aTexCoord;
	gl_Position = camera * vec4(model_pos, 1.0);
}
