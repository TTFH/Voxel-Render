#ifndef VAO_H
#define VAO_H

#include "../glad/glad.h"

class VAO {
private:
	GLuint vao;
public:
	VAO();
	~VAO();
	void LinkAttrib(GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
	void Bind();
	void Unbind();
};

#endif
