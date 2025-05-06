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

void Camera::updateMatrix() {
	mat4 view = lookAt(position, position + direction, up);
	mat4 projection = perspective(radians(FOV), (float)screen_width / screen_height, NEAR_PLANE, FAR_PLANE);
	vp_matrix = projection * view;
	updateFrustum();
}

void Camera::updateFrustum() {
	const vec3 front = normalize(direction);
	const vec3 right = normalize(cross(front, up));
	const vec3 up = normalize(cross(right, front));
	const vec3 front_mult_far = FAR_PLANE * front;
	const vec3 front_mult_near = NEAR_PLANE * front;
	const float half_v_side = FAR_PLANE * tanf(radians(FOV) * 0.5f);
	const float half_h_side = half_v_side * (float)screen_width / screen_height;

	frustum.near = Plane(position + front_mult_near, front);
	frustum.far = Plane(position + front_mult_far, -front);
	frustum.right = Plane(position, cross(front_mult_far - right * half_h_side, up));
	frustum.left = Plane(position, cross(up, front_mult_far + right * half_h_side));
	frustum.top = Plane(position, cross(right, front_mult_far - up * half_v_side));
	frustum.bottom = Plane(position, cross(front_mult_far + up * half_v_side, right));
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
		float rot_x = sensitivity * (mouse_y - (screen_height / 2.0)) / screen_height;
		float rot_y = sensitivity * (mouse_x - (screen_width / 2.0)) / screen_width;

		vec3 new_orientation = rotate(direction, radians(-rot_x), normalize(cross(direction, up)));
		if (abs(angle(new_orientation, up) - radians(90.0f)) <= radians(85.0f))
			direction = new_orientation;
		direction = rotate(direction, radians(-rot_y), up);
		glfwSetCursorPos(window, screen_width / 2.0, screen_height / 2.0);
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		first_click = true;
	}
	updateMatrix();
}

bool Camera::isInFrustum(vector<vec3> corners) {
	vector<Plane> planes = {
		frustum.near, frustum.far,
		frustum.left, frustum.right,
		frustum.top, frustum.bottom
	};
	for (vector<Plane>::iterator plane = planes.begin(); plane != planes.end(); plane++) {
		int outside_count = 0;
		for (vector<vec3>::iterator corner = corners.begin(); corner != corners.end(); corner++) {
			float dist = dot(plane->normal, *corner) + plane->distance;
			if (dist < 0) outside_count++;
		}
		if (outside_count == 8)
			return false;
	}
	return true;
}
