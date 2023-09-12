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
	ShadowVolume();
	// TODO: quat rotation, float scale
	void addShape(const MV_Shape& shape, vec3 position); // TODO: rename draw to match interface
	void draw(Shader& shader, Camera& camera); // TODO: actual_draw()
	~ShadowVolume();
};

#endif
