#version 410 core

uniform vec3 camera_pos;
uniform vec3 light_pos;
uniform sampler2D tex0;
uniform sampler2D shadowMap;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPosition;
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
	vec3 color = texture(tex0, vTexCoord).rgb;

	vec3 lightDir = normalize(light_pos - vFragPosition);
	vec3 viewDir = normalize(camera_pos - vFragPosition);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	vec3 ambient = 0.1 * color;

	float diff = max(dot(vNormal, lightDir), 0.0);
	vec3 diffuse = diff * color;

	float spec = pow(max(dot(vNormal, halfwayDir), 0.0), 32);
	vec3 specular = spec * vec3(0.5);

	vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
	FragColor = vec4(lighting, 1.0);
}
