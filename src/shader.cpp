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
	string contents;
	in.seekg(0, in.end);
	contents.resize(in.tellg());
	in.seekg(0, in.beg);
	in.read(&contents[0], contents.size());
	in.close();
	return contents;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	string vertexCode = ReadFile(vertexPath);
	const char* vertexSource = vertexCode.c_str();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX", vertexPath);

	string fragmentCode = ReadFile(fragmentPath);
	const char* fragmentSource = fragmentCode.c_str();
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT", fragmentPath);

	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	compileErrors(id, "PROGRAM", "");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Use() {
	glUseProgram(id);
}

Shader::~Shader() {
	//glDeleteProgram(id);
}

void Shader::compileErrors(unsigned int shader, const char* type, const char* filename) {
	GLint hasCompiled;
	char infoLog[1024];
	if (strcmp(type, "PROGRAM") != 0) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			printf("[ERROR] %s shader %s failed to compile\n%s\n", type, filename, infoLog);
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			printf("[ERROR] %s linking failed\n%s\n", type, infoLog);
			exit(EXIT_FAILURE);
		}
	}
}
