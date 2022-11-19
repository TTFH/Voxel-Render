#version 420 core
uniform vec3 lightpos;

in vec3 normal;
out vec4 FragColor;

void main() {
	float l = 0.6f + 0.4f * max(0.0f, dot(normal, normalize(lightpos)));
	FragColor.rgb = vec3(0.22f, 0.22f, 0.44f) * l;
	FragColor.a = 0.9f;
}
