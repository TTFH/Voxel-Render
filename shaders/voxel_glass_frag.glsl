#version 410 core
uniform int uPalette;
uniform sampler2D uColor;
uniform sampler2D shadowMap;
uniform vec3 lightpos;

in vec3 normal;
in float tex_coord;
in vec4 fragPosLight;

out vec4 FragColor;

void main() {
	vec4 color = texelFetch(uColor, ivec2(tex_coord + 0.5, uPalette), 0);
	if (color.a == 1.0f) discard;
	float l = 0.6f + 0.4f * max(0.0f, dot(normal, normalize(lightpos)));
	FragColor = vec4(color.rgb * l, 0.66f);
}
