#include "vbo.h"
#include "vox_loader.h"
#include "render_mesh.h"
#include "render_vox_greedy.h"

// Vertex Buffer Object
template <typename T>
VBO<T>::VBO(vector<T>& vertices) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_STATIC_DRAW);
}

template <typename T>
VBO<T>::VBO(const GLfloat* vertices, GLsizeiptr size) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

template <typename T>
void VBO<T>::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

template <typename T>
void VBO<T>::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template <typename T>
VBO<T>::~VBO() {
	//glDeleteBuffers(1, &vbo);
}

template class VBO<GLfloat>;
template class VBO<vec2>;
template class VBO<vec3>;
template class VBO<GM_Vertex>;
template class VBO<MV_Voxel>;
template class VBO<MeshVertex>;
