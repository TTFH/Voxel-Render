#ifndef CAMERA_H
#define CAMERA_H

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "shader.h"

using namespace glm;

class Camera {
private:
	const float FOV = 45;
	float speed = 0.1;
	float sensitivity = 100.0;
	bool firstClick = true;
	void updateMatrix();
public:
	const float NEAR_PLANE = 0.1;
	const float FAR_PLANE = 1000;
	int screen_width, screen_height;
	const vec3 up = vec3(0, 1, 0);
	vec3 position, direction;
	mat4 vpMatrix = mat4(1.0);
	Camera(vec3 pos = vec3(0, 0, 0));
	void updateScreenSize(int width, int height);
	void translateAndInvertPitch(float distance);
	void handleInputs(GLFWwindow* window);
};

#endif
