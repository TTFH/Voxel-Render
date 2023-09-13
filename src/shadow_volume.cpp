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

	width = 200; height = 150; depth = 200;
	volume = width * height * depth;
	shadowVolume = new uint8_t[volume];
	memset(shadowVolume, 0, volume);

	glGenTextures(1, &volumeTexture);
	glBindTexture(GL_TEXTURE_3D, volumeTexture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 2);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, width, height, depth, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexImage3D(GL_TEXTURE_3D, 1, GL_R8UI, width / 2, height / 2, depth / 2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexImage3D(GL_TEXTURE_3D, 2, GL_R8UI, width / 4, height / 4, depth / 4, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_3D, 0);
}

void ShadowVolume::addShape(const MV_Shape& shape, vec3 position) {
	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		int x = shape.voxels[i].x + position.x * 10.0f;
		int y = shape.voxels[i].z + position.y * 10.0f;
		int z = shape.sizey - shape.voxels[i].y + position.z * 10.0f;

		if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth)
			continue;

		// Up to 8 voxels share the same index
		int index = (x / 2) + width * ((y / 2) + height * (z / 2));
		if (index < volume)
			shadowVolume[index] += 1 << ((x % 2) + 2 * (y % 2) + 4 * (z % 2));
	}
}

void ShadowVolume::updateTexture() {
	int width_mip1 = width / 2;
	int height_mip1 = height / 2;
	int depth_mip1 = depth / 2;
	int volume_mip1 = width_mip1 * height_mip1 * depth_mip1;
	uint8_t* shadowVolume_mip1 = new uint8_t[volume_mip1];
	memset(shadowVolume_mip1, 0, volume_mip1);

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < depth; k++) {
				int index = i + width * (j + height * k);
				int index_mip1 = (i / 2) + width_mip1 * ((j / 2) + height_mip1 * (k / 2));
				if (index_mip1 < volume_mip1)
					shadowVolume_mip1[index_mip1] |= shadowVolume[index];
			}
		}
	}

	int width_mip2 = width_mip1 / 2;
	int height_mip2 = height_mip1 / 2;
	int depth_mip2 = depth_mip1 / 2;
	int volume_mip2 = width_mip2 * height_mip2 * depth_mip2;
	uint8_t* shadowVolume_mip2 = new uint8_t[volume_mip2];
	memset(shadowVolume_mip2, 0, volume_mip2);

	for (int i = 0; i < width_mip1; i++) {
		for (int j = 0; j < height_mip1; j++) {
			for (int k = 0; k < depth_mip1; k++) {
				int index = i + width_mip1 * (j + height_mip1 * k);
				int index_mip2 = (i / 2) + width_mip2 * ((j / 2) + height_mip2 * (k / 2));
				if (index_mip2 < volume_mip2)
					shadowVolume_mip2[index_mip2] |= shadowVolume_mip1[index];
			}
		}
	}

	glBindTexture(GL_TEXTURE_3D, volumeTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadowVolume);
	glTexSubImage3D(GL_TEXTURE_3D, 1, 0, 0, 0, width_mip1, height_mip1, depth_mip1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadowVolume_mip1);
	glTexSubImage3D(GL_TEXTURE_3D, 2, 0, 0, 0, width_mip2, height_mip2, depth_mip2, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadowVolume_mip2);
	glBindTexture(GL_TEXTURE_3D, 0);

	delete[] shadowVolume_mip1;
	delete[] shadowVolume_mip2;
}

void ShadowVolume::draw(Shader& shader, Camera& camera) {
	vao.Bind();

	shader.PushFloat("uVolTexelSize", 0.2);
	shader.PushVec3("uCameraPos", camera.position);
	shader.PushVec3("uVolOffset", vec3(0, 0, 0));
	shader.PushVec3("uVolResolution", vec3(width, height, depth));
	shader.PushMatrix("uVpInvMatrix", inverse(camera.vpMatrix));
	shader.PushTexture3D("uVolTex", volumeTexture, 0);

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushMatrix("uVpMatrix", camera.vpMatrix);

	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

ShadowVolume::~ShadowVolume() {
	glDeleteTextures(1, &volumeTexture);
	delete[] shadowVolume;
}
