#include "vbo.h"
#include "vox_loader.h"
#include "render_mesh.h"
#include "render_vox_greedy.h"

// Vertex Buffer Object
template <typename T>
VBO::VBO(vector<T>& vertices) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_STATIC_DRAW);
}

VBO::VBO(const GLfloat* vertices, GLsizeiptr size) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void VBO::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VBO::~VBO() {
	//glDeleteBuffers(1, &vbo);
}

template VBO::VBO(vector<GLfloat>&);
template VBO::VBO(vector<vec2>&);
template VBO::VBO(vector<vec3>&);
template VBO::VBO(vector<GM_Vertex>&);
template VBO::VBO(vector<MV_Voxel>&);
template VBO::VBO(vector<MeshVertex>&);
