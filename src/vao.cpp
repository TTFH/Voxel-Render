#include "vao.h"

// Vertex Array Object
VAO::VAO() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

void VAO::linkAttrib(GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset) {
	glVertexAttribPointer(index, size, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(index);
}

void VAO::bind() {
	glBindVertexArray(vao);
}

void VAO::unbind() {
	glBindVertexArray(0);
}

VAO::~VAO() {
	glDeleteVertexArrays(1, &vao);
}
