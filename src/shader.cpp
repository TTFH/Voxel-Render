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

static const char* VERSION = "#version 410 core\n";
static const char* VERTEX = "#define VERTEX\n";
static const char* FRAGMENT = "#define FRAGMENT\n";

Shader::Shader(const char* name) {
	string path = "shaders/" + string(name) + ".glsl";
	string shaderCode = ReadFile(path.c_str());

	string vertexCode = string(VERSION) + VERTEX + shaderCode;
	const char* vertexSource = vertexCode.c_str();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	CheckCompileErrors(vertexShader, "Vertex", name);

	string fragmentCode = string(VERSION) + FRAGMENT + shaderCode;
	const char* fragmentSource = fragmentCode.c_str();
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	CheckCompileErrors(fragmentShader, "Fragment", name);

	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	CheckCompileErrors(id, "Program", name);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	string vertexCode = ReadFile(vertexPath);
	const char* vertexSource = vertexCode.c_str();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	CheckCompileErrors(vertexShader, "Vertex", vertexPath);

	string fragmentCode = ReadFile(fragmentPath);
	const char* fragmentSource = fragmentCode.c_str();
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	CheckCompileErrors(fragmentShader, "Fragment", fragmentPath);

	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	CheckCompileErrors(id, "Program");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Use() {
	glUseProgram(id);
}

Shader::~Shader() {
	glDeleteProgram(id);
}

void Shader::CheckCompileErrors(unsigned int shader, const char* type, const char* filename) {
	GLint hasCompiled;
	char infoLog[1024];
	if (strcmp(type, "Program") != 0) {
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
