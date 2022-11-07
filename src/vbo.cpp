#include "vbo.h"

// Vertex Buffer Object
VBO::VBO(vector<vec2>& vertices) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), vertices.data(), GL_STATIC_DRAW);
}

VBO::VBO(vector<vec3>& vertices) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);
}

VBO::VBO(vector<Vertex>& vertices) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

VBO::VBO(vector<MV_Voxel>& vertices) {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MV_Voxel), vertices.data(), GL_STATIC_DRAW);
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
