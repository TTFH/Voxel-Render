uniform float uNear;
uniform float uFar;
uniform float uInvFar;

uniform vec3 uCameraPos;
uniform mat4 uVpMatrix;
uniform mat4 uVpInvMatrix;

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

uniform sampler2D uColor;
uniform sampler2D uMaterial;
uniform sampler2D uAlbedoMap;
uniform sampler2D uBlendMap;
uniform sampler2D uNormalMap;
uniform sampler2D uWindowAlbedo;
uniform sampler2D uWindowNormal;

uniform mat4 uVolMatrix;

uniform vec4 uTextureTile;
uniform vec3 uTextureParams;
uniform int uPalette;
uniform float uEmissive;
uniform vec4 uVoxelSize;
uniform vec3 uObjSize;

uniform usampler3D uVolTex;

uniform bool uHasAlpha;
uniform float uHighlight;
uniform float uAlpha;

#ifdef VERTEX
layout(location = 0) in vec3 aPosition;

out vec4 vHPos;
out vec3 vLocalCameraPos;
out vec3 vLocalPos;

void main() {
	vec4 worldPos = uVolMatrix * vec4(aPosition * uObjSize.xyz * uVoxelSize.w, 1.0);
	mat3 volMatrixInv = transpose(mat3(uVolMatrix));
	vLocalCameraPos = volMatrixInv * (uCameraPos - uVolMatrix[3].xyz);
	vLocalPos = volMatrixInv * (worldPos.xyz - uVolMatrix[3].xyz);
	vHPos = uVpMatrix * worldPos;
	gl_Position = vHPos;
}
#endif

#ifdef FRAGMENT
layout(location = 0) out vec4 outputColor;
layout(location = 1) out vec3 outputNormal;
layout(location = 2) out vec4 outputMaterial;
layout(location = 4) out float outputDepth;

in vec4 vHPos;
in vec3 vLocalCameraPos;
in vec3 vLocalPos;

vec3 computeFarVec(vec2 texCoord) {
	vec4 aa = vec4(texCoord * 2.0 - vec2(1.0), 1.0, 1.0);
	aa = uVpInvMatrix * aa;
	return aa.xyz / aa.w - uCameraPos;
}

float raycastVolume(vec3 origin, vec3 dir, float dist, out uint index, out vec3 coord, out int normal, out vec4 color) {
	vec3 invDir = vec3(1.0) / (abs(dir) + vec3(0.0001));
	vec3 tSign = sign(dir);
	vec3 zSign = step(vec3(0.0), tSign);

	vec3 cmp;
	int mip = 2;
	bool opaque = mod(gl_FragCoord.x + gl_FragCoord.y, 2.0) == 0.0;
	float t = 0.0;
	while (t < dist) {
		float mipScale = float(1 << mip);
		float voxSize = uVoxelSize.w * mipScale;
		vec3 voxRes = uVoxelSize.xyz / mipScale;
		vec3 tDelta = invDir * voxSize;
		vec3 tPos = (origin + t * dir) / voxSize;
		vec3 ti = floor(tPos);
		vec3 tMax = (invDir * (zSign + tSign * (ti - tPos))) * voxSize + vec3(t);
		ti = (ti + vec3(0.5)) / voxRes;
		vec3 tStep = tSign / voxRes;
		if (mip == 0) {
			float dist2 = min(dist, t + voxSize * 5.0);
			mip = 1;
			while (t < dist2) {
				uint a = textureLod(uVolTex, ti, 0).x;
				if (a != 0u) {
					color = texelFetch(uColor, ivec2(a, uPalette), 0);
					if (opaque || color.a == 1.0) {
						index = a;
						coord = floor(ti * uVoxelSize.xyz);
						normal = int(cmp.y + cmp.z * 2.0);
						return t;
					}
				}
				cmp = step(tMax.xyz, tMax.zxy) * step(tMax.xyz, tMax.yzx);
				t = dot(tMax, cmp);
				tMax += tDelta * cmp;
				ti += tStep * cmp;
			}
		} else {
			float dist2 = (mip == 1 ? min(dist, t + voxSize * 5.0) : dist);
			mip++;
			while (t < dist2) {
				uint a = textureLod(uVolTex, ti, mip - 1).x;
				if (a != 0u) {
					mip -= 2;
					break;
				} else {
					cmp = step(tMax.xyz, tMax.zxy) * step(tMax.xyz, tMax.yzx);
					t = dot(tMax, cmp);
					tMax += tDelta * cmp;
					ti += tStep * cmp;
				}
			}
		}
	}
	return dist;
}

float distanceToBox(vec3 origin, vec3 dir, vec3 size) {
	if (clamp(origin, vec3(0.0), size) == origin)
		return 0.0;

	vec3 invDir = vec3(1.0) / dir;
	vec3 sgn = step(dir, vec3(0.0));

	vec3 tmin = (sgn * size - origin) * invDir;
	return max(max(tmin.x, tmin.y), tmin.z);
}

vec2 getTile(vec2 tc, float tile, vec2 tileCount) {
	vec2 tmp = vec2(mod(tile, tileCount.x), floor(tile / tileCount.x));
	return (tmp + fract(tc)) / tileCount;
}

