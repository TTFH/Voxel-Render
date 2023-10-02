#version 410 core
uniform sampler2D shadowMap;
uniform vec3 lightpos;
uniform vec3 color;

in vec3 normal;
in vec4 fragPosLight;

out vec4 FragColor;

float computeShadows() {
	float shadow = 0.0f;
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	if (lightCoords.z <= 1.0f) {
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		float bias = max(0.0025f * (1.0f - dot(normal, normalize(lightpos))), 0.001f);

		int sampleRadius = 2;
		vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
		for(int y = -sampleRadius; y <= sampleRadius; y++) {
			for(int x = -sampleRadius; x <= sampleRadius; x++) {
				float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;	 
			}
		}
		shadow /= pow((sampleRadius * 2 + 1), 2);
	}
	shadow *= 0.5f;
	return shadow;
}

void main() {
	float shadow = computeShadows();
	float l = 0.6f + 0.4f * max(0.0f, dot(normal, normalize(lightpos)));
	FragColor = vec4(color * l * (1.0f - shadow), 1.0f);

	// Debug normals
	//FragColor = vec4((normal + 1.0f) / 2.0f, 1.0f);
}
