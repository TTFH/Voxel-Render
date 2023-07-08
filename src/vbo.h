#ifndef VBO_H
#define VBO_H

#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include "../glad/glad.h"

using namespace std;
using namespace glm;

struct MV_Voxel {
	uint8_t x, y, z, index;
};

struct GM_Vertex {
	vec3 position;
	vec3 normal;
	uint8_t index;
};

struct MeshVertex {
	vec3 position;
	vec3 normal;
	vec2 tex_coord;
};

class VBO {
private:
	GLuint vbo;
public:
	VBO(vector<vec2>& vertices); // Used for water rendering
	VBO(vector<vec3>& vertices); // Used for ropes rendering
	VBO(vector<GM_Vertex>& vertices); // Used for greedy voxel rendering
	VBO(vector<MV_Voxel>& vertices); // Used for slow voxel rendering
	VBO(vector<MeshVertex>& vertices); // Used for mesh rendering
	VBO(const GLfloat* vertices, GLsizeiptr size); // Used for cube rendering
	~VBO();
	void Bind();
	void Unbind();
};

#endif
