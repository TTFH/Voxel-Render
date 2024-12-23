#version 410 core
uniform sampler2D diffuse;
uniform vec4 tint_color;

in vec2 tex_coord;

out vec4 FragColor;

void main() {
	vec4 color = texture(diffuse, tex_coord) * tint_color;
	if (color.a < 0.1f) discard;
	FragColor = color;
}
