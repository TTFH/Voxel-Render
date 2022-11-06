#version 420 core
uniform sampler1D palette;
uniform vec3 lightpos;

in vec3 normal;
in float tex_coord;

out vec4 FragColor;

void main() {
	float l = 0.6f + 0.4f * max(0.0f, dot(normalize(normal), normalize(lightpos)));
	FragColor = texture(palette, (tex_coord + 0.5f) / 256.0f).rgba * l;

	// Debug normals
	//FragColor = vec4((normal + 1.0f) / 2.0f, 1.0f);

	// Debug light
	//float l = max(0.0f, dot(normal, normalize(lightpos)));
	//FragColor = vec4(l, l, l, 1.0f);
}
