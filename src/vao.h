#ifndef VAO_H
#define VAO_H

#include "vbo.h"

class VAO {
private:
	GLuint vao;
public:
	VAO();
	~VAO();
	void LinkAttrib(VBO& vbo, GLuint layout, GLint numComponents, GLenum type, GLsizeiptr stride, GLvoid* offset);
	void Bind();
	void Unbind();
};

#endif
