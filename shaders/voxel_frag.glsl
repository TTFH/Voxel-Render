#version 410 core
uniform int uPalette;
uniform sampler2D uColor;
uniform sampler2D shadowMap;
uniform vec3 light_pos;
uniform bool transparent_glass;

in vec3 vNormal;
in float vTexCoord;
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
	vec4 color = texelFetch(uColor, ivec2(vTexCoord + 0.5, uPalette), 0);

	if (transparent_glass && color.a < 1.0f) discard;
	FragColor = vec4(color.rgb * l * (1.0f - shadow), 1.0f);

	// Debug normals
	//FragColor = vec4((vNormal + 1.0f) / 2.0f, 1.0f);

	// Debug light
	//float l = max(0.0f, dot(vNormal, normalize(light_pos)));
	//FragColor = vec4(0.1f, l, 0.1f, 1.0f);
}
