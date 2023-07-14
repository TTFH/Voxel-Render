#ifndef WATER_RENDER_H
#define WATER_RENDER_H

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include <glm/gtc/quaternion.hpp>

using namespace glm;

class WaterRender {
private:
	VAO vao;
	GLsizei vertex_count;
	vec3 position = vec3(0, 0, 0);
public:
	GLuint reflectionFrameBuffer;
	GLuint reflectionTexture;
	GLuint reflectionDepthBuffer;
	
	GLuint refractionFrameBuffer;
	GLuint refractionTexture;
	GLuint refractionDepthTexture;

	WaterRender(vector<vec2> vertices);
	float GetHeight();
	void BindReflectionFB();
	void BindrefractionFB();
	void UnbindFB(Camera& camera);
	void setWorldTransform(vec3 position);
	void draw(Shader& shader, Camera& camera);
};

#endif
