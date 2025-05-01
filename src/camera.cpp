#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "camera.h"
#include "utils.h"

Camera::Camera(vec3 pos) {
	screen_width = WINDOW_WIDTH;
	screen_height = WINDOW_HEIGHT;
	position = pos;
	direction = vec3(0, 0, -1);
	updateMatrix();
}

void Camera::updateScreenSize(int width, int height) {
	screen_width = width;
	screen_height = height;
	updateMatrix();
}

void Camera::updateFarPlane(float distance) {
	FAR_PLANE = distance;
	updateMatrix();
}

void Camera::translateAndInvertPitch(float distance) {
	position.y += distance;
	direction.y *= -1;
	updateMatrix();
}

void Camera::updateMatrix() {
	mat4 view = lookAt(position, position + direction, up);
	mat4 projection = perspective(radians(FOV), (float)screen_width / screen_height, NEAR_PLANE, FAR_PLANE);
	vpMatrix = projection * view;
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

		if (first_click) {
			glfwSetCursorPos(window, screen_width / 2, screen_height / 2);
			first_click = false;
		}

		double mouse_x, mouse_y;
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		const float sensitivity = 100.0;
		float rotX = sensitivity * (mouse_y - (screen_height / 2.0)) / screen_height;
		float rotY = sensitivity * (mouse_x - (screen_width / 2.0)) / screen_width;

		vec3 new_orientation = rotate(direction, radians(-rotX), normalize(cross(direction, up)));
		if (abs(angle(new_orientation, up) - radians(90.0f)) <= radians(85.0f))
			direction = new_orientation;
		direction = rotate(direction, radians(-rotY), up);
		glfwSetCursorPos(window, screen_width / 2.0, screen_height / 2.0);
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		first_click = true;
	}
	updateMatrix();
}

Frustum Camera::getFrustum() {
	Frustum frustum;
	const vec3 front = normalize(direction);
	const vec3 right = normalize(cross(front, up));
	const vec3 up = normalize(cross(right, front));
	const vec3 frontMultFar = FAR_PLANE * front;
	const vec3 frontMultNear = NEAR_PLANE * front;
	const float halfVSide = FAR_PLANE * tanf(radians(FOV) * 0.5f);
	const float halfHSide = halfVSide * (float)screen_width / screen_height;

	frustum.near = Plane(position + frontMultNear, front);
	frustum.far = Plane(position + frontMultFar, -front);
	frustum.right = Plane(position, cross(frontMultFar - right * halfHSide, up));
	frustum.left = Plane(position, cross(up, frontMultFar + right * halfHSide));
	frustum.top = Plane(position, cross(right, frontMultFar - up * halfVSide));
	frustum.bottom = Plane(position, cross(frontMultFar + up * halfVSide, right));
	return frustum;
}
