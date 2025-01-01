#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

#include "camera.h"
#include "shader.h"
#include "vox_loader.h"

class Light {
private:
	float altitude;
	float radius;
	float azimuth;
	vec3 position;
	mat4 vpMatrix;
	VoxLoader model;
	void updatePos(float altitude, float radius, float azimuth);
	void updateMatrix();
public:
	Light(vec3 position);
	vec3 getPosition() const;
	mat4 getMatrix() const;
	void handleInputs(GLFWwindow* window);
	void draw(Shader& shader, Camera& camera, RenderMethod method);
};

#endif
