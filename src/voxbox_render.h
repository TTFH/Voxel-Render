#ifndef VOXBOX_RENDER_H
#define VOXBOX_RENDER_H

#include "vao.h"
#include "ebo.h"
#include "camera.h"
#include "shader.h"

class VoxboxRender {
private:
	VAO vao;
	vec3 size = vec3(10, 10, 10);
	vec3 color = vec3(1, 1, 1);
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);

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
	VoxboxRender(vec3 size, vec3 color);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera);
};

#endif
