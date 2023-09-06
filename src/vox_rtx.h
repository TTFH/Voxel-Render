#ifndef VOX_RTX_H
#define VOX_RTX_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"

class RTX_Render {
private:
	VAO vao;
	int paletteId;
	GLuint paletteBank;
	GLuint volumeTexture;
	vec3 shapeSize;
	uint8_t* voxels;

	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
public:
	RTX_Render(const MV_Shape& shape, GLuint paletteBank, int paletteId);
	void setTransform(vec3 position, quat rotation);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera, vec4 clip_plane, float scale = 1);
	~RTX_Render();
};

#endif
