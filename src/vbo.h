#ifndef VBO_H
#define VBO_H

#include <vector>

#include "../glad/glad.h"
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

struct MV_Voxel {
	uint8_t x, y, z, index;
};

struct Vertex {
	vec3 position;
	vec3 normal;
	uint8_t index;
};

class VBO {
private:
	GLuint vbo;
public:
	VBO(vector<vec2>& vertices);
	VBO(vector<vec3>& vertices);
	VBO(vector<Vertex>& vertices);
	VBO(vector<MV_Voxel>& vertices);
	VBO(const GLfloat* vertices, GLsizeiptr size);
	~VBO();
	void Bind();
	void Unbind();
};

#endif
