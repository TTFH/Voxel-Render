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
	ShadowVolume(float width_m, float height_m, float depth_m);
	void addShape(const MV_Shape& shape, mat4 modelMatrix);
	void updateTexture();
	void draw(Shader& shader, Camera& camera);
	~ShadowVolume();
};

#endif
