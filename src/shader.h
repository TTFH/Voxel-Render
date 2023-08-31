#ifndef SHADER_H
#define SHADER_H

#include "../glad/glad.h"
#include <string>

using namespace std;

string ReadFile(const char* filename);

class Shader {
private:
	void CheckCompileErrors(unsigned int shader, const char* type, const char* filename = "");
public:
	GLuint id;
	Shader(const char* name);
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();
	void Use();
};

#endif
