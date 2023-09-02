#version 410 core

uniform samplerCube skybox;
in vec3 texCoords;
out vec4 FragColor;

void main() {
	vec3 color = texture(skybox, texCoords).rgb;
	// Gamma correction
	color = pow(color, vec3(1.0 / 2.2));
	FragColor = vec4(color, 1.0);
}
