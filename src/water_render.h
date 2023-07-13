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
/*
const int REFLECTION_WIDTH = 320;
const int REFLECTION_HEIGHT = 180;
const int REFRACTION_WIDTH = 1280;
const int REFRACTION_HEIGHT = 720;
*/
class WaterRender {
private:
	VAO vao;
	GLsizei vertex_count;
	vec3 position = vec3(0, 0, 0);
	AABB_2D bounding_box;
public:
	WaterRender(vector<vec2> vertices);
	void setWorldTransform(vec3 position);
	void draw(Shader& shader, Camera& camera);
};

#endif
