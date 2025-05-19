uniform vec3 uCameraPos;
uniform mat4 uVpInvMatrix;

#ifdef VERTEX
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vFarVec;

void main() {
	gl_Position = vec4(aPosition, 0.0, 1.0);
	vTexCoord = aTexCoord;

	vec4 aa = vec4(vTexCoord * 2.0 - vec2(1.0), 1.0f, 1.0f);
	aa = uVpInvMatrix * aa;
	vFarVec = aa.xyz / aa.w - uCameraPos;
}
#endif

#ifdef FRAGMENT
uniform float uVolTexelSize;
uniform vec3 uVolOffset;
uniform vec3 uVolResolution;
uniform usampler3D uShadowVolume;

uniform float uNear;
uniform float uFar;
uniform mat4 uVpMatrix;

uniform sampler2D uNormal;
uniform sampler2D uDepth;

uniform float uRndFrame;
uniform vec2 uPixelSize;
uniform sampler2D uBlueNoise;

in vec2 vTexCoord;
in vec3 vFarVec;
out vec4 FragColor;

vec2 blueNoiseTc;

const float alpha1 = 0.618034005;
const vec2 alpha2 = vec2(0.75487762, 0.56984027);
const vec3 alpha3 = vec3(0.819172502, 0.671043575, 0.549700439);

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

vec3 blueNoise3() {
	vec3 n = texture(uBlueNoise, blueNoiseTc).rgb;
	vec3 v = fract(n + alpha3 * uRndFrame);
	blueNoiseTc += alpha2;
	return v;
}

vec3 jitterPosition(vec3 normal, vec3 dir) {
	vec3 positionOffset = vec3(0.0f, 0.0f, 0.0f);

	// Jitter around normal hide voxel artefacts
	vec3 jitter = (blueNoise3() - vec3(0.5f, 0.5f, 0.5f));
	jitter -= normal * dot(normal, jitter);

	jitter = normalize(jitter) * uVolTexelSize * 0.5f * blueNoise();
	positionOffset += jitter;

	// Offset along direction here
	positionOffset += dir * uVolTexelSize * 0.5f;

	// Continue along normal until one voxel out
	positionOffset +=
		normal * uVolTexelSize *
		(0.6f * clamp(1.0f - max(0.0f, dot(dir, normal)), 0.2f, 0.8f));

	return positionOffset;
}

bool blockedInScreenspace(vec3 start, vec3 dir) {
	float invFar = 1.0 / uFar;
	vec3 mid = start + dir * (uVolTexelSize * 0.5 * blueNoise());
	float thickness = uVolTexelSize * invFar;
	vec4 p = uVpMatrix * vec4(mid, 1.0);
	vec2 tc = (p.xy / p.w) * 0.5 + vec2(0.5);
	float pDepth = p.w * invFar * 0.99;
	float linearDepth = texture(uDepth, tc).r;
	return pDepth > linearDepth && pDepth < linearDepth + thickness;
}

float raycastShadowVolumeSuperSparse(vec3 origin, vec3 dir, float dist) {
	vec3 invShadowVolumeResolution = 1.0 / uVolResolution;
	float invVolumeTexelSize = 1.0 / uVolTexelSize;
	origin -= uVolOffset;
	float step = uVolTexelSize;
	vec3 stepDir = dir * invShadowVolumeResolution;
	vec3 pos = origin * invVolumeTexelSize * invShadowVolumeResolution;
	float baseDistance = 0.5;
	float d = 0.0;

	// Start in half res and bitmask
	stepDir *= 0.5;
	step *= 0.5;
	while (d < baseDistance) {
		uint c = textureLod(uShadowVolume, pos, 0.0).x;
		uint bit = 0u;
		bit += fract(pos.x * uVolResolution.x) > 0.5f ? 1u : 0u;
		bit += fract(pos.y * uVolResolution.y) > 0.5f ? 2u : 0u;
		bit += fract(pos.z * uVolResolution.z) > 0.5f ? 4u : 0u;
		uint mask = 1u << bit;
		if ((mask & c) != 0u) return d;

		pos += stepDir;
		d += step;
	}

	// Move up to base level
	stepDir *= 2.0;
	step *= 2.0;
	while (d < baseDistance * 2.0) {
		if (textureLod(uShadowVolume, pos, 0.0).x != 0u) return d;
		pos += stepDir;
		d += step;
	}

	// First mip
	stepDir *= 2.0;
	step *= 2.0;
	while (d < baseDistance * 4.0) {
		if (textureLod(uShadowVolume, pos, 1.0).x != 0u) return d;
		pos += stepDir;
		d += step;
	}

	// Second mip
	stepDir *= 2.0;
	step *= 2.0;
	while (d < dist) {
		if (textureLod(uShadowVolume, pos, 2.0).x != 0u) return d;
		pos += stepDir;
		d += step;
	}

	return dist;
}

