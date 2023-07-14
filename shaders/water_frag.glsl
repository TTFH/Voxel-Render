#version 420 core
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform float time;

in vec2 uv;
in vec3 to_camera;
in vec3 from_light;
in vec4 clip_space;

out vec4 FragColor;

float waterSpeed = 0.02f;
float waveStrength = 0.04f;
float shineDamper = 20.0f;
float reflectivity = 0.5f;

void main() {
	float moveFactor = waterSpeed * time;
	moveFactor -= floor(moveFactor);

	vec2 ndc = (clip_space.xy / clip_space.w) / 2.0f + 0.5f;
/*
	float near = 0.1f;
	float far = 500.0f;
	float depth = texture(depthMap, ndc).r;
	float floorDistance = 2.0f * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	float waterDistance = 2.0f * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
	float waterDepth = floorDistance - waterDistance;
*/
	vec2 distortion = texture(dudvMap, vec2(uv.x + moveFactor, uv.y)).rg * 0.1f;
	distortion = uv + vec2(distortion.x, distortion.y + moveFactor);
	vec2 totalDistortion = texture(dudvMap, distortion).rg * 2.0f - 1.0f;

	ndc += totalDistortion * waveStrength;
	ndc = clamp(ndc, 0.001f, 0.999f);

	vec4 reflectColor = texture(reflectionTexture, vec2(ndc.x, -ndc.y));
	vec4 refractColor = texture(refractionTexture, ndc);

	float refractiveFactor = dot(to_camera, vec3(0.0f, 1.0f, 0.0f));
	refractiveFactor = pow(refractiveFactor, 0.5f);

	vec4 normalColor = texture(normalMap, distortion);
	vec3 normal = vec3(normalColor.r * 2.0f - 1.0f, normalColor.b, normalColor.g * 2.0f - 1.0f);
	normal = normalize(normal);

	vec3 reflectedLight = reflect(from_light, normal);
	float specular = max(dot(reflectedLight, to_camera), 0.0f);
	//specular = pow(specular, shineDamper);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 specularHighlights = lightColor * specular * reflectivity;

	vec4 outColor = mix(reflectColor, refractColor, refractiveFactor);
	vec4 waterColor = vec4(0.22f, 0.22f, 0.44f, 1.0f);
	FragColor = mix(outColor, waterColor, 0.2f) + vec4(specularHighlights, 0.0f);
}
