#ifndef SHADOW_VOLUME_H
#define SHADOW_VOLUME_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"

class ShadowVolume {
private:
	VAO vao;
	int width, height, depth, volume;
	GLuint volumeTexture;
	uint8_t* shadowVolume;
public:
	ShadowVolume(int width_m, int height_m, int depth_m);
	void addShape(const MV_Shape& shape, mat4 modelMatrix);
	void updateTexture();
	void draw(Shader& shader, Camera& camera);
	~ShadowVolume();
};

#endif
