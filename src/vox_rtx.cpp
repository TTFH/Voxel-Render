#include <stdint.h>
#include <string.h>

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

bool init = false;
GLuint albedoMap;
GLuint blendMap;
GLuint normalMap;
GLuint windowAlbedo;
GLuint windowNormal;
GLuint blueNoise;

RTX_Render::RTX_Render(const MV_Shape& shape, GLuint paletteBank, int paletteId) {
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	int width_mip0 = RoundTo_nth_Power(shape.sizex, 2);
	int height_mip0 = RoundTo_nth_Power(shape.sizey, 2);
	int depth_mip0 = RoundTo_nth_Power(shape.sizez, 2);
	int volume_mip0 = width_mip0 * height_mip0 * depth_mip0;
	uint8_t* voxels_mip0 = new uint8_t[volume_mip0];
	memset(voxels_mip0, 0, volume_mip0);

	for (unsigned int v = 0; v < shape.voxels.size(); v++) {
		int x = shape.voxels[v].x;
		int y = shape.voxels[v].y;
		int z = shape.voxels[v].z;
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

	this->paletteId = paletteId;
	this->paletteBank = paletteBank;
	shapeSize = vec3(width_mip0, height_mip0, depth_mip0);

	if (!init) {
		init = true;
		albedoMap = LoadTexture("textures/albedo.png", GL_RGB);
		blendMap = LoadTexture("textures/blend.png", GL_RGBA);
		normalMap = LoadTexture("textures/normal.png", GL_RGBA);
		windowAlbedo = LoadTexture("textures/window.png", GL_RGBA);
		windowNormal = LoadTexture("textures/window_normal.png", GL_RGBA);
		blueNoise = LoadTexture("textures/bluenoise512rgb.png", GL_RGBA);
	}
}

// Simple shader, from Teardown editor.
void RTX_Render::DrawSimple(Shader& shader, Camera& camera) {
	vao.Bind();

	shader.PushTexture("uColor", paletteBank, 0);
	shader.PushTexture3D("uVolTex", volumeTexture, 1);

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	glUniform1ui(glGetUniformLocation(shader.id, "uMaxValue"), 254);
	shader.PushInt("uPalette", paletteId);
	shader.PushVec4("uMultColor", vec4(1, 1, 1, 1));
	shader.PushFloat("uVolTexelSize", 0.1f * scale);
	shader.PushVec3("uVolResolution", shapeSize);
	shader.PushVec3("uCameraPos", camera.position);

	mat4 scaleBox = glm::scale(mat4(1.0f), 0.1f * scale * shapeSize);
	mat4 toWorldCoords = mat4(vec4(1, 0, 0, 0),
							  vec4(0, 0, -1, 0),
							  vec4(0, 1, 0, 0),
							  vec4(0, 0, 0, 1));

	// Coordinate system: x right, z up, y forward, scale 10 voxels : 1 meter
	mat4 pos = translate(mat4(1.0f), 0.1f * position);
	mat4 rot = mat4_cast(rotation);
	mat4 localTr = toWorldCoords * pos * rot;

	// Coordinate system: x right, y up, -z forward, scale 1:1 (in meters)
	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	mat4 worldTr = world_pos * world_rot;

	mat4 volMatrix = worldTr * localTr;
	mat4 modelMatrix = volMatrix * scaleBox;
	mat4 vpMatrix = camera.vpMatrix;
	mat4 mvpMatrix = vpMatrix * modelMatrix;
	mat4 volMatrixInv = inverse(volMatrix);

	shader.PushMatrix("uModelMatrix", modelMatrix);
	shader.PushMatrix("uVpMatrix", vpMatrix);
	shader.PushMatrix("uMvpMatrix", mvpMatrix);
	shader.PushMatrix("uVolMatrix", volMatrix);
	shader.PushMatrix("uVolMatrixInv", volMatrixInv);

	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

void RTX_Render::DrawAdvanced(Shader& shader, Camera& camera) {
	vec4 texture = vec4(0, 0, 1, 1); // TODO: load texture
	vao.Bind();

	shader.PushTexture3D("uVolTex", volumeTexture, 0);
	shader.PushTexture("uColor", paletteBank, 1);
	//shader.PushTexture("uMaterial", materialBank, 2);
	shader.PushTexture("uAlbedoMap", albedoMap, 3);
	shader.PushTexture("uBlendMap", blendMap, 4);
	shader.PushTexture("uNormalMap", normalMap, 5);
	shader.PushTexture("uWindowAlbedo", windowAlbedo, 6);
	shader.PushTexture("uWindowNormal", windowNormal, 7);
	shader.PushTexture("uBlueNoise", blueNoise, 8);

	shader.PushInt("uPalette", paletteId);
	shader.PushVec3("uObjSize", shapeSize);
	shader.PushVec4("uVoxelSize", vec4(shapeSize, 0.1f * scale));
	shader.PushFloat("uEmissive", 1.0f);
	shader.PushVec4("uTextureTile", texture);
	shader.PushVec3("uTextureParams", vec3(0, 0, 0));
	shader.PushFloat("uAlpha", 1.0f);
	shader.PushFloat("uHighlight", 0.0f);
	glUniform1ui(glGetUniformLocation(shader.id, "uEmissiveGlassCount"), 0);
	float emissiveGlass[12] = { 0 };
	glUniform1fv(glGetUniformLocation(shader.id, "uEmissiveGlass"), 12, emissiveGlass);

	shader.PushFloat("uRndFrame", 0.0f);
	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushVec3("uCameraPos", camera.position);
	shader.PushVec2("uPixelSize", vec2(0.0007, 0.00123));

	mat4 toWorldCoords = mat4(vec4(1, 0, 0, 0),
							  vec4(0, 0, -1, 0),
							  vec4(0, 1, 0, 0),
							  vec4(0, 0, 0, 1));

	// Coordinate system: x right, z up, y forward, scale 10 voxels : 1 meter
	mat4 pos = translate(mat4(1.0f), 0.1f * position);
	mat4 rot = mat4_cast(rotation);
	mat4 localTr = toWorldCoords * pos * rot;

	// Coordinate system: x right, y up, -z forward, scale 1:1 (in meters)
	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	mat4 worldTr = world_pos * world_rot;

	mat4 vpMatrix = camera.vpMatrix;
	mat4 volMatrix = worldTr * localTr;
	mat4 volMatrixInv = inverse(volMatrix);

	shader.PushMatrix("uVpMatrix", vpMatrix);
	shader.PushMatrix("uVolMatrix", volMatrix);
	shader.PushMatrix("uStableVpMatrix", vpMatrix);
	shader.PushMatrix("uVpInvMatrix", volMatrixInv);

	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

void RTX_Render::draw(Shader& shader, Camera& camera) {
	DrawAdvanced(shader, camera);
}

RTX_Render::~RTX_Render() {
	glDeleteTextures(1, &volumeTexture);
}
