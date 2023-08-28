#include <glm/gtc/type_ptr.hpp>
#include "light.h"

void Light::updatePos(float altitude, float radius, float azimuth) {
	position = vec3(radius * cos(azimuth), altitude, radius * sin(azimuth));
}

Light::Light(vec3 pos) {
	position = pos;
	altitude = pos.y;
	radius = sqrt(pos.x * pos.x + pos.z * pos.z);
	azimuth = atan2(pos.z, pos.x);
	model.load("light.vox");
}

void Light::pushLight(Shader& shader) {
	shader.Use();
	glUniform3fv(glGetUniformLocation(shader.id, "lightpos"), 1, value_ptr(position));
}

void Light::pushProjection(Shader& shader) {
	mat4 orthgonalProjection = ortho(-90.0f, 90.0f, -90.0f, 90.0f, 0.1f, 500.0f);
	mat4 lightView = lookAt(position, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	projection = orthgonalProjection * lightView;
	shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "lightProjection"), 1, GL_FALSE, value_ptr(projection));
}

mat4 Light::getProjection() {
	return projection;
}

void Light::handleInputs(GLFWwindow* window) {
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
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		printf("Light position: (%.2f, %.2f, %.2f)\n", position.x, position.y, position.z);
}

void Light::draw(Shader& shader, Camera& camera) {
	model.draw(shader, camera, vec4(0, 1, 0, 0), position);
}
