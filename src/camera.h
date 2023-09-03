#ifndef CAMERA_H
#define CAMERA_H

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "shader.h"

using namespace glm;

const float FAR_PLANE = 500;

class Camera {
private:
	float speed = 0.1;
	float sensitivity = 100.0;
	bool firstClick = true;
public:
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
	mat4 cameraMatrix = mat4(1.0f);
	int screen_width;
	int screen_height;
	const vec3 up = vec3(0, 1, 0);
	vec3 position = vec3(0, 0, 0);
	vec3 orientation = vec3(0, 0, -1);

	Camera();
	void initialize(int width, int height, vec3 position);
	void updateScreenSize(int width, int height);
	void translateAndInvertPitch(float distance);
	void pushMatrix(Shader& shader, const char* uniform);
	void handleInputs(GLFWwindow* window);
};

#endif
