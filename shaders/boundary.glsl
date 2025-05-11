uniform sampler2D uTexture;
uniform float uVisibleDistance;
uniform vec4 uBoundaryColor;
uniform mat4 uMvpMatrix;
uniform vec3 uPlayerPos;

#ifdef VERTEX
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec3 vWorldPos;
out vec2 vTexCoord;

void main() {
	gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
	vWorldPos = vec4(aPosition, 1.0).xyz;
	vTexCoord = aTexCoord;
}
#endif

#ifdef FRAGMENT
in vec3 vWorldPos;
in vec2 vTexCoord;

out vec4 FragColor;

void main() {
	float dist = length(uPlayerPos - vWorldPos);
	float a = 1.0 - min(dist / uVisibleDistance, 1.0);
	a *= a;

	if (a == 0.0)
		discard;

	FragColor = vec4(uBoundaryColor);
	FragColor.a *= texture(uTexture, vTexCoord).a * a;
}
#endif
