#version 420 core
uniform vec3 lightpos;
uniform vec3 color;

in vec3 normal;
out vec4 FragColor;

void main() {
	float l = 0.6f + 0.4f * max(0.0f, dot(normalize(normal), normalize(lightpos)));
	FragColor = vec4(color * l, 1.0f);
}
