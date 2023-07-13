#version 420 core
uniform float time; // glUniform1f(glGetUniformLocation(shader_program.id, "time"), glfwGetTime());

in vec2 uv0;
out vec4 FragColor;

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
		vec3 col = palette(length(uv0) + i * 0.4 + time * 0.4);
		d = sin(d * 8.0 + time) / 8.0;
		d = abs(d);
		d = pow(0.01 / d, 1.2);
		finalColor += col * d;
	}

	FragColor = vec4(finalColor, 1.0);
}
