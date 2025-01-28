#ifndef LIGHT_H
#define LIGHT_H

#include "camera.h"
#include "shader.h"
#include "vox_loader.h"

#include <glm/glm.hpp>
#include "../glad/glad.h"

class Light {
private:
	float altitude;
	float radius;
	float azimuth;
	mat4 vpMatrix;
	VoxLoader* model;

	GLuint shadow_map_fbo;
	GLuint shadow_map_texture;
	const unsigned int WIDTH = 4096;
	const unsigned int HEIGHT = 4096;

	void initShadowMap();
	void updatePos(float altitude, float radius, float azimuth);
	void updateMatrix();
public:
	vec3 position;
	Light(vec3 position);
	~Light();

	void bindShadowMap(Shader& shader);
	void unbindShadowMap(Camera& camera);
	void pushUniforms(Shader& shader);

	void handleInputs(GLFWwindow* window);
	void draw(Shader& shader, Camera& camera, RenderMethod method);
};

#endif
