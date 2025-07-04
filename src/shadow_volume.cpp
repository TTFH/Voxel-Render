#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ebo.h"
#include "vbo.h"
#include "utils.h"
#include "shadow_volume.h"
#include "render_vox_rtx.h"

static const GLfloat screen_vertices[] = {
	// pos  uv
	-1, -1, 0, 0,
	 1, -1, 1, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,
};

static const GLuint screen_indices[] = {
	0, 1, 2,
	1, 3, 2,
};

ShadowVolume::ShadowVolume(float width_m, float height_m, float depth_m) {
	VBO vbo(screen_vertices, sizeof(screen_vertices));
	EBO ebo(screen_indices, sizeof(screen_indices));
	vao.linkAttrib(0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.linkAttrib(1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.unbind();
	vbo.unbind();
	ebo.unbind();

	width = CeilExp2(10.0f * width_m, 2);
	height = CeilExp2(10.0f * height_m, 2);
	depth = CeilExp2(10.0f * depth_m, 2);

	int volume = width * height * depth;
	shadow_volume_mip0 = new uint8_t[volume];
	memset(shadow_volume_mip0, 0, volume);

	glGenTextures(1, &volume_texture);
	glBindTexture(GL_TEXTURE_3D, volume_texture);

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

#ifdef _BLENDER
	scene_root = scene_xml.NewElement("scene");
	scene_xml.InsertFirstChild(scene_root);
#endif
}

// Receives the global transform of a shape
void ShadowVolume::addShape(const MV_Shape& shape, mat4 model_matrix) {
	// Export scene to Blender
#ifdef _BLENDER
	vec3 position(model_matrix[3]);
	mat3 rot_matrix(model_matrix);
	quat quat = quat_cast(rot_matrix);
	vec3 rotation = degrees(eulerAngles(quat));

	XMLElement* mesh_element = scene_xml.NewElement("mesh");
	string path = shape.id + ".obj";
	string pos = to_string(position.x) + " " + to_string(position.y) + " " + to_string(position.z);
	string rot = to_string(rotation.x) + " " + to_string(rotation.y) + " " + to_string(rotation.z);
	mesh_element->SetAttribute("file", path.c_str());
	mesh_element->SetAttribute("pos", pos.c_str());
	mesh_element->SetAttribute("rot", rot.c_str());
	scene_root->InsertEndChild(mesh_element);
#endif

	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		int xv = shape.voxels[i].x;
		int yv = shape.voxels[i].y;
		int zv = shape.voxels[i].z;
		vec3 voxel_pos = vec3(xv, yv, zv) + vec3(0.5f, 0.5f, 0.5f); // offset to center of voxel
		voxel_pos = vec3(model_matrix * vec4(voxel_pos, 10.0f)); // convert from meters
		int x = floor(voxel_pos.x) + width / 2;
		int y = floor(voxel_pos.y);
		int z = floor(voxel_pos.z) + depth / 2;

		if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth)
			continue;

		// Up to 8 voxels share the same index
		int index = (x / 2) + width * ((y / 2) + height * (z / 2));
		shadow_volume_mip0[index] += 1 << ((x % 2) + 2 * (y % 2) + 4 * (z % 2));
	}
}

void ShadowVolume::updateTexture() {
#ifdef _BLENDER
	scene_xml.SaveFile("scene.xml");
	VoxRender::SaveTexture();
#endif

	int width_mip1 = width / 2;
	int height_mip1 = height / 2;
	int depth_mip1 = depth / 2;
	int volume_mip1 = width_mip1 * height_mip1 * depth_mip1;
	uint8_t* shadow_volume_mip1 = new uint8_t[volume_mip1];
	memset(shadow_volume_mip1, 0, volume_mip1);

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < depth; k++) {
				int index = i + width * (j + height * k);
				int index_mip1 = (i / 2) + width_mip1 * ((j / 2) + height_mip1 * (k / 2));
				if (index_mip1 < volume_mip1)
					shadow_volume_mip1[index_mip1] |= shadow_volume_mip0[index];
			}
		}
	}

	int width_mip2 = width_mip1 / 2;
	int height_mip2 = height_mip1 / 2;
	int depth_mip2 = depth_mip1 / 2;
	int volume_mip2 = width_mip2 * height_mip2 * depth_mip2;
	uint8_t* shadow_volume_mip2 = new uint8_t[volume_mip2];
	memset(shadow_volume_mip2, 0, volume_mip2);

	for (int i = 0; i < width_mip1; i++) {
		for (int j = 0; j < height_mip1; j++) {
			for (int k = 0; k < depth_mip1; k++) {
				int index = i + width_mip1 * (j + height_mip1 * k);
				int index_mip2 = (i / 2) + width_mip2 * ((j / 2) + height_mip2 * (k / 2));
				if (index_mip2 < volume_mip2)
					shadow_volume_mip2[index_mip2] |= shadow_volume_mip1[index];
			}
		}
	}

	glBindTexture(GL_TEXTURE_3D, volume_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadow_volume_mip0);
	glTexSubImage3D(GL_TEXTURE_3D, 1, 0, 0, 0, width_mip1, height_mip1, depth_mip1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadow_volume_mip1);
	glTexSubImage3D(GL_TEXTURE_3D, 2, 0, 0, 0, width_mip2, height_mip2, depth_mip2, GL_RED_INTEGER, GL_UNSIGNED_BYTE, shadow_volume_mip2);
	glBindTexture(GL_TEXTURE_3D, 0);

	delete[] shadow_volume_mip1;
	delete[] shadow_volume_mip2;
}

void ShadowVolume::draw(Shader& shader, Camera& camera) {
	shader.pushFloat("uFar", camera.FAR_PLANE);
	shader.pushFloat("uNear", camera.NEAR_PLANE);
	shader.pushFloat("uVolTexelSize", 0.2f);
	shader.pushMatrix("uVpInvMatrix", inverse(camera.vp_matrix));
	shader.pushMatrix("uVpMatrix", camera.vp_matrix);
	shader.pushTexture3D("uShadowVolume", volume_texture, 0);
	shader.pushVec3("uCameraPos", camera.position);
	shader.pushVec3("uVolOffset", vec3(-width / 20.0f, 0.0f, -depth / 20.0f));
	shader.pushVec3("uVolResolution", vec3(width, height, depth));

	shader.pushFloat("uRndFrame", RTX_Render::random_frame);
	shader.pushVec2("uPixelSize", vec2(1.0f / camera.screen_width, 1.0f / camera.screen_height));

	vao.bind();
	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.unbind();
}

ShadowVolume::~ShadowVolume() {
	glDeleteTextures(1, &volume_texture);
	delete[] shadow_volume_mip0;
}
