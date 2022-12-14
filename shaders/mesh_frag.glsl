#version 330 core
uniform vec3 camera_pos;
uniform vec3 lightpos;
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform sampler2D displacement0;
uniform sampler2D shadowMap;

in vec3 model_pos;
in vec3 normal;
in vec2 tex_coord;
in vec4 fragPosLight;

out vec4 FragColor;

float computeShadows() {
	float shadow = 0.0f;
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	if(lightCoords.z <= 1.0f) {
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		float bias = max(0.0025f * (1.0f - dot(normal, normalize(lightpos))), 0.001f);

		int sampleRadius = 8;
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

vec4 directLight() {
	if (texture(diffuse0, tex_coord).a < 0.1)
		discard;
	float shadow = computeShadows();

	float ambient = 0.25f;
	vec3 lightDirection = normalize(lightpos);
	float diffuse = max(0.0f, dot(normal, lightDirection));

	float specular = 0.0f;
	if (diffuse != 0.0f) {
		float specularLight = 0.5f;
		vec3 viewDirection = normalize(camera_pos - model_pos);
		vec3 halfwayVec = normalize(viewDirection + lightDirection);
		float specAmount = pow(max(0.0f, dot(normal, halfwayVec)), 8);
		specular = specAmount * specularLight;
	}

	return texture(diffuse0, tex_coord) * (diffuse * (1.0f - shadow) + ambient) + texture(specular0, tex_coord).r * specular * (1.0f - shadow);
}

void main() {
	FragColor = directLight();

	// Debug normals
	//FragColor = vec4((normal + 1.0f) / 2.0f, 1.0f);

	// Debug light
	//float l = max(0.0f, dot(normal, normalize(lightpos)));
	//FragColor = vec4(0.1f, 0.1f, l, 1.0f);
}
