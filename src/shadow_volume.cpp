#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ebo.h"
#include "utils.h"
#include "shadow_volume.h"

static GLfloat screen_vertices[] = {
	// pos  uv
	-1, -1, 0, 0,
	 1, -1, 1, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,
};

static GLuint screen_indices[] = {
	0, 1, 2,
	1, 3, 2,
};

ShadowVolume::ShadowVolume() {
	vao.Bind();
	VBO vbo(screen_vertices, sizeof(screen_vertices));
	EBO ebo(screen_indices, sizeof(screen_indices));
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.LinkAttrib(vbo, 1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void ShadowVolume::addShape(const MV_Shape& shape) {
	int width = shape.sizex / 2;// + shape.sizex % 2;
	int height = shape.sizez / 2;// + shape.sizez % 2;
	int depth = shape.sizey / 2;// + shape.sizey % 2;
	volumeSize = vec3(width, height, depth);

	int volume = width * height * depth;
	uint8_t* shadowVolume = new uint8_t[volume];
	memset(shadowVolume, 0, volume);

	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		int x = shape.voxels[i].x;
		int y = shape.voxels[i].z;
		int z = shape.sizey - shape.voxels[i].y;

		// Up to 8 voxels share the same index
		int index = (x / 2) + width * ((y / 2) + height * (z / 2));
		if (index < volume)
			shadowVolume[index] += 1 << ((x % 2) + 2 * (y % 2) + 4 * (z % 2));
	}

	glGenTextures(1, &volumeTexture);
	glBindTexture(GL_TEXTURE_3D, volumeTexture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 2);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, width, height, depth, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadowVolume);

	glBindTexture(GL_TEXTURE_3D, 0);
	delete[] shadowVolume;
}

void ShadowVolume::draw(Shader& shader, Camera& camera) {
	vao.Bind();

	shader.PushFloat("uVolTexelSize", 0.2);
	shader.PushVec3("uCameraPos", camera.position);
	shader.PushVec3("uVolOffset", vec3(0, 0, 0));
	shader.PushVec3("uVolResolution", volumeSize);
	shader.PushMatrix("uVpInvMatrix", inverse(camera.vpMatrix));
	shader.PushTexture3D("uVolTex", volumeTexture, 0);

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushMatrix("uVpMatrix", camera.vpMatrix);

	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
