#include "ebo.h"
#include "utils.h"
#include "vox_rtx.h"
#include <glm/gtc/type_ptr.hpp>

static GLfloat cube_vertices[] = {
	0, 0, 0,
	0, 0, 1,
	0, 1, 0,
	0, 1, 1,
	1, 0, 0,
	1, 0, 1,
	1, 1, 0,
	1, 1, 1,
};

static GLuint cube_indices[] = {
	2, 1, 0,
	6, 2, 0,
	1, 4, 0,
	4, 6, 0,
	2, 3, 1,
	5, 4, 1,
	7, 5, 1,
	3, 7, 1,
	6, 3, 2,
	6, 7, 3,
	5, 6, 4,
	7, 6, 5,
};

RTX_Render::RTX_Render(const MV_Shape& shape, int paletteId, GLuint paletteBank) {
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	shapeSize = vec3(shape.sizex, shape.sizey, shape.sizez);
	this->paletteId = paletteId;
	this->paletteBank = paletteBank;
}

void RTX_Render::setTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void RTX_Render::setWorldTransform(vec3 position, quat rotation) {
	this->world_position = position;
	this->world_rotation = rotation;
}

void RTX_Render::draw(Shader& shader, Camera& camera, vec4 clip_plane, float scale) {
	vao.Bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
