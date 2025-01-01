#ifndef VBO_H
#define VBO_H

#include <vector>

#include <glm/glm.hpp>
#include "../glad/glad.h"

using namespace std;
using namespace glm;

template <typename T>
class VBO {
private:
	GLuint vbo;
public:
	VBO(vector<T>& vertices);
	VBO(const GLfloat* vertices, GLsizeiptr size);
	~VBO();
	void Bind();
	void Unbind();
};

#endif
