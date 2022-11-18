#include "vao.h"

// Vertex Array Object
VAO::VAO() {
	glGenVertexArrays(1, &vao);
}

void VAO::LinkAttrib(VBO& vbo, GLuint layout, GLint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
	vbo.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	vbo.Unbind();
}

void VAO::Bind() {
	glBindVertexArray(vao);
}

void VAO::Unbind() {
	glBindVertexArray(0);
}

VAO::~VAO() {
	glDeleteVertexArrays(1, &vao);
}
