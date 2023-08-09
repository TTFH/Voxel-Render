#ifndef TEXTURE_H
#define TEXTURE_H

#include "../glad/glad.h"
#include "shader.h"

class Texture {
public:
	GLuint texture_id;
	const char* type;
	Texture(const char* path, const char* texType);
	~Texture();
	void Bind(Shader& shader, const char* uniform, GLuint unit);
	void Unbind();
};

#endif
