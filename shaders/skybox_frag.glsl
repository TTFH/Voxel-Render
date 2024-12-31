#version 410 core

uniform samplerCube skybox;

in vec3 vTexCoord;

out vec4 FragColor;

void main() {
	FragColor = texture(skybox, vTexCoord);
}
