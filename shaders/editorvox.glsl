uniform float uNear;
uniform float uFar;
uniform vec3 uCameraPos;

uniform mat4 uModelMatrix;
uniform mat4 uVpMatrix;
uniform mat4 uMvpMatrix;
uniform mat4 uVolMatrix;
uniform mat4 uVolMatrixInv;

uniform uint uMaxValue;
uniform int uPalette;
uniform vec4 uMultColor;
uniform sampler2D uColor;

uniform float uVolTexelSize;
uniform vec3 uVolResolution;
uniform usampler3D uVolTex;

#ifdef VERTEX
layout(location = 0) in vec3 aPosition;

out vec3 vWorldPos;
out vec3 vLocalPos;
out vec3 vLocalCameraPos;

void main() {
	vWorldPos = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
	vLocalPos = (uVolMatrixInv * vec4(vWorldPos, 1.0)).xyz;
	vLocalCameraPos = (uVolMatrixInv * vec4(uCameraPos, 1.0)).xyz;
	gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
}
#endif

#ifdef FRAGMENT
layout(location = 0) out vec3 outputColor;
layout(location = 1) out vec3 outputNormal;
layout(location = 4) out float outputDepth;

in vec3 vWorldPos;
in vec3 vLocalPos;
in vec3 vLocalCameraPos;

float raycastVolume(vec3 origin, vec3 dir, float dist, int mip, out int normal, out uint value) {
	float mipScale = float(1 << mip);
	float texelSize = uVolTexelSize * mipScale;
	vec3 resolution = uVolResolution / mipScale;

	vec3 tPos = origin / texelSize;
	vec3 ti = floor(tPos);
	vec3 tStep = sign(dir);
	vec3 tDelta = vec3(1.0) / (abs(dir) + vec3(0.00001));
	vec3 tDist;
	tDist.x = tStep.x > 0.0 ? ti.x + 1.0 - tPos.x : tPos.x - ti.x;
	tDist.y = tStep.y > 0.0 ? ti.y + 1.0 - tPos.y : tPos.y - ti.y;
	tDist.z = tStep.z > 0.0 ? ti.z + 1.0 - tPos.z : tPos.z - ti.z;
	vec3 tMax = tDelta * tDist;

	tStep /= resolution;
	ti /= resolution;
	ti += 0.5 / resolution;

	float t = 0.0;
	float tLength = dist / texelSize;
	normal = 0;
	int iter = 0;

	while (t < tLength) {
		if (iter++ > 200)
			return -1.0;
		uint a = textureLod(uVolTex, ti, mip).x;
		if (mip == 0) {
			if (a > 0u && a <= uMaxValue) {
				float alpha = texelFetch(uColor, ivec2(a, uPalette), 0).a * uMultColor.a;
				if (alpha == 1.0 || mod(gl_FragCoord.x + gl_FragCoord.y, 2.0) == 0.0) {
					value = a;
					return t * texelSize;
				}
			}
		} else {
			value = a;
			return t * texelSize;
		}

		if (tMax.x < tMax.y) {
			if (tMax.x < tMax.z) {
				ti.x += tStep.x;
				t = tMax.x;
				tMax.x += tDelta.x;
				normal = 0;
			} else {
				ti.z += tStep.z;
				t = tMax.z;
				tMax.z += tDelta.z;
				normal = 2;
			}
		} else {
			if (tMax.y < tMax.z) {
				ti.y += tStep.y;
				t = tMax.y;
				tMax.y += tDelta.y;
				normal = 1;
			} else {
				ti.z += tStep.z;
				t = tMax.z;
				tMax.z += tDelta.z;
				normal = 2;
			}
		}
	}
	return -1.0;
}

float raycast(vec3 origin, vec3 dir, float minDist, float maxDist, int mip, out int normal, out uint value) {
	float offset = uVolTexelSize * 0.5;
	vec3 newOrigin = origin + dir * (minDist - offset);
	float newMaxDist = maxDist - minDist + offset;

	float hitDist = raycastVolume(newOrigin, dir, newMaxDist, mip, normal, value);
	if (hitDist != -1.0)
		return minDist + hitDist - offset;
	else
		return -1.0;
}

float distanceToBox(vec3 origin, vec3 dir, vec3 size) {
	if (clamp(origin, vec3(0.0), size) == origin)
		return 0.0;

	vec3 invDir;
	invDir.x = dir.x == 0.0 ? 0.0 : 1.0 / dir.x;
	invDir.y = dir.y == 0.0 ? 0.0 : 1.0 / dir.y;
	invDir.z = dir.z == 0.0 ? 0.0 : 1.0 / dir.z;
	vec3 sgn = step(dir, vec3(0.0));

	vec3 tmin = (sgn * size - origin) * invDir;
	return max(max(tmin.x, tmin.y), tmin.z);
}

void main() {
	vec3 away = vWorldPos - uCameraPos;
	float maxDist = length(away);
	away /= maxDist;

	vec3 localPos = vLocalCameraPos;
	vec3 localDir = (vLocalPos - vLocalCameraPos) / maxDist;

	int normal = 0;
	uint value = 0u;
	float minDist = distanceToBox(localPos, localDir, uVolResolution * uVolTexelSize);
	float hitDist = raycast(localPos, localDir, minDist, maxDist, 2, normal, value);
	if (hitDist != -1.0)
		hitDist = raycast(localPos, localDir, hitDist, maxDist, 0, normal, value);

	if (hitDist != -1.0) {
		vec4 localNormal = vec4(0.0);
		localNormal[normal] = -sign(localDir[normal]);
		vec3 hitNormal = (uVolMatrix * localNormal).xyz;

		vec4 hitPos = vec4(uCameraPos + away * hitDist, 1.0);
		vec4 hpos = uVpMatrix * hitPos;

		vec4 color = texelFetch(uColor, ivec2(value, uPalette), 0);
		color *= uMultColor;

		outputColor = color.rgb;
		outputNormal = hitNormal;
		outputDepth = hpos.w / uFar;
		gl_FragDepth = (1.0 / hpos.w - 1.0 / uNear) / (1.0 / uFar - 1.0 / uNear);
	} else
		discard;
}
#endif
