#version 420 core
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

in vec4 world_pos;
out vec4 FragColor;

void main() {
	vec2 uv = (world_pos.xy / world_pos.w) / 2.0f + 0.5f;

	vec4 reflectColor = texture(reflectionTexture, vec2(uv.x, -uv.y));
	vec4 refractColor = texture(refractionTexture, uv);
	FragColor = mix(reflectColor, refractColor, 0.5f);
}
