#include <string.h>
#include <glm/glm.hpp>

#include "ebo.h"
#include "utils.h"
#include "vox_rtx.h"

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

	int volume = shape.sizex * shape.sizey * shape.sizez;
	voxels = new uint8_t[volume];
	memset(voxels, 0, volume);

	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		int x = shape.voxels[i].x;
		int y = shape.voxels[i].y;
		int z = shape.voxels[i].z;
		voxels[x + shape.sizex * (y + shape.sizey * z)] = shape.voxels[i].index;
	}

	glGenTextures(1, &volumeTexture);
	glBindTexture(GL_TEXTURE_3D, volumeTexture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, shape.sizex, shape.sizey, shape.sizez, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);
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
	(void)clip_plane;
	glDisable(GL_CLIP_DISTANCE0);
	vao.Bind();

	shader.PushTexture("uColor", paletteBank, 0);
	shader.PushTexture3D("uVolTex", volumeTexture, 1);

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	glUniform1ui(glGetUniformLocation(shader.id, "uMaxValue"), 255);
	shader.PushInt("uPalette", paletteId);
	shader.PushVec4("uMultColor", vec4(1, 1, 1, 1));
	shader.PushFloat("uVolTexelSize", 0.1);
	shader.PushVec3("uVolResolution", shapeSize);
	shader.PushVec3("uCameraPos", camera.position);

	mat4 scaleBox = glm::scale(mat4(1.0f), 0.1f * scale * shapeSize);
	mat4 toWorldCoords = mat4(vec4(1, 0, 0, 0),
							  vec4(0, 0, -1, 0),
							  vec4(0, 1, 0, 0),
							  vec4(0, 0, 0, 1));

	// Coordinate system: x right, z up, y forward, scale 10:1
	mat4 pos = translate(mat4(1.0f), position * 0.1f);
	mat4 rot = mat4_cast(rotation);
	mat4 localTr = toWorldCoords * pos * rot;

	// Coordinate system: x right, y up, -z forward, scale 1:1
	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	mat4 worldTr = world_pos * world_rot;

	mat4 modelMatrix = worldTr * localTr * scaleBox;
	mat4 vpMatrix = camera.vpMatrix;
	mat4 mvpMatrix = vpMatrix * modelMatrix;
	mat4 volMatrix = worldTr * localTr;
	mat4 volMatrixInv = inverse(volMatrix);

	shader.PushMatrix("uModelMatrix", modelMatrix);
	shader.PushMatrix("uVpMatrix", vpMatrix);
	shader.PushMatrix("uMvpMatrix", mvpMatrix);
	shader.PushMatrix("uVolMatrix", volMatrix);
	shader.PushMatrix("uVolMatrixInv", volMatrixInv);

	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

RTX_Render::~RTX_Render() {
	glDeleteTextures(1, &volumeTexture);
	delete[] voxels;
}
