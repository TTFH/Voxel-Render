#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

Camera::Camera() { }

void Camera::initialize(int width, int height, vec3 position) {
	this->screen_width = width;
	this->screen_height = height;
	this->position = position;
}

void Camera::updateScreenSize(int width, int height) {
	screen_width = width;
	screen_height = height;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane) {
	mat4 view = mat4(1.0f);
	mat4 projection = mat4(1.0f);
	view = lookAt(position, position + orientation, up);
	projection = perspective(radians(FOVdeg), (float)screen_width / screen_height, nearPlane, farPlane);
	cameraMatrix = projection * view;
}

void Camera::pushMatrix(Shader& shader, const char* uniform) {
	GLint camera_unif = glGetUniformLocation(shader.id, uniform);
	glUniformMatrix4fv(camera_unif, 1, GL_FALSE, value_ptr(cameraMatrix));
}

void Camera::translateAndInvertPitch(float distance) {
	position.y += distance;
	orientation.y *= -1;
	updateMatrix(45, 0.1, FAR_PLANE);
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
		speed = 0.5;
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		speed = 0.1;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, screen_width / 2, screen_height / 2);
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		float rotX = sensitivity * (float)(mouseY - (screen_height / 2)) / screen_height;
		float rotY = sensitivity * (float)(mouseX - (screen_width / 2)) / screen_width;

		vec3 new_orientation = rotate(orientation, radians(-rotX), normalize(cross(orientation, up)));
		if (abs(angle(new_orientation, up) - radians(90.0f)) <= radians(85.0f))
			orientation = new_orientation;
		orientation = rotate(orientation, radians(-rotY), up);
		glfwSetCursorPos(window, screen_width / 2, screen_height / 2);
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}

	updateMatrix(45, 0.1, FAR_PLANE);
}
