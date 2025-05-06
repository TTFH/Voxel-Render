#ifndef SKYBOX_H
#define SKYBOX_H

#include "../glad/glad.h"

#include "vao.h"
#include "camera.h"
#include "shader.h"

class Skybox {
private:
	VAO vao;
	GLuint texture = 0;
public:
	Skybox(const char* name);
	void reloadTexture(const char* name);
	void draw(Shader& shader, Camera& camera);
	~Skybox();
};

#endif
