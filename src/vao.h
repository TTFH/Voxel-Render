#ifndef VAO_H
#define VAO_H

#include "../glad/glad.h"
#include "vbo.h"

class VAO {
private:
	GLuint vao;
public:
	VAO();
	~VAO();
	void LinkAttrib(VBO& vbo, GLuint layout, GLint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void Bind();
	void Unbind();
};

#endif
