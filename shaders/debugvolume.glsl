uniform float uVolTexelSize;
uniform vec3 uCameraPos;
uniform vec3 uVolOffset;
uniform vec3 uVolResolution;
uniform mat4 uVpInvMatrix;
uniform usampler3D uShadowVolume;

uniform float uNear;
uniform float uFar;
uniform mat4 uVpMatrix;

#ifdef VERTEX
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec3 vFarVec;

vec3 computeFarVec(vec2 texCoord) {
	vec4 aa = vec4(texCoord * 2.0 - vec2(1.0), 1.0, 1.0);
	aa = uVpInvMatrix * aa;
	return aa.xyz / aa.w - uCameraPos;
}

void main() {
	gl_Position = vec4(aPosition, 0.0, 1.0);
	vFarVec = computeFarVec(aTexCoord);
}
#endif

#ifdef FRAGMENT
layout(location = 0) out vec3 outputColor;
layout(location = 1) out vec3 outputNormal;
layout(location = 2) out float outputDepth;

in vec3 vFarVec;

float raycastShadowVolume(vec3 origin, vec3 dir, float dist, out int normal) {
	origin -= uVolOffset;
	vec3 invDir = vec3(1.0) / (abs(dir) + vec3(0.00001));
	vec3 invRes = vec3(1.0) / uVolResolution;
	vec3 halfInvRes = invRes * 0.5;

	vec3 tSign = sign(dir);
	vec3 zSign = step(vec3(0.0), tSign);

	normal = 0;
	int mip = -1;
	float t = 0.0;
	while (t < dist) {
		if (mip == -1) {
			float mipScale = 0.5;
			float texelSize = uVolTexelSize * mipScale;
			vec3 invResolution = invRes * mipScale;
			vec3 tDelta = invDir * texelSize;
			vec3 tPos = (origin + t * dir) / texelSize;
			vec3 ti = floor(tPos);
			vec3 tMax = (invDir * (zSign + tSign * (ti - tPos))) * texelSize + vec3(t);
			ti = (ti + vec3(0.5)) * invResolution;
			vec3 tStep = tSign * invResolution;

			int c = 0;
			while (t < dist && c++ < 8) {
				uint a = textureLod(uShadowVolume, ti, 0).x;
				if (a != 0u) {
					uint bit = 0u;
					bit += mod(ti.x, invRes.x) > halfInvRes.x ? 1u : 0u;
					bit += mod(ti.y, invRes.y) > halfInvRes.y ? 2u : 0u;
					bit += mod(ti.z, invRes.z) > halfInvRes.z ? 4u : 0u;
					uint mask = 1u << bit;
					if ((mask & a) != 0u)
						return t;
				}

				if (tMax.x < tMax.y)
					if (tMax.x < tMax.z)
						normal = 0;
					else
						normal = 2;
				else
					if (tMax.y < tMax.z)
						normal = 1;
					else
						normal = 2;

				vec3 cmp = step(tMax.xyz, tMax.zxy) * step(tMax.xyz, tMax.yzx);
				t = dot(tMax, cmp);
				tMax += tDelta * cmp;
				ti += tStep * cmp;
			}
			mip = 0;
		} else {
			float mipScale = float(1 << mip);
			float texelSize = uVolTexelSize * mipScale;
			vec3 invResolution = invRes * mipScale;
			vec3 tDelta = invDir * texelSize;
			vec3 tPos = (origin + t * dir) / texelSize;
			vec3 ti = floor(tPos);
			vec3 tMax = (invDir * (zSign + tSign * (ti - tPos))) * texelSize + vec3(t);
			ti = (ti + vec3(0.5)) * invResolution;
			vec3 tStep = tSign * invResolution;

			int c = mip < 2 ? 8 : 1024;
			while (t < dist) {
				if (c-- == 0) {
					mip++;
					break;
				}
				uint a = textureLod(uShadowVolume, ti, mip).x;
				if (a != 0u) {
					mip--;
					break;
				}

				if (tMax.x < tMax.y)
					if (tMax.x < tMax.z)
						normal = 0;
					else
						normal = 2;
				else
					if (tMax.y < tMax.z)
						normal = 1;
					else
						normal = 2;

				vec3 cmp = step(tMax.xyz, tMax.zxy) * step(tMax.xyz, tMax.yzx);
				t = dot(tMax, cmp);
				tMax += tDelta * cmp;
				ti += tStep * cmp;
			}
		}
	}
	return dist;
}

void main() {
	vec3 pos = uCameraPos;
	vec3 dir = normalize(vFarVec);
	float maxDist = 64.0;

	int n;
	float dist = raycastShadowVolume(pos, dir, maxDist, n);
	//float a = 1.0 - dist / maxDist;

	float lineWidth = 0.02;
	vec3 vcoord = 10.0 * (pos + dir * dist);
	float ex = min(step(lineWidth, vcoord.x - floor(vcoord.x)), step(vcoord.x - floor(vcoord.x), 1.0 - lineWidth));
	float ey = min(step(lineWidth, vcoord.y - floor(vcoord.y)), step(vcoord.y - floor(vcoord.y), 1.0 - lineWidth));
	float ez = min(step(lineWidth, vcoord.z - floor(vcoord.z)), step(vcoord.z - floor(vcoord.z), 1.0 - lineWidth));
	float inner = (ex == 0.0 && ey == 0.0) || (ex == 0.0 && ez == 0.0) || (ey == 0.0 && ez == 0.0) ? 0.0 : 1.0;
	float l = 0.6 + 0.4 * inner;

	vec3 normal = vec3(0.0);
	normal[n] = -sign(dir[n]);

	vec4 hitPos = vec4(pos + dir * dist, 1.0);
	vec4 hpos = uVpMatrix * hitPos;

	outputColor = ((normal + 1.0f) / 2.0f) * l;
	outputNormal = normal;
	outputDepth = hpos.w / uFar;
	gl_FragDepth = (1.0 / hpos.w - 1.0 / uNear) / (1.0 / uFar - 1.0 / uNear);

	if (dist == maxDist)
		discard;
}
#endif
