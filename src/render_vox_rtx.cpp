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
GLuint albedo_map = 0;
GLuint blend_map = 0;
GLuint normal_map = 0;
GLuint window_albedo = 0;
GLuint window_normal = 0;
GLuint bluenoise = 0;

RTX_Render::RTX_Render(const MV_Shape& shape, int palette_id) {
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.linkAttrib(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.unbind();
	vbo.unbind();
	ebo.unbind();

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
		albedo_map = LoadTexture2D("textures/albedo.png");
		blend_map = LoadTexture2D("textures/blend.png");
		normal_map = LoadTexture2D("textures/normal.png");
		window_albedo = LoadTexture2D("textures/window.png");
		window_normal = LoadTexture2D("textures/window_normal.png");
		bluenoise = LoadTexture2D("textures/bluenoise512rgb.png");
	}
}

// Simple shader, from the Teardown editor
void RTX_Render::DrawSimple(Shader& shader, Camera& camera) {
	shader.pushTexture3D("uVolTex", volumeTexture, 0);
	shader.pushTexture2D("uColor", paletteBank, 1);

	shader.pushFloat("uNear", camera.NEAR_PLANE);
	shader.pushFloat("uFar", camera.FAR_PLANE);
	shader.pushUInt("uMaxValue", 254);
	shader.pushInt("uPalette", palette_id);
	shader.pushVec4("uMultColor", vec4(1, 1, 1, 1));
	shader.pushFloat("uVolTexelSize", 0.1f * scale);
	shader.pushVec3("uVolResolution", matrix_size);
	shader.pushVec3("uCameraPos", camera.position);

	mat4 scale_box = glm::scale(mat4(1.0f), 0.1f * scale * matrix_size);
	mat4 model_matrix = volume_matrix * scale_box;
	mat4 mvp_matrix = camera.vp_matrix * model_matrix;
	mat4 vol_matrix_inv = inverse(volume_matrix);

	shader.pushMatrix("uModelMatrix", model_matrix);
	shader.pushMatrix("uVpMatrix", camera.vp_matrix);
	shader.pushMatrix("uMvpMatrix", mvp_matrix);
	shader.pushMatrix("uVolMatrix", volume_matrix);
	shader.pushMatrix("uVolMatrixInv", vol_matrix_inv);

	vao.bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.unbind();
}

void RTX_Render::DrawAdvanced(Shader& shader, Camera& camera) {
	shader.pushTexture3D("uVolTex", volumeTexture, 0);
	shader.pushTexture2D("uColor", paletteBank, 1);
	shader.pushTexture2D("uMaterial", materialBank, 2);
	shader.pushTexture2D("uAlbedoMap", albedo_map, 3);
	shader.pushTexture2D("uBlendMap", blend_map, 4);
	shader.pushTexture2D("uNormalMap", normal_map, 5);
	shader.pushTexture2D("uWindowAlbedo", window_albedo, 6);
	shader.pushTexture2D("uWindowNormal", window_normal, 7);
	shader.pushTexture2D("uBlueNoise", bluenoise, 8);

	shader.pushInt("uPalette", palette_id);
	shader.pushVec3("uObjSize", shape_size);
	shader.pushVec4("uVoxelSize", vec4(matrix_size, 0.1f * scale));
	shader.pushVec4("uTextureTile", texture);
	shader.pushVec3("uTextureParams", vec3(0, 0, 0));
	shader.pushFloat("uAlpha", 1.0f);
	shader.pushFloat("uHighlight", 0.0f);

	shader.pushFloat("uRndFrame", 0.0f);
	shader.pushFloat("uNear", camera.NEAR_PLANE);
	shader.pushFloat("uFar", camera.FAR_PLANE);
	shader.pushFloat("uInvFar", 1.0f / camera.FAR_PLANE);
	shader.pushVec3("uCameraPos", camera.position);
	shader.pushVec2("uPixelSize", vec2(1.0f / camera.screen_width, 1.0f / camera.screen_height));

	mat4 vol_matrix_inv = inverse(volume_matrix);

	shader.pushMatrix("uVpMatrix", camera.vp_matrix);
	shader.pushMatrix("uVolMatrix", volume_matrix);
	shader.pushMatrix("uVpInvMatrix", vol_matrix_inv);

	vao.bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.unbind();
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
