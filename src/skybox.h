#ifndef SKYBOX_H
#define SKYBOX_H

#include "../glad/glad.h"

#include "vao.h"
#include "camera.h"
#include "shader.h"

class Skybox {
private:
	VAO vao;
	GLuint texture;
public:
	Skybox();
	void draw(Shader& shader, Camera& camera);
};

#endif
