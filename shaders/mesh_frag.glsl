#version 330 core
uniform vec3 camera_pos;
uniform vec3 lightpos;
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform sampler2D displacement0;

in vec3 model_pos;
in vec3 normal;
in vec2 tex_coord;

out vec4 FragColor;

vec4 directLight() {
	if (texture(diffuse0, tex_coord).a < 0.1)
		discard;

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

	return texture(diffuse0, tex_coord) * (diffuse + ambient) + texture(specular0, tex_coord).r * specular;
}

void main() {
	FragColor = directLight();
}
