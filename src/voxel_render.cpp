#include "ebo.h"
#include "voxel_render.h"
#include <glm/gtc/type_ptr.hpp>

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
	1, 0, 0,	 0,  0, -1,
	0, 1, 0,	 0,  0, -1,
	1, 1, 0,	 0,  0, -1,

	0, 0, 0,	 0, -1,  0,
	0, 0, 1,	 0, -1,  0,
	1, 0, 0,	 0, -1,  0,
	1, 0, 1,	 0, -1,  0,

	0, 0, 0,	-1,  0,  0,
	0, 1, 0,	-1,  0,  0,
	0, 0, 1,	-1,  0,  0,
	0, 1, 1,	-1,  0,  0,

	1, 1, 1,	 0,  0,  1,
	1, 0, 1,	 0,  0,  1,
	0, 1, 1,	 0,  0,  1,
	0, 0, 1,	 0,  0,  1,

	1, 1, 1,	 0,  1,  0,
	0, 1, 1,	 0,  1,  0,
	1, 1, 0,	 0,  1,  0,
	0, 1, 0,	 0,  1,  0,

	1, 1, 1,	 1,  0,  0,
	1, 1, 0,	 1,  0,  0,
	1, 0, 1,	 1,  0,  0,
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

//  4 _ _ _ _ 3
//   /       \
//  /         \
// / 5         \ 2
// \           /
//  \         /
//   \_ _ _ _/
//   0       1
/*
0, 0
1, 0
1.5, 0.5 * sqrt(3)
1, sqrt(3)
0, sqrt(3)
-0.5, 0.5 * sqrt(3)

offsetx
0, 0
1.5, 0.5 * sqrt(3)
3, 0
4.5, 0.5 * sqrt(3)
1.5 * x, x % 2 == 0 ? 0 : 0.5 * sqrt(3)

offsety
0, 0
0, sqrt(3)
0, 2 * sqrt(3)
0, y * sqrt(3)

offsetxy
1.5 * x, sqrt(3) * y + (x % 2 == 0 ? 0 : 0.5 * sqrt(3))
*/

static GLfloat hex_prism_vertices[] = {
	// Position				// Normal
	0, 0, 0,				0,  0, -1,
	1, 0, 0,				0,  0, -1,
	1.5, 0.5 * sqrt(3), 0,	0,  0, -1,
	1, sqrt(3), 0,			0,  0, -1,
	0, sqrt(3), 0,			0,  0, -1,
	-0.5, 0.5 * sqrt(3), 0,	0,  0, -1,

	0, 0, 1,				0,  0,  1,
	1, 0, 1,				0,  0,  1,
	1.5, 0.5 * sqrt(3), 1,	0,  0,  1,
	1, sqrt(3), 1,			0,  0,  1,
	0, sqrt(3), 1,			0,  0,  1,
	-0.5, 0.5 * sqrt(3), 1,	0,  0,  1,

	// + 6 faces * 4 vertices
};

static GLuint hex_prism_indices[] = {
	 0,  1,  2,
};

VoxelRender::VoxelRender(vector<MV_Voxel> voxels, GLuint texture_id) {
	this->texture_id = texture_id;
	this->voxel_count = voxels.size();
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(hex_prism_vertices));
	EBO ebo(cube_indices, sizeof(hex_prism_indices));

	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0);					  // Vertex position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Normal

	VBO instaceVBO(voxels);
	instaceVBO.Bind();
	vao.LinkAttrib(instaceVBO, 2, 1, GL_UNSIGNED_BYTE, sizeof(MV_Voxel), (GLvoid*)(3 * sizeof(uint8_t))); // Texture coord
	vao.LinkAttrib(instaceVBO, 3, 3, GL_UNSIGNED_BYTE, sizeof(MV_Voxel), (GLvoid*)0);					  // Relative position
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	instaceVBO.Unbind();

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void VoxelRender::setTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void VoxelRender::setWorldTransform(vec3 position, quat rotation) {
	this->world_position = position;
	this->world_rotation = rotation;
}

void VoxelRender::draw(Shader& shader, Camera& camera, vec4 clip_plane, float scale) {
	shader.Use();
	vao.Bind();
	camera.pushMatrix(shader, "camera");

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "position"), 1, GL_FALSE, value_ptr(pos));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "rotation"), 1, GL_FALSE, value_ptr(rot));

	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "world_pos"), 1, GL_FALSE, value_ptr(world_pos));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "world_rot"), 1, GL_FALSE, value_ptr(world_rot));

	glUniform4fv(glGetUniformLocation(shader.id, "clip_plane"), 1, value_ptr(clip_plane));

	glUniform1f(glGetUniformLocation(shader.id, "scale"), scale);
	glUniform1i(glGetUniformLocation(shader.id, "palette"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, texture_id);

	// Use GL_LINES for wireframe
	glDrawElementsInstanced(GL_TRIANGLES, sizeof(hex_prism_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0, voxel_count);
	vao.Unbind();
}
