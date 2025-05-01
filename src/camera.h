#ifndef CAMERA_H
#define CAMERA_H

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
	void updateMatrix();
public:
	const float FOV = 45;
	const float NEAR_PLANE = 1; // 0.1 - 1000
	float FAR_PLANE = 500;
	const vec3 up = vec3(0, 1, 0);
	int screen_width, screen_height;
	bool fullscreen = false;
	vec3 position, direction;
	mat4 vpMatrix = mat4(1.0);
	Camera(vec3 position = vec3(0, 1.8, 0));
	void updateScreenSize(int width, int height);
	void updateFarPlane(float distance);
	void translateAndInvertPitch(float distance);
	void handleInputs(GLFWwindow* window);
	Frustum getFrustum();
};

#endif
