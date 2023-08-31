uniform float time;
uniform vec2 offset;

varying vec2 uv0;

#ifdef VERTEX
layout(location = 0) in vec2 aPos;

void main() {
	gl_Position = vec4(aPos / 2.0 + offset, 0.0, 1.0);
	uv0 = 2.0 * aPos -1.0;
}
#endif

#ifdef FRAGMENT
vec3 palette(float t) {
	vec3 a = vec3(0.5, 0.5, 0.5);
	vec3 b = vec3(0.5, 0.5, 0.5);
	vec3 c = vec3(1.0, 1.0, 1.0);
	vec3 d = vec3(0.263, 0.416, 0.557);
	return a + b * cos(6.28318 * (c * t + d));
}

// An introduction to Shader Art Coding
// https://www.youtube.com/watch?v=f4s1h2YETNY
void main() {
	vec2 uv = uv0;
	vec3 finalColor = vec3(0.0);

	for (float i = 0.0; i < 4.0; i++) {
		uv = fract(uv * 1.5) - 0.5;
		float d = length(uv) * exp(-length(uv0));
		vec3 col = palette(length(uv0) + 0.4 * i + 0.2 * time);
		d = sin(8.0 * d + time / 2.0) / 8.0;
		d = abs(d);
		d = pow(0.01 / d, 1.2);
		finalColor += col * d;
	}

	gl_FragColor = vec4(finalColor, 1.0);
}
#endif
