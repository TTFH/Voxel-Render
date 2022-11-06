#ifndef EBO_H
#define EBO_H

#include <vector>
#include "../glad/glad.h"

using namespace std;

class EBO {
private:
	GLuint ebo;
public:
	EBO(vector<GLuint>& indices);
	EBO(const GLuint* indices, GLsizeiptr size);
	~EBO();
	void Bind();
	void Unbind();
};

#endif
