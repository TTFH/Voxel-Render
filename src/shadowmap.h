#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "camera.h"
#include "shader.h"

#include <glm/glm.hpp>
#include "../glad/glad.h"

class ShadowMap {
private:
	GLuint shadowMap;
	GLuint shadowMapFBO;
	const unsigned int shadowMapWidth = 4096;
	const unsigned int shadowMapHeight = 4096;
public:
	ShadowMap();
	void BindShadowMap();
	void UnbindShadowMap(Camera& camera);
	GLuint GetTexture();
	~ShadowMap();
};

#endif
