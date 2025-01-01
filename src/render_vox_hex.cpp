#include "ebo.h"
#include "utils.h"
#include "render_vox_hex.h"

//   4 _ _ _ _ 3
//   /         \.
//  /           \.
//5/             \2
// \             /
//  \           /
//   \ _ _ _ _ /
//   0         1
// offset =  1.5 * x, sqrt(3) * y + (x % 2) * 0.5 * sqrt(3), z

static GLfloat hex_prism_vertices[] = {
	// Position // Normal
	0, 0, 1, 0, 0, 1,
	1, 0, 1, 0, 0, 1,
	1.5, 0.5 * sqrt(3), 1, 0, 0, 1,
	1, sqrt(3), 1, 0, 0, 1,
	0, sqrt(3), 1, 0, 0, 1,
	-0.5, 0.5 * sqrt(3), 1, 0, 0, 1,

	0, 0, 0, 0, 0, -1,
	1, 0, 0, 0, 0, -1,
	1.5, 0.5 * sqrt(3), 0, 0, 0, -1,
	1, sqrt(3), 0, 0, 0, -1,
	0, sqrt(3), 0, 0, 0, -1,
	-0.5, 0.5 * sqrt(3), 0, 0, 0, -1,

	0, 0, 0, 0, -1, 0,
	1, 0, 0, 0, -1, 0,
	1, 0, 1, 0, -1, 0,
	0, 0, 1, 0, -1, 0,

	1, 0, 1, 0.5 * sqrt(3), -0.5, 0,
	1, 0, 0, 0.5 * sqrt(3), -0.5, 0,
	1.5, 0.5 * sqrt(3), 0, 0.5 * sqrt(3), -0.5, 0,
	1.5, 0.5 * sqrt(3), 1, 0.5 * sqrt(3), -0.5, 0,

	1.5, 0.5 * sqrt(3), 1, 0.5 * sqrt(3), 0.5, 0,
	1.5, 0.5 * sqrt(3), 0, 0.5 * sqrt(3), 0.5, 0,
	1, sqrt(3), 0, 0.5 * sqrt(3), 0.5, 0,
	1, sqrt(3), 1, 0.5 * sqrt(3), 0.5, 0,

	1, sqrt(3), 1, 0, 1, 0,
	1, sqrt(3), 0, 0, 1, 0,
	0, sqrt(3), 0, 0, 1, 0,
	0, sqrt(3), 1, 0, 1, 0,

	-0.5, 0.5 * sqrt(3), 0, -0.5 * sqrt(3), 0.5, 0,
	-0.5, 0.5 * sqrt(3), 1, -0.5 * sqrt(3), 0.5, 0,
	0, sqrt(3), 1, -0.5 * sqrt(3), 0.5, 0,
	0, sqrt(3), 0, -0.5 * sqrt(3), 0.5, 0,

	0, 0, 0, -0.5 * sqrt(3), -0.5, 0,
	0, 0, 1, -0.5 * sqrt(3), -0.5, 0,
	-0.5, 0.5 * sqrt(3), 1, -0.5 * sqrt(3), -0.5, 0,
	-0.5, 0.5 * sqrt(3), 0, -0.5 * sqrt(3), -0.5, 0,
};

static GLuint hex_prism_indices[] = {
	 0,  1,  2,
	 0,  2,  3,
	 0,  3,  4,
	 0,  4,  5,

	 8,  7,  6,
	 9,  8,  6,
	10,  9,  6,
	11, 10,  6,

	12, 13, 14,
	12, 14, 15,

	16, 17, 18,
	16, 18, 19,

	20, 21, 22,
	20, 22, 23,

	24, 25, 26,
	24, 26, 27,

	28, 29, 30,
	28, 30, 31,

	32, 33, 34,
	32, 34, 35,
};

HexRender::HexRender(const MV_Shape& shape, GLuint palette_bank, int palette_id) {
	uint8_t*** voxels = MatrixInit(shape);
	TrimShape(voxels, shape.sizex, shape.sizey, shape.sizez);

	vector<MV_Voxel> trimed_voxels;
	trimed_voxels.reserve(shape.voxels.size());
	for (int i = 0; i < shape.sizex; i++)
		for (int j = 0; j < shape.sizey; j++)
			for (int k = 0; k < shape.sizez; k++)
				if (voxels[i][j][k] != 0) {
					MV_Voxel voxel = { (uint8_t)i, (uint8_t)j, (uint8_t)k, voxels[i][j][k] };
					trimed_voxels.push_back(voxel);
				}
	MatrixDelete(voxels, shape);

	this->palette_bank = palette_bank;
	this->palette_id = palette_id;
	this->voxel_count = trimed_voxels.size();

	vao.Bind();
	VBO<GLfloat> vbo(hex_prism_vertices, sizeof(hex_prism_vertices));
	EBO ebo(hex_prism_indices, sizeof(hex_prism_indices));

	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0);					  // Vertex position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Normal

	VBO instaceVBO(trimed_voxels);
	vao.LinkAttrib(instaceVBO, 2, 1, GL_UNSIGNED_BYTE, sizeof(MV_Voxel), (GLvoid*)(3 * sizeof(uint8_t))); // Texture coord
	vao.LinkAttrib(instaceVBO, 3, 3, GL_UNSIGNED_BYTE, sizeof(MV_Voxel), (GLvoid*)0);					  // Relative position
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	instaceVBO.Unbind();

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void HexRender::draw(Shader& shader, Camera& camera) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);

	shader.PushFloat("scale", scale);
	shader.PushInt("uPalette", palette_id);
	shader.PushVec3("size", vec3(0, 0, 0)); // SM flag not a voxagon
	shader.PushTexture2D("uColor", palette_bank, 1); // Texture 0 is SM

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.PushMatrix("position", pos);
	shader.PushMatrix("rotation", rot);

	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	shader.PushMatrix("world_pos", world_pos);
	shader.PushMatrix("world_rot", world_rot);

	// Use GL_LINES for wireframe
	glDrawElementsInstanced(GL_TRIANGLES, sizeof(hex_prism_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0, voxel_count);
	vao.Unbind();
}
