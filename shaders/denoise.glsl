#ifdef VERTEX
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
	gl_Position = vec4(aPosition, 0.0, 1.0);
	vTexCoord = aTexCoord;
}
#endif

#ifdef FRAGMENT
uniform sampler2D uNew;
uniform sampler2D uOld;
uniform sampler2D uNormal;
uniform sampler2D uDepth;

uniform float uFar;
uniform vec3 uCameraPos;
uniform mat4 uVpMatrix;
uniform mat4 uVpInvMatrix;

uniform float uRndFrame;
uniform vec2 uPixelSize;
uniform sampler2D uBlueNoise;

in vec2 vTexCoord;
out vec4 FragColor;

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

vec3 computePixelDir(vec2 texCoord) {
	vec4 aa = vec4(texCoord * 2.0 - vec2(1.0), 1.0, 1.0);
	aa = uVpInvMatrix * aa;
	return aa.xyz / aa.w - uCameraPos;
}

float calculatePixelMaxVelocity(vec2 pixelSize) {
	const vec2 kDefaultResolution = vec2(1920.0f, 1080.0f);
	const float kDefaultMaxPixelVel = 30.0f;

	return kDefaultMaxPixelVel * (1.0f /(pixelSize.x * kDefaultResolution.x));
}

void main() {
	float linearDepth = texture(uDepth, vTexCoord).r;
	if (linearDepth > 0.9) {
		FragColor = vec4(0.0);
		return;
	}

	float depth1 = texture(uDepth, vTexCoord + vec2(0.0f, uPixelSize.y)).r;
	vec2 texCoord = linearDepth > depth1 ? vTexCoord : vTexCoord + vec2(0.0f, uPixelSize.y);

	vec2 motion = vec2(0.0f);
	vec3 normal = texture(uNormal, vTexCoord).rgb;

	vec3 pixRay = computePixelDir(vTexCoord);
	vec3 position = pixRay * linearDepth;
	vec4 plane = vec4(normal, dot(position, normal));
	vec3 precalc = plane.w * vec3(uVpMatrix[0][3], uVpMatrix[1][3], uVpMatrix[2][3]);

	vec4 new = texture(uNew, vTexCoord);
	vec4 org = new;
	vec4 mi = new;
	vec4 ma = new;

	blueNoiseInit(vTexCoord);
	float tot = 1.0;
	const float goldenAngle = 2.39996323;
	const float size = 12.0;
	const float radInc = 1.0;
	float radius = 1.0;
	for (float ang = blueNoise() * goldenAngle; radius < size; ang += goldenAngle) {
		radius += radInc;
		vec2 tc = vTexCoord + vec2(cos(ang), sin(ang)) * uPixelSize * (radius);

		// Ramp function based on radius
		float t = 1.2 - radius / size;

		// Compute the planar depth (what we expect the depth to be)
		vec3 ray = computePixelDir(tc);
		vec3 hitPoint = ray / dot(ray, plane.xyz);
		float hitDepth = dot(precalc, hitPoint);

		// Check depth
		float d = texture(uDepth, tc).r;
		t *= step(abs(d - hitDepth), 8.0 / 65535.0);

		// Also check normal
		vec3 n = texture(uNormal, tc).rgb;
		t *= step(0.9, dot(normal, n));

		vec4 col = texture(uNew, tc);
		vec4 mc = mix(org, col, t);
		mi = min(mi, mc);
		ma = max(ma, mc);

		new += t*col;
		tot += t;
	}
	new /= tot;

	vec4 colOld = texture(uOld, vTexCoord - motion);
	colOld = clamp(colOld, mi, ma);

	float blendFactor = 0.5f;
	float maxVel = calculatePixelMaxVelocity(uPixelSize);
	float currVel = length(motion / uPixelSize);
	float velFactor = min(currVel, maxVel) / maxVel;
	blendFactor = mix(blendFactor, 0.1f, velFactor);

	FragColor = mix(new, colOld, blendFactor);
}
#endif
