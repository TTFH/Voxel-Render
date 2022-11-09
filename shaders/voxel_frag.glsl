#version 420 core
uniform sampler1D palette;
uniform vec3 lightpos;

in vec3 normal;
in float tex_coord;

out vec4 FragColor;

float linearDepth(float depth) {
	float near = 0.1f;
	float far = 500.0f;
	float z = depth * 2.0f - 1.0f;
	return (2.0f * near * far) / (far + near - z * (far - near));
}

float logisticDepth(float depth) {
	float steepness = 0.1f;
	float offset = 10.0f;
	float zval = linearDepth(depth);
	return 1 / (1 + exp(-steepness * (zval - offset)));
}

void main() {
	float l = 0.6f + 0.4f * max(0.0f, dot(normalize(normal), normalize(lightpos)));
	vec4 color = texture(palette, (tex_coord + 0.5f) / 256.0f);
	FragColor = vec4(color.rgb * l, 1.0f);

	// Debug normals
	//FragColor = vec4((normal + 1.0f) / 2.0f, 1.0f);

	// Debug light
	//float l = max(0.0f, dot(normalize(normal), normalize(lightpos)));
	//FragColor = vec4(0.1f, l, 0.1f, 1.0f);

	// Silent Hill
	//float depth = logisticDepth(gl_FragCoord.z);
	//FragColor = color * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), 1.0f);
}
