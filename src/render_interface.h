#ifndef RENDER_INTERFACE_H
#define RENDER_INTERFACE_H

#include "vao.h"
#include "camera.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

struct MV_Diffuse;

class VoxRender {
protected:
	static int paletteCount;
	static GLuint paletteBank;
	static const int MAX_PALETTES = 1024;

	VAO vao;
	int palette_id;
	vec3 shape_size;

	float scale = 1;
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
public:
	mat4 volume_matrix = mat4(1.0);
	vector<vec3> obb_corners;

	void setTransform(vec3 position, quat rotation);
	void setWorldTransform(vec3 position, quat rotation);
	void setScale(float scale);
	void generateMatrixAndOBB();

	static int getIndex(const MV_Diffuse* palette);
	static void saveTexture();

	virtual void draw(Shader& shader, Camera& camera) = 0;
	virtual ~VoxRender() {}
};

#endif
