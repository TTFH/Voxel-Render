#include "vao.h"
#include "vox_loader.h"
#include "render_mesh.h"
#include "render_vox_greedy.h"

// Vertex Array Object
VAO::VAO() {
	glGenVertexArrays(1, &vao);
}

template <typename T>
void VAO::LinkAttrib(VBO<T>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset) {
	vbo.Bind();
	glVertexAttribPointer(index, size, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(index);
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

template void VAO::LinkAttrib<GLfloat>(VBO<GLfloat>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
template void VAO::LinkAttrib<vec2>(VBO<vec2>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
template void VAO::LinkAttrib<vec3>(VBO<vec3>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
template void VAO::LinkAttrib<GM_Vertex>(VBO<GM_Vertex>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
template void VAO::LinkAttrib<MV_Voxel>(VBO<MV_Voxel>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
template void VAO::LinkAttrib<MeshVertex>(VBO<MeshVertex>& vbo, GLuint index, GLint size, GLenum type, GLsizeiptr stride, GLvoid* offset);
