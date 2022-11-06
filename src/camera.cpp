#include "camera.h"

using namespace glm;

Camera::Camera() { }

void Camera::initialize(int width, int height, vec3 position) {
	this->width = width;
	this->height = height;
	this->position = position;
}

void Camera::updateScreenSize(int width, int height) {
	this->width = width;
	this->height = height;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane) {
	mat4 view = mat4(1.0f);
	mat4 projection = mat4(1.0f);
	view = lookAt(position, position + orientation, up);
	projection = perspective(radians(FOVdeg), (float)width / height, nearPlane, farPlane);
	cameraMatrix = projection * view;
}

void Camera::pushMatrix(Shader& shader, const char* uniform) {
	GLint loc_ref = glGetUniformLocation(shader.id, uniform);
	glUniformMatrix4fv(loc_ref, 1, GL_FALSE, value_ptr(cameraMatrix));
}

void Camera::handleInputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		position += speed * orientation;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		position += speed * -normalize(cross(orientation, up));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		position += speed * -orientation;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		position += speed * normalize(cross(orientation, up));
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		position += speed * up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		position += speed * -up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed = 0.4;
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		speed = 0.1;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, width / 2, height / 2);
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		vec3 neworientation = rotate(orientation, radians(-rotX), normalize(cross(orientation, up)));
		if (abs(angle(neworientation, up) - radians(90.0f)) <= radians(85.0f))
			orientation = neworientation;
		orientation = rotate(orientation, radians(-rotY), up);
		glfwSetCursorPos(window, width / 2, height / 2);
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}