void main() {
	vec2 tc = (vHPos.xy / vHPos.w) * 0.5 + vec2(0.5);

	if (uAlpha < 1.0) {
		blueNoiseInit(tc);
		if (blueNoise() > uAlpha)
			discard;
	}

	vec3 fv = computeFarVec(tc);
	float depth = 500; //texture(uDepth, tc).r;
	float currentMinDepth = length(fv * depth);

	vec3 localPos = vLocalCameraPos;
	vec3 localDir = vLocalPos - vLocalCameraPos;
	float maxDist = length(localDir);
	localDir /= maxDist;
	float minDist = distanceToBox(localPos, localDir, uObjSize.xyz * uVoxelSize.w);

	if (minDist > currentMinDepth)
		discard;

	maxDist = min(maxDist, currentMinDepth);

	vec3 or = localPos + localDir * (minDist - 0.001);
	float rmd = maxDist - minDist;
	uint index;
	vec3 coord;
	int n;
	float hitDist = raycastVolume(or, localDir, rmd, index, coord, n, outputColor);
	if (hitDist == rmd)
		hitDist = maxDist;
	else
		hitDist += minDist;
	if (hitDist < maxDist) {
		outputMaterial = texelFetch(uMaterial, ivec2(index, uPalette), 0);
		outputColor.rgb = pow(outputColor.rgb, vec3(2.2));
		vec3 localPos = vLocalCameraPos + localDir * hitDist;
		vec3 localNormal = vec3(0.0);
		localNormal[n] = -sign(localDir[n]);

		outputMaterial.w *= 32.0 * uEmissive;

		vec4 tt = (index == 254u ? vec4(12.0, 0.0, 1.0, 0.0) : uTextureTile);
		if (outputColor.a < 1.0) {
			vec3 noiseNormal;

			vec2 ntc;
			if (n == 0) {
				ntc = localPos.yz;
				noiseNormal = texture(uWindowNormal, ntc * 0.2).zxy - vec3(0.5);
			} else if (n == 1) {
				ntc = localPos.xz;
				noiseNormal = texture(uWindowNormal, ntc * 0.2).yzx - vec3(0.5);
			} else if (n == 2) {
				ntc = localPos.xy;
				noiseNormal = texture(uWindowNormal, ntc * 0.2).xyz - vec3(0.5);
			}

			localNormal += noiseNormal * 0.2;
			localNormal = normalize(localNormal);

			vec3 noise = texture(uWindowAlbedo, ntc * 0.2).xyz - vec3(0.5);
			outputColor.rgb += noise * 0.2;
			outputMaterial.x = 0.1;
			outputMaterial.y = 1.0;
		} else {
			vec2 tc;
			tc.x = (n == 0 ? coord.y + uTextureParams.y : coord.x + uTextureParams.x);
			tc.y = (n == 2 ? coord.y + uTextureParams.y : coord.z + uTextureParams.z);
			tc = floor(tc) / 256.0;

			if (tt.x != 0) {
				vec2 tcTile = getTile(tc, tt.x, vec2(4.0, 8.0));

				if (outputMaterial.w == 0.0f) {
					vec4 albedoTex = texture(uAlbedoMap, tcTile);
					albedoTex = mix(vec4(1.0), albedoTex, tt.z);
					outputColor *= albedoTex;
					outputMaterial.y *= clamp((albedoTex.r - 0.5) * 2.0, 0.0, 1.0);
				}

				vec2 normalTex = mix(vec2(0.5), texture(uNormalMap, tcTile).xy, tt.z);
				normalTex = (normalTex.xy * 2.0 - vec2(1.0)) * 0.2;

				localNormal.yz += (n == 0 ? vec2(0.0) : normalTex);
				localNormal.xz += (n == 1 ? vec2(0.0) : normalTex);
				localNormal.xy += (n == 2 ? vec2(0.0) : normalTex);
				localNormal = normalize(localNormal);
			}
			if (tt.y != 0) {
				vec2 tcTile = getTile(tc, tt.y, vec2(4.0, 4.0));

				vec4 blendTex = texture(uBlendMap, tcTile);
				blendTex.rgb = pow(blendTex.rgb, vec3(2.2));
				blendTex.a *= tt.w;
				outputColor.rgb = mix(outputColor.rgb, blendTex.rgb, blendTex.a);
				outputMaterial.y = min(outputMaterial.y, 1.0 - blendTex.a);
			}
		}

		outputColor.rgb += vec3(uHighlight) * 0.3;
		outputMaterial.w += uHighlight * 0.3;

		vec4 worldPos4 = uVolMatrix * vec4(localPos, 1.0);
		vec4 worldNormal4 = uVolMatrix * vec4(localNormal, 0.0);
		float linearDepth = (uVpMatrix * worldPos4).w;

		outputNormal = worldNormal4.xyz;
		outputDepth = linearDepth * uInvFar;

		outputColor.rgb = pow(outputColor.rgb, vec3(1/2.2));
		outputColor = clamp(outputColor, vec4(0.0), vec4(1.0));
		gl_FragDepth = (1.0 / (linearDepth + 0.001) - 1.0 / uNear) / (1.0 / uFar - 1.0 / uNear);
	} else
		discard;
}
#endif
