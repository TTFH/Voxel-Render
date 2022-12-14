#version 420 core
uniform sampler2D tex0;

in vec2 tex_coord;

out vec4 FragColor;

void main() {
	FragColor = texture(tex0, tex_coord);
}
