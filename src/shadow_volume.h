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
	void addShape(const MV_Shape& shape, vec3 position);
	void updateTexture();
	void draw(Shader& shader, Camera& camera);
	~ShadowVolume();
};
/*
class ShadowRender {
private:
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
public:
	ShadowRender(const MV_Shape& shape);
	void setTransform(vec3 position, quat rotation);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera, vec4 clip_plane, float scale = 1);
};
*/
#endif
