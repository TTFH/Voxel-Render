#include <string.h>
#include <fstream>
#include <iostream>
#include "shader.h"

string ReadFile(const char* filename) {
	ifstream in(filename, ifstream::binary);
	if (!in) {
		printf("Error: Could not open file %s\n", filename);
		exit(EXIT_FAILURE);
	}
	string content;
	in.seekg(0, in.end);
	content.resize(in.tellg());
	in.seekg(0, in.beg);
	in.read(&content[0], content.size());
	in.close();
	return content;
}

void Shader::Create(const char* vertexSource, const char* fragmentSource) {
	GLint hasCompiled;
	char infoLog[1024];

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &hasCompiled);
	glGetShaderInfoLog(vertexShader, 1024, NULL, infoLog);
	if (hasCompiled == GL_FALSE)
		printf("[ERROR] Vertex shader %s failed to compile\n%s\n", path1, infoLog);
	else if (strcmp(infoLog, "") != 0)
		printf("[Warning] Vertex shader %s compiled with warnings\n%s\n", path1, infoLog);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &hasCompiled);
	glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
	if (hasCompiled == GL_FALSE)
		printf("[ERROR] Fragment shader %s failed to compile\n%s\n", path1, infoLog);
	else if (strcmp(infoLog, "") != 0)
		printf("[Warning] Fragment shader %s compiled with warnings\n%s\n", path1, infoLog);

	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &hasCompiled);
	if (hasCompiled == GL_FALSE) {
		//glGetProgramInfoLog(id, 1024, NULL, infoLog);
		printf("[ERROR] Program linking failed\n");
		exit(EXIT_FAILURE); // TODO: Handle error
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::Shader(const char* name) {
	string path = "shaders/" + string(name) + ".glsl";
	unified = true;
	path1 = path.c_str();
	path2 = path.c_str();
	Load();
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	unified = false;
	path1 = vertexPath;
	path2 = fragmentPath;
	Load();
}

void Shader::Load() {
	if (unified) {
		string shaderCode = ReadFile(path1);
		string vertexCode = string(VERSION) + VERTEX + shaderCode;
		string fragmentCode = string(VERSION) + FRAGMENT + shaderCode;
		Create(vertexCode.c_str(), fragmentCode.c_str());
	} else {
		string vertexCode = ReadFile(path1);
		string fragmentCode = ReadFile(path2);
		Create(vertexCode.c_str(), fragmentCode.c_str());
	}
}

void Shader::Reload() {
	GLuint old_id = id;
	Load();
	glDeleteProgram(old_id);
}

void Shader::Use() {
	glUseProgram(id);
}

Shader::~Shader() {
	glDeleteProgram(id);
}
