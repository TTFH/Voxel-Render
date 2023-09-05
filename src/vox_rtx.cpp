#include "ebo.h"
#include "utils.h"
#include "vox_rtx.h"
#include <glm/glm.hpp>
#include <stdio.h>

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

RTX_Render::RTX_Render(const MV_Shape& shape, GLuint paletteBank, int paletteId) {
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	this->paletteId = paletteId;
	this->paletteBank = paletteBank;
	shapeSize = vec3(shape.sizex, shape.sizey, shape.sizez);
	printf("Palette ID: %d\n", paletteId);
	printf("Size: %d %d %d\n", shape.sizex, shape.sizey, shape.sizez);

	int volume = shape.sizex * shape.sizey * shape.sizez;
	uint8_t* voxels = new uint8_t[volume];
	for (int i = 0; i < volume; i++)
		voxels[i] = 0;
	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		int x = shape.voxels[i].x;
		int y = shape.voxels[i].y;
		int z = shape.voxels[i].z;
		voxels[x + shape.sizex * (y + shape.sizey * z)] = shape.voxels[i].index;
	}
	printf("Palette ID %d loaded!\n\n", paletteId);

	glGenTextures(1, &volumeTexture);
	glBindTexture(GL_TEXTURE_3D, volumeTexture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, shape.sizex, shape.sizey, shape.sizez, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);
	delete[] voxels;
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
	glDisable(GL_CLIP_DISTANCE0);
	(void)clip_plane;
	(void)scale;

	vao.Bind();

	shader.PushTexture3D("uVolTex", volumeTexture, 0);
	shader.PushTexture("uColor", paletteBank, 1);

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	glUniform1ui(glGetUniformLocation(shader.id, "uMaxValue"), 255);
	shader.PushInt("uPalette", paletteId);
	shader.PushVec4("uMultColor", vec4(1, 1, 1, 1));
	shader.PushFloat("uVolTexelSize", 0.1);
	shader.PushVec3("uVolResolution", shapeSize);
	shader.PushVec3("uCameraPos", camera.position);

	mat4 volMatrix = translate(mat4(1.0f), world_position);
	mat4 volMatrixInv = inverse(volMatrix);
	mat4 modelMatrix = volMatrix * glm::scale(mat4(1.0f), shapeSize * 0.1f);
	mat4 vpMatrix = camera.vpMatrix;
	mat4 mvpMatrix = camera.vpMatrix * modelMatrix;

	shader.PushMatrix("uModelMatrix", modelMatrix);
	shader.PushMatrix("uVpMatrix", vpMatrix);
	shader.PushMatrix("uMvpMatrix", mvpMatrix);
	shader.PushMatrix("uVolMatrix", volMatrix);
	shader.PushMatrix("uVolMatrixInv", volMatrixInv);

	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
