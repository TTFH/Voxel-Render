#ifndef VAO_H
#define VAO_H

#include "vbo.h"
#include "../glad/glad.h"

class VAO {
private:
	GLuint vao;
public:
	VAO();
	~VAO();
	template <typename T>
	void LinkAttrib(VBO<T>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
	void Bind();
	void Unbind();
};

#endif
