#ifndef VBO_H
#define VBO_H

#include <vector>

#include <glm/glm.hpp>
#include "../glad/glad.h"

using namespace std;
using namespace glm;

class VBO {
private:
	GLuint vbo;
public:
	template <typename T>
	VBO(const vector<T>& vertices) {
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_STATIC_DRAW);
	}
	VBO(const GLfloat* vertices, GLsizeiptr size);
	~VBO();
	void bind();
	void unbind();
};

#endif
