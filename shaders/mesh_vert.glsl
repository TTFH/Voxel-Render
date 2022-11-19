#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 lightProjection;

out vec3 model_pos;
out vec3 normal;
out vec2 tex_coord;
out vec4 fragPosLight;

void main() {
	vec4 currentPos = position * rotation * vec4(aPos, 1.0f);
	model_pos = currentPos.xyz;
	normal = (rotation * vec4(aNormal, 1.0f)).xyz;
	tex_coord = aTexCoord;
	fragPosLight = lightProjection * currentPos;
	gl_Position = camera * currentPos;
}
