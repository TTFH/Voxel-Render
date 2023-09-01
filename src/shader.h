#ifndef SHADER_H
#define SHADER_H

#include "../glad/glad.h"
#include <string>

using namespace std;

string ReadFile(const char* filename);

class Shader {
private:
	const char* VERSION = "#version 410 core\n";
	const char* VERTEX = "#define VERTEX\n";
	const char* FRAGMENT = "#define FRAGMENT\n";

	bool unified;
	const char* path1;
	const char* path2;
	void Load();
	void Create(const char* vertexSource, const char* fragmentSource);
public:
	GLuint id;
	Shader(const char* name);
	Shader(const char* vertexPath, const char* fragmentPath);
	void Reload();
	~Shader();
	void Use();
};

#endif
