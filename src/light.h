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
	vec3 position;
	mat4 projection;
	VoxLoader model;
	void updatePos(float altitude, float radius, float azimuth);
	void updateProjection();
public:
	Light(vec3 pos);
	vec3 getPosition();
	mat4 getProjection();
	void handleInputs(GLFWwindow* window);
	void draw(Shader& shader, Camera& camera);
};

#endif
