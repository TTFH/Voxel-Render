#ifndef SKYBOX_H
#define SKYBOX_H

#include "../glad/glad.h"

#include "camera.h"
#include "shader.h"

class Skybox {
private:
	GLuint skyboxVAO;
	GLuint cubemapTexture;
	float aspectRatio;
public:
	Skybox(Shader& skyboxProgram, float aspectRatio);
	void Draw(Shader& skyboxProgram, Camera& camera);
};

#endif
