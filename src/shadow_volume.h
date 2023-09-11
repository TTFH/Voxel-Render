#ifndef SHADOW_VOLUME_H
#define SHADOW_VOLUME_H

#include "vao.h"
#include "camera.h"
#include "shader.h"

class ShadowVolume {
private:
	VAO vao;
	GLuint volumeTexture;
	vec3 volumeSize;
public:
	ShadowVolume();
	void addShape(const MV_Shape& shape); // TODO: position, rotation, scale
	void draw(Shader& shader, Camera& camera);
};

#endif
