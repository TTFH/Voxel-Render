#ifndef RENDER_INTERFACE_H
#define RENDER_INTERFACE_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "camera.h"
#include "shader.h"

class IRender {
protected:
	float scale = 1;
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
	vec4 texture = vec4(0, 0, 1, 1);
public:
	void setTransform(vec3 position, quat rotation) {
		this->position = position;
		this->rotation = rotation;
	}
	void setWorldTransform(vec3 position, quat rotation) {
		this->world_position = position;
		this->world_rotation = rotation;
	}
	void setScale(float scale) {
		this->scale = scale;
	}
	void setTexture(vec4 texture) {
		this->texture = texture;
	}
	virtual void draw(Shader& shader, Camera& camera) = 0;
	virtual ~IRender() {}
};

#endif
