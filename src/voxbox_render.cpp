#include "ebo.h"
#include "voxbox_render.h"

//   6--------7
//  /|       /|
// 2--------3 |
// | |      | |
// | 4------|-5
// |/       |/
// 0--------1
static GLfloat cube_vertices[] = {
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

static GLuint cube_indices[] = {
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
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0);					  // Vertex position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Normal
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void VoxboxRender::setWorldTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void VoxboxRender::draw(Shader& shader, Camera& camera) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);
	shader.PushVec3("color", color);
	shader.PushVec3("size", size);
	shader.PushFloat("scale", 0); // Flag not a voxel

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.PushMatrix("position", pos);
	shader.PushMatrix("rotation", rot);

	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}
