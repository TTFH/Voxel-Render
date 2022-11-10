#ifndef TEXTURE_H
#define TEXTURE_H

#include "../glad/glad.h"
#include "shader.h"

class Texture {
private:
	GLuint texture_id;
	GLuint unit;
public:
	const char* type;
	Texture(const char* path, const char* texType, GLuint slot);
	~Texture();
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	void Bind();
	void Unbind();
};

#endif
