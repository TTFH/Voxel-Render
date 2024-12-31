#version 410 core
uniform vec3 color;
uniform vec3 light_pos;
uniform sampler2D shadowMap;

in vec3 vNormal;
in vec4 vFragPosLight;

out vec4 FragColor;

float calculateShadow() {
	float shadow = 0.0f;
	vec3 lightCoords = vFragPosLight.xyz / vFragPosLight.w;
	if (lightCoords.z <= 1.0f) {
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		float bias = max(0.0025f * (1.0f - dot(vNormal, normalize(light_pos))), 0.0005f);

		int sampleRadius = 4;
		vec2 pixelSize = 1.0f / textureSize(shadowMap, 0);
		for (int y = -sampleRadius; y <= sampleRadius; y++) {
			for (int x = -sampleRadius; x <= sampleRadius; x++) {
				float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;	 
			}
		}
		shadow /= pow(sampleRadius * 2.0f + 1.0f, 2.0f);
	}
	shadow *= 0.5f;
	return shadow;
}

void main() {
	float shadow = calculateShadow();
	float l = 0.6f + 0.4f * max(0.0f, dot(vNormal, normalize(light_pos)));
	FragColor = vec4(color * l * (1.0f - shadow), 1.0f);

	// Debug normals
	//FragColor = vec4((vNormal + 1.0f) / 2.0f, 1.0f);
}
