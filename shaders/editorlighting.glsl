uniform sampler2D uTexture;
uniform sampler2D uNormal;
uniform sampler2D uDepth;
uniform sampler2D uBlueNoise;
uniform vec2 uPixelSize;

uniform vec3 uCameraPos;
uniform float uNear;
uniform float uFar;

uniform mat4 uVpMatrix;
uniform mat4 uVpInvMatrix;

uniform vec3 uLightDir;

varying vec2 vTexCoord;
varying vec3 vFarVec;

#ifdef VERTEX
attribute vec2 aPosition;
attribute vec2 aTexCoord;

void main() {
	vTexCoord = aTexCoord;

	vec4 aa = vec4(vTexCoord * 2.0 - vec2(1.0), 1.0, 1.0);
	aa = uVpInvMatrix * aa;
	vFarVec = aa.xyz / aa.w - uCameraPos;

	gl_Position = vec4(aPosition, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT
void main() {
	float depth = texture(uDepth, vTexCoord).r;
	vec3 pos = uCameraPos + vFarVec * depth;
	vec3 normal = texture(uNormal, vTexCoord).xyz;

	vec4 hpos = uVpMatrix * vec4(pos, 1.0);

	float l = max(0.0, dot(normal, -uLightDir)) * 0.3 + 0.7;

	if (depth < 0.99) {
		const int count = 128;
		float o = 0.0;
		float c = 0.0;
		vec2 blueNoiseTc = (vTexCoord / uPixelSize) / 512.0;
		float r1 = texture(uBlueNoise, blueNoiseTc + vec2(0.5)).x;
		vec3 r3 = texture(uBlueNoise, blueNoiseTc).xyz;
		float rad = depth * uFar * 0.1;
		rad = clamp(rad, 0.5, 2.0);
		for (int i = 0; i < count; i++) {
			vec3 dir = normalize(r3 * 2.0 - vec3(1.0));

			r3 = fract(r3 + vec3(0.819172502, 0.671043575, 0.549700439));
			r1 = fract(r1 + 0.618034005);
		
			float d = dot(dir, normal);
			if (d < 0.1)
				continue;

			vec3 probe = pos + dir * (r1 * rad);

			vec4 hp = uVpMatrix * vec4(probe, 1.0);
			vec2 tc = hp.xy / hp.w * 0.5 + 0.5;

			float probeDepth = hp.w;
			float sceneDepth = texture(uDepth, tc).r * uFar;

			d *= 1.0 - r1;
			if (probeDepth > sceneDepth * 1.01 && probeDepth < sceneDepth + rad)
				o += d;
			c += d;
		}
		if (c > 0.0)
			l *= 1.0 - o / c;
	}

	gl_FragColor = texture(uTexture, vTexCoord) * l;
	gl_FragDepth = (1.0 / hpos.w - 1.0 / uNear) / (1.0 / uFar - 1.0 / uNear);
}
#endif
