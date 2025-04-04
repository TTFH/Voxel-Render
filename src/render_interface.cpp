#include <stdio.h>

#include "utils.h"
#include "render_interface.h"

int VoxRender::paletteCount = 0;
GLuint VoxRender::paletteBank = 0;

mat4 VoxRender::getVolumeMatrix() {
	mat4 toWorldCoords = mat4(  vec4(1, 0, 0, 0),
								vec4(0, 0, -1, 0),
								vec4(0, 1, 0, 0),
								vec4(0, 0, 0, 1));

	// Coordinate system: x right, z up, y forward, scale 10 voxels : 1 meter
	mat4 pos = translate(mat4(1.0f), 0.1f * scale * position);
	mat4 rot = mat4_cast(rotation);
	mat4 localTr = toWorldCoords * pos * rot;

	// Coordinate system: x right, y up, -z forward, scale 1:1 (in meters)
	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	mat4 worldTr = world_pos * world_rot;

	return worldTr * localTr;
}

void VoxRender::setTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void VoxRender::setWorldTransform(vec3 position, quat rotation) {
	this->world_position = position;
	this->world_rotation = rotation;
}

void VoxRender::setScale(float scale) {
	this->scale = scale;
}

int VoxRender::getIndex(const MV_Diffuse* palette) {
	if (paletteCount == 0) { // Create texture
		glGenTextures(1, &paletteBank);
		glBindTexture(GL_TEXTURE_2D, paletteBank);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, MAX_PALETTES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	if (paletteCount < MAX_PALETTES) {
		glBindTexture(GL_TEXTURE_2D, paletteBank);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, paletteCount, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, palette);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (paletteCount == MAX_PALETTES)
		printf("[Warning] Palette limit reached!\n");
	paletteCount++;
	return paletteCount - 1;
}

void VoxRender::saveTexture() {
	SaveTexture("palette.png", paletteBank);
}
