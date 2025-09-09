#include <unordered_set>

#include "ebo.h"
#include "vbo.h"
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

static const GLfloat hex_prism_vertices[] = {
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

static const GLuint hex_prism_indices[] = {
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

static void TrimVoxels(const vector<MV_Voxel>& voxels, vector<MV_Voxel>& trimmed) {
	unordered_set<tuple<uint8_t, uint8_t, uint8_t>, VoxelHash> positions;
	for (vector<MV_Voxel>::const_iterator it = voxels.begin(); it != voxels.end(); it++) {
		const MV_Voxel& voxel = *it;
		if (voxel.index != HOLE_INDEX)
			positions.insert({ voxel.x, voxel.y, voxel.z });
	}

	for (vector<MV_Voxel>::const_iterator it = voxels.begin(); it != voxels.end(); it++) {
		bool is_exposed = false;
		const MV_Voxel& voxel = *it;
		if (voxel.index == HOLE_INDEX)
			continue;

		int dx[] = { -1, 1, 0, 0, 0, 0 };
		int dy[] = { 0, 0, -1, 1, 0, 0 };
		int dz[] = { 0, 0, 0, 0, -1, 1 };

		for (int i = 0; i < 6; i++) {
			int nx = voxel.x + dx[i];
			int ny = voxel.y + dy[i];
			int nz = voxel.z + dz[i];
			if (nx < 0 || ny < 0 || nz < 0 || nx > 255 || ny > 255 || nz > 255) {
				is_exposed = true;
				break;
			}
			const tuple<uint8_t, uint8_t, uint8_t> neighbor_pos = make_tuple(nx, ny, nz);
			if (positions.find(neighbor_pos) == positions.end()) {
				is_exposed = true;
				break;
			}
		}
		if (is_exposed)
			trimmed.push_back(voxel);
	}
}

HexRender::HexRender(const MV_Shape& shape, int palette_id) {
	vector<MV_Voxel> trimmed;
	TrimVoxels(shape.voxels, trimmed);

	this->palette_id = palette_id;
	this->voxel_count = trimmed.size();
	shape_size = vec3(shape.sizex, shape.sizey, shape.sizez);

	VBO vbo(hex_prism_vertices, sizeof(hex_prism_vertices));
	EBO ebo(hex_prism_indices, sizeof(hex_prism_indices));

	vao.linkAttrib(0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0);					 // Vertex position
	vao.linkAttrib(1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Normal

	VBO instaceVBO(trimmed);
	vao.linkAttrib(2, 1, GL_UNSIGNED_BYTE, sizeof(MV_Voxel), (GLvoid*)(3 * sizeof(uint8_t))); // Palette index
	vao.linkAttrib(3, 3, GL_UNSIGNED_BYTE, sizeof(MV_Voxel), (GLvoid*)0);					  // Voxel position
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	instaceVBO.unbind();

	vao.unbind();
	vbo.unbind();
	ebo.unbind();
}

void HexRender::draw(Shader& shader, Camera& camera) {
	shader.pushMatrix("camera", camera.vp_matrix);

	shader.pushFloat("scale", scale);
	shader.pushInt("uPalette", palette_id);
	shader.pushVec3("size", vec3(0, 0, 0)); // SM flag not a voxagon
	shader.pushTexture2D("uColor", paletteBank, 1); // Texture 0 is SM

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.pushMatrix("position", pos);
	shader.pushMatrix("rotation", rot);

	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	shader.pushMatrix("world_pos", world_pos);
	shader.pushMatrix("world_rot", world_rot);

	// Use GL_LINES for wireframe
	vao.bind();
	glDrawElementsInstanced(GL_TRIANGLES, sizeof(hex_prism_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0, voxel_count);
	vao.unbind();
}
