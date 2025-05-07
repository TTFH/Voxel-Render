#include <glm/gtc/matrix_transform.hpp>

#include "light.h"

void Light::updatePos(float altitude, float radius, float azimuth) {
	position = vec3(radius * cos(azimuth), altitude, radius * sin(azimuth));
}

void Light::updateMatrix() {
	mat4 view = lookAt(position, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 projection = ortho(-90.0f, 90.0f, -90.0f, 90.0f, 0.1f, 500.0f);
	vp_matrix = projection * view;
}

Light::Light(vec3 pos) {
	position = pos;
	altitude = pos.y;
	radius = sqrt(pos.x * pos.x + pos.z * pos.z);
	azimuth = atan2(pos.z, pos.x);
	updateMatrix();
	initShadowMap();
}

void Light::handleInputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)
		return;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		altitude += 0.5f;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		altitude -= 0.5f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		radius += 0.3f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		radius -= 0.3f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		azimuth += 0.01f;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		azimuth -= 0.01f;
	updatePos(altitude, radius, azimuth);
	updateMatrix();
}

void Light::initShadowMap() {
	glGenTextures(1, &shadow_map_texture);
	glBindTexture(GL_TEXTURE_2D, shadow_map_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float clamp_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clamp_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glGenFramebuffers(1, &shadow_map_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Light::bindShadowMap(Shader& shader) {
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
	glViewport(0, 0, WIDTH, HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	shader.use();
	shader.pushMatrix("lightMatrix", vp_matrix);
}

void Light::unbindShadowMap(Camera& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, camera.screen_width, camera.screen_height);
}

void Light::pushUniforms(Shader& shader) {
	shader.pushVec3("light_pos", position);
	shader.pushMatrix("lightMatrix", vp_matrix);
	shader.pushTexture2D("shadowMap", shadow_map_texture, 0);
}

Light::~Light() {
	glDeleteTextures(1, &shadow_map_texture);
	glDeleteFramebuffers(1, &shadow_map_fbo);
}
