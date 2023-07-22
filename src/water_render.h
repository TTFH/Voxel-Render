#ifndef WATER_RENDER_H
#define WATER_RENDER_H

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include <glm/gtc/quaternion.hpp>

using namespace glm;

struct AABB_2D {
	vec2 min;
	vec2 max;
};

class WaterRender {
private:
	VAO vao;
	AABB_2D bounding_box;
	GLsizei vertex_count;
	vec3 position = vec3(0, 0, 0);

	GLuint reflectionFrameBuffer;
	//GLuint reflectionTexture;
	GLuint reflectionDepthBuffer;

	GLuint refractionFrameBuffer;
	//GLuint refractionTexture;
	GLuint refractionDepthTexture;

	GLuint dudv_texture;
	GLuint normal_texture;

	const int REFLECTION_WIDTH = 640;
	const int REFLECTION_HEIGHT = 360;

	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;
public:
	GLuint reflectionTexture;
	GLuint refractionTexture;

	WaterRender(vector<vec2> vertices);
	float GetHeight();
	void BindReflectionFB();
	void BindRefractionFB();
	void UnbindFB(Camera& camera);
	void setWorldTransform(vec3 position);
	void draw(Shader& shader, Camera& camera);
};

#endif
