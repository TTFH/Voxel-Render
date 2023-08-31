#version 410 core
uniform sampler2D diffuse0;

in vec2 texCoord;
out vec4 FragColor;

void main() {
	FragColor = texture(diffuse0, texCoord);
}
