#ifndef RENDER_INTERFACE_H
#define RENDER_INTERFACE_H

#include <glm/glm.hpp>

#include "camera.h"
#include "shader.h"

class IRender {
public:
	// The shader changes depending on the render method
	// Some methods may render to multiple shaders
	// This will not fucking work
	virtual void draw(Shader& shader, Camera& camera, float scale, vec4 texture_or_clipplane_or_something) = 0;
	virtual void setModelTransform(mat4 volMatrix) = 0;
	virtual ~IRender() {}
};

#endif
