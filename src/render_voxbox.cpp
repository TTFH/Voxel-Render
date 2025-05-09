#include "ebo.h"
#include "vbo.h"
#include "render_voxbox.h"

//   6--------7
//  /|       /|
// 2--------3 |
// | |      | |
// | 4------|-5
// |/       |/
// 0--------1
static const GLfloat cube_vertices[] = {
	// Position  // Normal
	0, 0, 0,	 0,  0, -1,
	0, 1, 0,	 0,  0, -1,
	1, 0, 0,	 0,  0, -1,
	1, 1, 0,	 0,  0, -1,

	0, 0, 0,	 0, -1,  0,
	1, 0, 0,	 0, -1,  0,
	0, 0, 1,	 0, -1,  0,
	1, 0, 1,	 0, -1,  0,

	0, 0, 0,	-1,  0,  0,
	0, 0, 1,	-1,  0,  0,
	0, 1, 0,	-1,  0,  0,
	0, 1, 1,	-1,  0,  0,

	1, 1, 1,	 0,  0,  1,
	0, 1, 1,	 0,  0,  1,
	1, 0, 1,	 0,  0,  1,
	0, 0, 1,	 0,  0,  1,

	1, 1, 1,	 0,  1,  0,
	1, 1, 0,	 0,  1,  0,
	0, 1, 1,	 0,  1,  0,
	0, 1, 0,	 0,  1,  0,

	1, 1, 1,	 1,  0,  0,
	1, 0, 1,	 1,  0,  0,
	1, 1, 0,	 1,  0,  0,
	1, 0, 0,	 1,  0,  0,
};

static const GLuint cube_indices[] = {
	 0,  1,  2,
	 2,  1,  3,
	 4,  5,  6,
	 6,  5,  7,
	 8,  9, 10,
	10,  9, 11,
	12, 13, 14,
	14, 13, 15,
	16, 17, 18,
	18, 17, 19,
	20, 21, 22,
	22, 21, 23,
};

VoxboxRender::VoxboxRender(vec3 size, vec3 color) {
	this->size = size;
	this->color = color;
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.linkAttrib(0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0);					 // Vertex position
	vao.linkAttrib(1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Normal
	vao.unbind();
	vbo.unbind();
	ebo.unbind();
}

void VoxboxRender::setColor(vec3 color) {
	this->color = color;
}

void VoxboxRender::setWorldTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void VoxboxRender::draw(Shader& shader, Camera& camera) {
	shader.pushMatrix("camera", camera.vp_matrix);

	shader.pushVec3("color", color);
	shader.pushVec3("size", size);
	shader.pushFloat("scale", 0); // SM flag not a voxel

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.pushMatrix("position", pos);
	shader.pushMatrix("rotation", rot);

	vao.bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.unbind();
}
