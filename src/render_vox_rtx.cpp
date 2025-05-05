#include <stdint.h>
#include <string.h>

#include "ebo.h"
#include "vbo.h"
#include "utils.h"
#include "render_vox_rtx.h"

static const GLfloat cube_vertices[] = {
	0, 0, 0,
	0, 0, 1,
	0, 1, 0,
	0, 1, 1,
	1, 0, 0,
	1, 0, 1,
	1, 1, 0,
	1, 1, 1,
};

static const GLuint cube_indices[] = {
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

bool init = false;
GLuint albedoMap;
GLuint blendMap;
GLuint normalMap;
GLuint windowAlbedo;
GLuint windowNormal;
GLuint blueNoise;

RTX_Render::RTX_Render(const MV_Shape& shape, int palette_id) {
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	int width_mip0 = CeilExp2(shape.sizex, 2);
	int height_mip0 = CeilExp2(shape.sizey, 2);
	int depth_mip0 = CeilExp2(shape.sizez, 2);
	int volume_mip0 = width_mip0 * height_mip0 * depth_mip0;
	uint8_t* voxels_mip0 = new uint8_t[volume_mip0];
	memset(voxels_mip0, 0, volume_mip0);

	// Center shape on extended volume
	for (unsigned int v = 0; v < shape.voxels.size(); v++) {
		int x = shape.voxels[v].x; // + (width_mip0 - shape.sizex) / 2;
		int y = shape.voxels[v].y; // + (height_mip0 - shape.sizey) / 2;
		int z = shape.voxels[v].z; // + (depth_mip0 - shape.sizez) / 2;
		int i = shape.voxels[v].index;
		int index = x + width_mip0 * (y + height_mip0 * z);
		voxels_mip0[index] = i != 255 ? i : 0;
	}

	int width_mip1 = width_mip0 / 2;
	int height_mip1 = height_mip0 / 2;
	int depth_mip1 = depth_mip0 / 2;
	int volume_mip1 = width_mip1 * height_mip1 * depth_mip1;
	uint8_t* voxels_mip1 = new uint8_t[volume_mip1];
	memset(voxels_mip1, 0, volume_mip1);

	for (int x = 0; x < width_mip0; x++) {
		for (int y = 0; y <height_mip0; y++) {
			for (int z = 0; z < depth_mip0; z++) {
				int index = x + width_mip0 * (y + height_mip0 * z);
				int index_mip1 = (x / 2) + width_mip1 * ((y / 2) + height_mip1 * (z / 2));
				if (index_mip1 < volume_mip1 && voxels_mip1[index_mip1] == 0)
					voxels_mip1[index_mip1] = voxels_mip0[index];
			}
		}
	}

	int width_mip2 = width_mip1 / 2;
	int height_mip2 = height_mip1 / 2;
	int depth_mip2 = depth_mip1 / 2;
	int volume_mip2 = width_mip2 * height_mip2 * depth_mip2;
	uint8_t* voxels_mip2 = new uint8_t[volume_mip2];
	memset(voxels_mip2, 0, volume_mip2);

	for (int x = 0; x < width_mip1; x++) {
		for (int y = 0; y < height_mip1; y++) {
			for (int z = 0; z < depth_mip1; z++) {
				int index = x + width_mip1 * (y + height_mip1 * z);
				int index_mip2 = (x / 2) + width_mip2 * ((y / 2) + height_mip2 * (z / 2));
				if (index_mip2 < volume_mip2 && voxels_mip2[index_mip2] == 0)
					voxels_mip2[index_mip2] = voxels_mip1[index];
			}
		}
	}

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, width_mip0, height_mip0, depth_mip0, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels_mip0);
	glTexImage3D(GL_TEXTURE_3D, 1, GL_R8UI, width_mip1, height_mip1, depth_mip1, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels_mip1);
	glTexImage3D(GL_TEXTURE_3D, 2, GL_R8UI, width_mip2, height_mip2, depth_mip2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels_mip2);

	glBindTexture(GL_TEXTURE_3D, 0);
	delete[] voxels_mip0;
	delete[] voxels_mip1;
	delete[] voxels_mip2;

	this->palette_id = palette_id;
	shape_size = vec3(shape.sizex, shape.sizey, shape.sizez);
	matrix_size = vec3(width_mip0, height_mip0, depth_mip0);

	if (!init) {
		init = true;
		albedoMap = LoadTexture2D("textures/albedo.png");
		blendMap = LoadTexture2D("textures/blend.png");
		normalMap = LoadTexture2D("textures/normal.png");
		windowAlbedo = LoadTexture2D("textures/window.png");
		windowNormal = LoadTexture2D("textures/window_normal.png");
		blueNoise = LoadTexture2D("textures/bluenoise512rgb.png");
	}
}

