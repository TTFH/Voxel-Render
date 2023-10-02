#ifndef RENDER_INTERFACE_H
#define RENDER_INTERFACE_H

#include <glm/glm.hpp>

#include "camera.h"
#include "shader.h"

class IRender {
public:
	virtual void setTransform(vec3 position, quat rotation) = 0;
	virtual void setWorldTransform(vec3 position, quat rotation) = 0;
	virtual void draw(Shader& shader, Camera& camera, float scale, vec4 texture_or_clipplane) = 0;
	virtual ~IRender() {}
};

#endif
