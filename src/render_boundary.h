#ifndef BOUNDARY_RENDER_H
#define BOUNDARY_RENDER_H

#include <vector>

#include "vao.h"
#include "camera.h"
#include "shader.h"

using namespace glm;

class BoundaryRender {
private:
	VAO vao;
	GLsizei index_count;
	GLuint boundary_texture;
public:
	BoundaryRender(vector<vec2> vertices);
	void draw(Shader& shader, Camera& camera);
};

#endif
