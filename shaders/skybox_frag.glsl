#version 330 core

uniform samplerCube skybox;
in vec3 texCoords;
out vec4 FragColor;

void main() {
	FragColor = texture(skybox, texCoords);
}
