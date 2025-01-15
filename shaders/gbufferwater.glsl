#define WAVE_SPEED 1.0 // m/s
#define WAVE_FREQ 14.0 // higher = more frequent waves
#define WAVE_SCALE 0.8 // normal map scale

uniform float uFar;
uniform float uInvFar;
uniform vec3 uCameraPos;

uniform float uRndFrame;
uniform vec2 uPixelSize;
uniform sampler2D uBlueNoise;

vec2 blueNoiseTc;

const float alpha1 = 0.618034005;
const vec2 alpha2 = vec2(0.75487762, 0.56984027);

void blueNoiseInit(vec2 texCoord) {
	blueNoiseTc = (texCoord / uPixelSize) / 512.0;
}

float blueNoise() {
	float n = texture(uBlueNoise, blueNoiseTc).r;
	float v = fract(n + alpha1 * uRndFrame);
	blueNoiseTc += alpha2;
	return v;
}

vec2 blueNoise2() {
	vec2 n = texture(uBlueNoise, blueNoiseTc).rg;
	vec2 v = fract(n + alpha2 * uRndFrame);
	blueNoiseTc += alpha2;
	return v;
}

uniform mat4 uMvpMatrix;
uniform mat4 uStableVpMatrix;
uniform mat4 uOldStableVpMatrix;
uniform float uTime;
uniform float uWave;
uniform float uRipple;
uniform float uMotion;
uniform float uFoam;
uniform sampler2D uFoamTexture;
uniform sampler2D uDepth;
uniform sampler2D uColor;
uniform vec4 uWaterColor;
uniform float uVisibility;

uniform vec4 uRings[64];
uniform int uRingCount;

varying vec4 vCurrentPos;
varying vec4 vOldPos;
varying vec3 vPosition;
varying float vZ;

#ifdef VERTEX
attribute vec3 aPosition;

void main() {
	gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
	if (uCameraPos.y - aPosition.y < 0.3)
		gl_Position.z = 0.0;
	vPosition = aPosition;
	vCurrentPos = uStableVpMatrix * vec4(aPosition, 1.0);
	vOldPos = uOldStableVpMatrix * vec4(aPosition, 1.0);
	vZ = gl_Position.w * uInvFar;
}
#endif

#ifdef FRAGMENT
layout(location = 0) out vec4 outputColor;
layout(location = 1) out vec3 outputNormal;
layout(location = 2) out vec4 outputMaterial;
layout(location = 3) out vec3 outputMotion;
layout(location = 4) out float outputDepth;

float ring(vec2 ringPos, float radius, vec2 pos) {
	vec2 dir = ringPos - pos;
	float dist = dot(dir, dir);
	if (dist > radius*radius)
		return 0.0;

	dist = sqrt(dist);
	dir /= dist + 0.01;

	float ripple = sin((radius - dist) * WAVE_FREQ - 1.57) * 0.5 + 0.5;

	float amp = dist / radius;
	ripple *= amp * amp;
	return ripple;
}

float nrand(vec2 c) {
	return fract(sin(dot(c.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float nnoise(vec2 p) {
	vec2 ij = floor(p);
	vec2 xy = mod(p, 1.0);
	xy = 3.0*xy*xy-2.0*xy*xy*xy;
	float a = nrand((ij + vec2(0., 0.)));
	float b = nrand((ij + vec2(1., 0.)));
	float c = nrand((ij + vec2(0., 1.)));
	float d = nrand((ij + vec2(1., 1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

const mat2 mtx = mat2(0.80, 0.60, -0.60, 0.80);
float pNoise(vec2 p, int res, float time) {
	float n = 0.;
	float amp = 1.;
	for (int i = 0; i<res; i++) {
		n += amp*(sin(nnoise(p)*6.28+time)*0.5+0.5);
		p = mtx*p * 2.0;
		time *= 1.5;
		amp *= 0.5;
	}
	return n;
}

float getHeight(vec2 orgp, vec2 p, float time, vec3 dir) {
	float h = pNoise(p*0.15, 2, time*1.0) * uWave;

	for (int i = 0; i < uRingCount; i++) {
		vec4 ringData = uRings[i];
		float life = ringData.z;
		float age = ringData.w;
		float size = (age + 0.2) * WAVE_SPEED;

		float scale = 1.0 - age / life;
		scale *= scale;

		h += ring(ringData.xy, size, orgp) * scale * WAVE_SCALE;
	}

	p -= (dir*h*1.0).xz;
	h += pNoise(p*1.0, 5, time*(0.5+uRipple*5.0))*(uRipple*0.2);
	return h;
}

void main() {
	vec2 tc = gl_FragCoord.xy * uPixelSize;
	vec2 tcCurrent = vCurrentPos.xy / vCurrentPos.w*0.5 + vec2(0.5);
	vec2 tcOld = vOldPos.xy / vOldPos.w*0.5 + vec2(0.5);

	vec2 xy = vPosition.xz;

	float dx2 = pNoise(xy*0.07, 1, uTime*0.4);
	float dy2 = pNoise(xy*0.09, 1, uTime*0.3);
	xy += vec2(dx2, dy2)*(uMotion*2.0);

	vec3 dir = normalize(uCameraPos - vPosition);
	float h = getHeight(vPosition.xz, xy, uTime, dir);

	vec3 surfacePos = vPosition;
	vec4 surfaceHPos = (uMvpMatrix * vec4(surfacePos, 1.0));
	vec2 surfaceUv = surfaceHPos.xy / surfaceHPos.w;
	float newZ = surfaceHPos.w * uInvFar;

	float depth = texture(uDepth, tc).r;

	vec3 hitPos = vPosition + vec3(0, -h, 0);

	float dx = dFdx(h);
	float dy = dFdy(h);

	vec3 n = vec3(0.0, 0.1, 0.0);
	float fade = max(0.0, 1.0 - abs(dy)*20.0);
	fade *= fade * fade;
	n.xz += vec2(dx, dy) * fade;
	n = normalize(n);

	float waterDepth = max(0.0, (depth - newZ)*uFar);

	blueNoiseInit(tcCurrent);

	vec2 distort = n.xz * 0.003 / depth;
	vec2 orgTc = tc;
	tc += distort + (blueNoise2()-vec2(0.5))*0.002*min(uVisibility, waterDepth);
	float ddepth = texture(uDepth, tc).r;
	if (ddepth < newZ) {
		tc = orgTc;
		ddepth = depth;
	}
	waterDepth = max(0.0, (ddepth - newZ)*uFar);

	if (ddepth > 0.99)
		ddepth = newZ;

	float depthFraction = min(1.0, 0.1 * waterDepth / uVisibility);
	depthFraction = sqrt(depthFraction);
	if (blueNoise() > depthFraction) {
		outputColor = mix(texture(uColor, tc)*0.7, uWaterColor, depthFraction);
		outputDepth = ddepth;
		outputNormal = n;
	} else {
		outputColor = uWaterColor;
		outputDepth = newZ;
		outputNormal = n;
	}

	if (blueNoise()*0.5 > max(0.0, (depth - newZ)*uFar))
		discard;

	// Foam
	vec2 remappedXy = xy - (dir.xz * h);
	float foam = texture(uFoamTexture, remappedXy*0.13).r;
	foam *= texture(uFoamTexture, remappedXy*0.009).r;
	foam *= (uFoam*0.1);
	outputColor += vec4(foam);

	outputMaterial = vec4(0.02, 1.0, 0.0, 0.0);
	//outputNormal = n;
	outputMotion = vec3(tcCurrent - tcOld, 1.0);
	//outputDepth = newZ;
}
#endif
