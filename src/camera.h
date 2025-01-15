#ifndef CAMERA_H
#define CAMERA_H

#include "shader.h"

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

class Camera {
private:
	float speed = 0.1;
	bool first_click = true;
	void updateMatrix();
public:
	const float FOV = 45;
	const float NEAR_PLANE = 0.1;
	const float FAR_PLANE = 1000;
	int screen_width, screen_height;
	const vec3 up = vec3(0, 1, 0);
	vec3 position, direction;
	mat4 vpMatrix = mat4(1.0);
	Camera(vec3 position = vec3(0, 1.8, 0));
	void updateScreenSize(int width, int height);
	void translateAndInvertPitch(float distance);
	void handleInputs(GLFWwindow* window);
};

#endif
