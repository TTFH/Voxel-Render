#ifndef LIGHT_H
#define LIGHT_H

#include "camera.h"
#include "shader.h"
#include "vox_loader.h"
#include <glm/glm.hpp>

class Light {
private:
	float altitude;
	float radius;
	float azimuth;
	mat4 projection;
	VoxLoader model;

	void updatePos(float altitude, float radius, float azimuth);
public:
	vec3 position;

	Light(vec3 pos);
	void pushLight(Shader& shader);
	void pushProjection(Shader& shader);
	mat4 getProjection();
	void handleInputs(GLFWwindow* window);
	void draw(Shader& shader, Camera& camera);
};

#endif
