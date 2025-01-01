#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "camera.h"
#include "shader.h"

#include <glm/glm.hpp>
#include "../glad/glad.h"

class ShadowMap {
private:
	GLuint shadow_map_texture;
	GLuint shadow_map_fbo;
	const unsigned int WIDTH = 4096;
	const unsigned int HEIGHT = 4096;
public:
	ShadowMap();
	void BindShadowMap();
	void UnbindShadowMap(Camera& camera);
	GLuint GetTexture() const;
	~ShadowMap();
};

#endif
