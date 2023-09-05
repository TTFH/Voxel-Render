#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

Camera::Camera() { }

void Camera::initialize(int width, int height, vec3 pos) {
	screen_width = width;
	screen_height = height;
	position = pos;
	direction = vec3(0, 0, -1);
	updateMatrix();
}

void Camera::updateScreenSize(int width, int height) {
	screen_width = width;
	screen_height = height;
}

void Camera::updateMatrix() {
	mat4 view = lookAt(position, position + direction, up);
	mat4 projection = perspective(radians(FOV), (float)screen_width / screen_height, NEAR_PLANE, FAR_PLANE);
	vpMatrix = projection * view;
}

void Camera::translateAndInvertPitch(float distance) {
	position.y += distance;
	direction.y *= -1;
	updateMatrix();
}

void Camera::handleInputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		position += speed * direction;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		position += speed * -normalize(cross(direction, up));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		position += speed * -direction;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		position += speed * normalize(cross(direction, up));
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

		vec3 new_orientation = rotate(direction, radians(-rotX), normalize(cross(direction, up)));
		if (abs(angle(new_orientation, up) - radians(90.0f)) <= radians(85.0f))
			direction = new_orientation;
		direction = rotate(direction, radians(-rotY), up);
		glfwSetCursorPos(window, screen_width / 2, screen_height / 2);
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
	updateMatrix();
}
