#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include "shader.h"

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

struct Plane {
	vec3 normal = vec3(0, 1, 0);
	float distance = 0;

	Plane() {};
	Plane(const vec3& point, const vec3& normal) {
		this->normal = normalize(normal);
		this->distance = -dot(this->normal, point);
	}
};

struct Frustum {
	Plane near;
	Plane far;
	Plane right;
	Plane left;
	Plane top;
	Plane bottom;
};

class Camera {
private:
	float speed = 0.1;
	bool first_click = true;
	Frustum frustum;
	void updateMatrix();
	void updateFrustum();
public:
	const float FOV = 45;
	const float NEAR_PLANE = 0.1;
	const float FAR_PLANE = 500;
	const vec3 up = vec3(0, 1, 0);
	int screen_width, screen_height;
	bool fullscreen = false;
	vec3 position, direction;
	mat4 vp_matrix = mat4(1.0);
	Camera(vec3 position = vec3(0, 1.8, 0));
	void updateScreenSize(int width, int height);
	void handleInputs(GLFWwindow* window);
	bool isInFrustum(vector<vec3> corners);
};

#endif
