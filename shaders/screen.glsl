#ifdef VERTEX
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 uPosition;
uniform vec2 uSize;

out vec2 vTexCoord;

void main() {
	vTexCoord = aTexCoord;
	vec2 scaledPos = aPosition * uSize;
	vec2 worldPos = uPosition + scaledPos;
	gl_Position = vec4(worldPos, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT
uniform sampler2D uTexture;
uniform int uChannels;

in vec2 vTexCoord;

out vec4 FragColor;

void main() {
	vec3 color = texture(uTexture, vTexCoord).rgb;
	if (uChannels == 1) color = vec3(color.r);
	FragColor = vec4(color, 1.0);
}
#endif
