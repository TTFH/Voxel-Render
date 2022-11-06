#ifndef VOXEL_RENDER_H
#define VOXEL_RENDER_H

#include "vao.h"
#include "ebo.h"
#include "camera.h"
#include "shader.h"

class VoxelRender {
private:
	VAO vao;
	GLuint texture_id;
	GLsizei voxel_count = 0;
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);

	//   6--------7
	//  /|       /|
	// 2--------3 |
	// | |      | |
	// | 4------|-5
	// |/       |/
	// 0--------1
	static constexpr GLfloat cube_vertices[] = {
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

	static constexpr GLuint cube_indices[] = {
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
public:
	VoxelRender(vector<MV_Voxel> voxels, GLuint texture_id);
	void setTransform(vec3 position, quat rotation);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera, float scale = 1);
};

#endif
