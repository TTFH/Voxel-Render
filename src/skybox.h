#ifndef SKYBOX_H
#define SKYBOX_H

#include "shader.h"
#include "camera.h"

class Skybox {
private:
	GLuint skyboxVAO;
	GLuint cubemapTexture;
	float aspectRatio;

	static constexpr GLfloat skyboxVertices[] = {
		-1, -1,  1,
		 1, -1,  1,
		 1, -1, -1,
		-1, -1, -1,
		-1,  1,  1,
		 1,  1,  1,
		 1,  1, -1,
		-1,  1, -1,
	};

	static constexpr GLuint skyboxIndices[] = {
		0, 1, 5,
		0, 3, 2,
		0, 4, 7,
		1, 2, 6,
		2, 1, 0,
		3, 7, 6,
		4, 5, 6,
		5, 4, 0,
		6, 2, 3,
		6, 5, 1,
		6, 7, 4,
		7, 3, 0,
	};
public:
	Skybox(Shader& skyboxProgram, float aspectRatio);
	void Draw(Shader& skyboxProgram, Camera& camera);
};

#endif
