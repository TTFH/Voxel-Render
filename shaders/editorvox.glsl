uniform float uNear;		// near plane
uniform float uFar;			// far plane
uniform vec3 uCameraPos;	// camera pos

uniform mat4 uModelMatrix;	// model matrix (scale by shape size)
uniform mat4 uVpMatrix;		// view projection matrix (camera)
uniform mat4 uMvpMatrix;	// model view projection matrix: p*v*m
uniform mat4 uVolMatrix;	// local to world matrix, used for normals
uniform mat4 uVolMatrixInv;	// world to local matrix

uniform uint uMaxValue;		// max palette index
uniform int uPalette;		// palette id
uniform vec4 uMultColor;	// white, why?
uniform sampler2D uColor;	// palettes

uniform float uVolTexelSize;// voxel size
uniform vec3 uVolResolution;// shape size
uniform usampler3D uVolTex;	// shape

varying vec3 vWorldPos;
varying vec3 vLocalCameraPos;
varying vec3 vLocalPos;

varying vec3 vpos;

#ifdef VERTEX
attribute vec3 aPosition;

void main() {
	vWorldPos = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
	vLocalCameraPos = (uVolMatrixInv * vec4(uCameraPos, 1.0)).xyz;
	vLocalPos = (uVolMatrixInv * vec4(vWorldPos, 1.0)).xyz;
	gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
	vpos = aPosition;
}
#endif

#ifdef FRAGMENT
layout(location=0) out vec3 outputColor;
layout(location=1) out vec3 outputNormal;
layout(location=2) out float outputDepth;

float raycastVolume(vec3 origin, vec3 dir, float dist, int mip, out int normal, out uint value) {
	float mipScale = float(1 << mip);
	float texelSize = uVolTexelSize * mipScale;
	vec3 resolution = uVolResolution / mipScale;

	vec3 tPos = origin / texelSize;
	vec3 ti = floor(tPos);
	vec3 tStep = sign(dir);
	vec3 tDelta = vec3(1.0) / (abs(dir) + vec3(0.00001));
	vec3 tDist;
	tDist.x = tStep.x > 0 ? ti.x + 1.0 - tPos.x : tPos.x - ti.x;
	tDist.y = tStep.y > 0 ? ti.y + 1.0 - tPos.y : tPos.y - ti.y;
	tDist.z = tStep.z > 0 ? ti.z + 1.0 - tPos.z : tPos.z - ti.z;
	vec3 tMax = tDelta * tDist;

	tStep /= resolution;
	ti /= resolution;

	// Offset half a voxel to avoid rounding errors in texel fetch
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
	invDir.x = dir.x == 0 ? 0.0 : 1.0 / dir.x;
	invDir.y = dir.y == 0 ? 0.0 : 1.0 / dir.y;
	invDir.z = dir.z == 0 ? 0.0 : 1.0 / dir.z;
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

	float minDist = distanceToBox(localPos, localDir, uVolResolution * uVolTexelSize);
	int n;
	uint value;
	float hitDist = raycast(localPos, localDir, minDist, maxDist, 2, n, value);
	if (hitDist != -1.0)
		hitDist = raycast(localPos, localDir, hitDist, maxDist, 0, n, value);

	if (hitDist != -1) {
		vec4 localNormal = vec4(0.0);
		localNormal[n] = -sign(localDir[n]);
		vec3 hitNormal = (uVolMatrix * localNormal).xyz;

		vec4 hitPos = vec4(uCameraPos + away * hitDist, 1.0);
		vec4 hpos = uVpMatrix * hitPos;

		vec4 c = texelFetch(uColor, ivec2(value, uPalette), 0);
		c *= uMultColor;
		outputColor = c.rgb;
		outputNormal = hitNormal;
		outputDepth = hpos.w / uFar;
		gl_FragDepth = (1.0 / hpos.w - 1.0 / uNear) / (1.0 / uFar - 1.0 / uNear);
	} else {
		//discard;
		outputColor = vpos;
	}
}
#endif