// Simple shader, from the Teardown editor
void RTX_Render::DrawSimple(Shader& shader, Camera& camera) {
	shader.PushTexture3D("uVolTex", volumeTexture, 0);
	shader.PushTexture2D("uColor", paletteBank, 1);

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushUInt("uMaxValue", 254);
	shader.PushInt("uPalette", palette_id);
	shader.PushVec4("uMultColor", vec4(1, 1, 1, 1));
	shader.PushFloat("uVolTexelSize", 0.1f * scale);
	shader.PushVec3("uVolResolution", matrix_size);
	shader.PushVec3("uCameraPos", camera.position);

	mat4 scaleBox = glm::scale(mat4(1.0f), 0.1f * scale * matrix_size);
	mat4 modelMatrix = volume_matrix * scaleBox;
	mat4 vpMatrix = camera.vpMatrix;
	mat4 mvpMatrix = vpMatrix * modelMatrix;
	mat4 volMatrixInv = inverse(volume_matrix);

	shader.PushMatrix("uModelMatrix", modelMatrix);
	shader.PushMatrix("uVpMatrix", vpMatrix);
	shader.PushMatrix("uMvpMatrix", mvpMatrix);
	shader.PushMatrix("uVolMatrix", volume_matrix);
	shader.PushMatrix("uVolMatrixInv", volMatrixInv);

	vao.Bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

void RTX_Render::DrawAdvanced(Shader& shader, Camera& camera) {
	shader.PushTexture3D("uVolTex", volumeTexture, 0);
	shader.PushTexture2D("uColor", paletteBank, 1);
	shader.PushTexture2D("uMaterial", materialBank, 2);
	shader.PushTexture2D("uAlbedoMap", albedoMap, 3);
	shader.PushTexture2D("uBlendMap", blendMap, 4);
	shader.PushTexture2D("uNormalMap", normalMap, 5);
	shader.PushTexture2D("uWindowAlbedo", windowAlbedo, 6);
	shader.PushTexture2D("uWindowNormal", windowNormal, 7);
	shader.PushTexture2D("uBlueNoise", blueNoise, 8);

	shader.PushInt("uPalette", palette_id);
	shader.PushVec3("uObjSize", shape_size);
	shader.PushVec4("uVoxelSize", vec4(matrix_size, 0.1f * scale));
	shader.PushVec4("uTextureTile", texture);
	shader.PushVec3("uTextureParams", vec3(0, 0, 0));
	shader.PushFloat("uAlpha", 1.0f);
	shader.PushFloat("uHighlight", 0.0f);

	shader.PushFloat("uRndFrame", 0.0f);
	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushFloat("uInvFar", 1.0f / camera.FAR_PLANE);
	shader.PushVec3("uCameraPos", camera.position);
	shader.PushVec2("uPixelSize", vec2(1.0f / camera.screen_width, 1.0f / camera.screen_height));

	mat4 vpMatrix = camera.vpMatrix;
	mat4 volMatrixInv = inverse(volume_matrix);

	shader.PushMatrix("uVpMatrix", vpMatrix);
	shader.PushMatrix("uVolMatrix", volume_matrix);
	shader.PushMatrix("uVpInvMatrix", volMatrixInv);

	vao.Bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

void RTX_Render::draw(Shader& shader, Camera& camera) {
	if (camera.isInFrustum(obb_corners))
		DrawAdvanced(shader, camera);
}

void RTX_Render::setTexture(vec4 texture) {
	this->texture = texture;
}

RTX_Render::~RTX_Render() {
	glDeleteTextures(1, &volumeTexture);
}
