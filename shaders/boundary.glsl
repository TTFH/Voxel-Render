uniform mat4 uMvpMatrix;
//uniform sampler2D uDepth;
uniform sampler2D uTexture;
uniform float uVisibleDistance;
uniform vec4 uColor;
uniform vec3 uFocus0;
uniform float uInvFar;

varying vec4 vHPos;
varying vec3 vWorldPos;
varying float vZ;
varying vec2 vTexCoord;

#ifdef VERTEX
attribute vec3 aPosition;
attribute vec2 aTexCoord;

void main() {
	gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
	vWorldPos = vec4(aPosition, 1.0).xyz;
	vHPos = gl_Position;
	vTexCoord = aTexCoord;
	vZ = gl_Position.w * uInvFar;
}
#endif

#ifdef FRAGMENT
void main() {
	vec2 tc = (vHPos.xy / vHPos.w) * 0.5 + vec2(0.5);
	float depth = 500; // texture(uDepth, tc).r;

	float dist = length(uFocus0 - vWorldPos);
	float a = 1.0 - min(dist / uVisibleDistance, 1.0);
	a *= a;

	if (vZ > depth || a == 0.0)
		discard;

	gl_FragColor = vec4(uColor);
	gl_FragColor.a *= texture(uTexture, vTexCoord).a * a;
}
#endif