float raycastShadowVolumeSparse(vec3 origin, vec3 dir, float dist) {
	vec3 invShadowVolumeResolution = 1.0 / uVolResolution;
	float invVolumeTexelSize = 1.0 / uVolTexelSize;

	origin -= uVolOffset;
	float step = uVolTexelSize;
	vec3 stepDir = dir * invShadowVolumeResolution;
	vec3 pos = origin * invVolumeTexelSize * invShadowVolumeResolution;

	stepDir *= 0.5;
	step *= 0.5;
	float lod = -1;

	float d = 0.0;
	while (d < dist) {
		uint c = textureLod(uShadowVolume, pos, lod).x;
		if (lod == -1) {
			if (c != 0u) {
				uint bit = 0u;
				bit +=
					fract(pos.x * uVolResolution.x) > 0.5f ? 1u : 0u;
				bit +=
					fract(pos.y * uVolResolution.y) > 0.5f ? 2u : 0u;
				bit +=
					fract(pos.z * uVolResolution.z) > 0.5f ? 4u : 0u;
				uint mask = 1u << bit;
				if ((mask & c) != 0u)
					return d;
				else {
					pos += stepDir;
					d += step;
				}
			} else {
				lod++;
				stepDir *= 2.0;
				step *= 2.0;
				pos += stepDir;
				d += step;
			}
		} else {
			if (c != 0u) {
				stepDir *= 0.5;
				step *= 0.5;
				lod--;
				pos -= stepDir;
				d -= step;
			} else {
				if (lod < 2) {
					c = textureLod(uShadowVolume, pos, lod + 1).x;
					if (c == 0u) {
						lod++;
						stepDir *= 2.0;
						step *= 2.0;
					}
				}
				pos += stepDir;
				d += step;
			}
		}
	}
	return dist;
}

float raycastAmbient(vec3 pos, vec3 normal, vec3 dir, float dist) {
	if (blockedInScreenspace(pos, dir)) return 0.0;
	pos += jitterPosition(normal, dir);
	return raycastShadowVolumeSuperSparse(pos, dir, dist);
}

vec3 sampleHemisphere(vec2 rand) {
	float r = sqrt(1.0f - rand.x * rand.x);
	float phi = 6.28 * rand.y;
	return vec3(cos(phi) * r, sin(phi) * r, rand.x);
}

vec3 cosineSampleHemisphere(vec2 rand) {
	float r = sqrt(rand.x);
	float theta = 6.28 * rand.y;
	float x = r * cos(theta);
	float y = r * sin(theta);
	return vec3(x, y, sqrt(max(0.0f, 1.0 - rand.x)));
}

vec4 ambientLight(vec3 pos, vec3 normal) {
	vec4 outputColor = vec4(0.0);

	vec3 t0 = abs(normal.z) > 0.5f ? vec3(0.0f, -normal.z, normal.y)
								   : vec3(-normal.y, normal.x, 0.0f);
	t0 = normalize(t0 - dot(normal, t0));
	vec3 t1 = cross(normal, t0);

	float ambientExponent = 1.3;
	vec3 ambientColor = vec3(1.0);
	vec3 constantColor = vec3(0.003);
	for (int i = 0; i < 2; i++) {
		vec3 dir = cosineSampleHemisphere(blueNoise2());
		dir = t0 * dir.x + t1 * dir.y + normal * dir.z;
		float incoming = dir.y * 0.2 + 0.8;
		float hitDist = raycastAmbient(pos, normal, dir, 24);

		float t = hitDist / float(24);
		t = pow(t, ambientExponent);

		outputColor.rgb += ambientColor * (incoming * t);

		// Ambient contribution, to avoid total darkness in crowded spaces
		outputColor.rgb += constantColor * (incoming * max(1.0f, hitDist * 0.25f));
	}
	return outputColor / float(2);
}

float raycastDirectional(vec3 pos, vec3 normal, vec3 dir, float dist) {
	if (blockedInScreenspace(pos, dir)) return 0.0;
	pos += jitterPosition(normal, dir);
	return raycastShadowVolumeSparse(pos, dir, dist);
}

void main() {
	float depth = texture(uDepth, vTexCoord).r;
	vec3 normal = texture(uNormal, vTexCoord).xyz;
	vec3 pos = uCameraPos + vFarVec * depth;
	vec4 hpos = uVpMatrix * vec4(pos, 1.0);

	blueNoiseInit(vTexCoord);
	FragColor += ambientLight(pos, normal);
	gl_FragDepth = (1.0 / hpos.w - 1.0 / uNear) / (1.0 / uFar - 1.0 / uNear);
}

#endif
