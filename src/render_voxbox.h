#ifndef VOXBOX_RENDER_H
#define VOXBOX_RENDER_H

#include "vao.h"
#include "camera.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

class VoxboxRender {
private:
	VAO vao;
	vec3 size = vec3(10, 10, 10);
	vec3 color = vec3(1, 1, 1);
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
public:
	VoxboxRender(vec3 size, vec3 color);
	void setColor(vec3 color);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera);
};

#endif
