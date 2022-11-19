#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform vec3 size;
uniform mat4 camera;
uniform mat4 position;
uniform mat4 rotation;
uniform mat4 lightProjection;

out vec3 normal;
out vec4 fragPosLight;

void main() {
	vec4 currentPos = position * rotation * vec4(aPos * size, 10.0f);
	normal = normalize((rotation * vec4(aNormal, 1.0f)).xyz);
	fragPosLight = lightProjection * currentPos;
	gl_Position = camera * currentPos;
}
