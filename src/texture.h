#ifndef TEXTURE_H
#define TEXTURE_H

#include "../glad/glad.h"
#include "shader.h"

class Texture {
private:
	GLuint unit;
public:
	GLuint texture_id;
	const char* type;
	Texture(GLuint tex_id, GLuint slot);
	Texture(const char* path, const char* texType, GLuint slot);
	~Texture();
	void Bind(Shader& shader, const char* uniform);
	void Unbind();
};

#endif
