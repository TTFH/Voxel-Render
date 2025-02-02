#ifndef ROPE_RENDER_H
#define ROPE_RENDER_H

#include <vector>

#include "vao.h"
#include "camera.h"
#include "shader.h"

#include <glm/gtc/quaternion.hpp>

class RopeRender {
private:
	VAO vao;
	GLsizei points_count;
	vec3 color = vec3(1, 1, 1);
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
public:
	RopeRender(vector<vec3> points, vec3 color);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera);
};

#endif
