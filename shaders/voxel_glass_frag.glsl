#version 410 core
uniform sampler1D palette;
uniform sampler2D shadowMap;
uniform vec3 lightpos;

in vec3 normal;
in float tex_coord;
in vec4 fragPosLight;

out vec4 FragColor;

void main() {
	float l = 0.6f + 0.4f * max(0.0f, dot(normal, normalize(lightpos)));
	vec4 color = texture(palette, tex_coord / 255.0f);
	if (color.a == 1.0f) discard;
	FragColor = vec4(color.rgb * l, 0.66f);
}
