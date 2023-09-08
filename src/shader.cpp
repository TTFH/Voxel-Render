#include <string.h>
#include <fstream>
#include <iostream>
#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

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
		printf("[ERROR] Vertex shader %s failed to compile\n%s\n", path1.c_str(), infoLog);
	else if (strcmp(infoLog, "") != 0)
		printf("[Warning] Vertex shader %s compiled with warnings\n%s\n", path1.c_str(), infoLog);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &hasCompiled);
	glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
	if (hasCompiled == GL_FALSE)
		printf("[ERROR] Fragment shader %s failed to compile\n%s\n", path2.c_str(), infoLog);
	else if (strcmp(infoLog, "") != 0)
		printf("[Warning] Fragment shader %s compiled with warnings\n%s\n", path2.c_str(), infoLog);

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
	path1 = path;
	path2 = path;
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
		string shaderCode = ReadFile(path1.c_str());
		string vertexCode = string(VERSION) + VERTEX + shaderCode;
		string fragmentCode = string(VERSION) + FRAGMENT + shaderCode;
		Create(vertexCode.c_str(), fragmentCode.c_str());
	} else {
		string vertexCode = ReadFile(path1.c_str());
		string fragmentCode = ReadFile(path2.c_str());
		Create(vertexCode.c_str(), fragmentCode.c_str());
	}
}

void Shader::PushInt(const char* uniform, int value) {
	this->Use();
	glUniform1i(glGetUniformLocation(id, uniform), value);
}

void Shader::PushFloat(const char* uniform, float value) {
	this->Use();
	glUniform1f(glGetUniformLocation(id, uniform), value);
}

void Shader::PushVec2(const char* uniform, vec2 value) {
	this->Use();
	glUniform2fv(glGetUniformLocation(id, uniform), 1, value_ptr(value));
}

void Shader::PushVec3(const char* uniform, vec3 value) {
	this->Use();
	glUniform3fv(glGetUniformLocation(id, uniform), 1, value_ptr(value));
}

void Shader::PushVec4(const char* uniform, vec4 value) {
	this->Use();
	glUniform4fv(glGetUniformLocation(id, uniform), 1, value_ptr(value));
}

void Shader::PushMatrix(const char* uniform, mat4 value) {
	this->Use();
	glUniformMatrix4fv(glGetUniformLocation(id, uniform), 1, GL_FALSE, value_ptr(value));
}

void Shader::PushTexture1D(const char* uniform, GLuint texture_id, GLuint unit) {
	this->Use();
	glUniform1i(glGetUniformLocation(id, uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_1D, texture_id);
}

void Shader::PushTexture(const char* uniform, GLuint texture_id, GLuint unit) {
	this->Use();
	glUniform1i(glGetUniformLocation(id, uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

void Shader::PushTexture3D(const char* uniform, GLuint texture_id, GLuint unit) {
	this->Use();
	glUniform1i(glGetUniformLocation(id, uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_3D, texture_id);
}

void Shader::PushTextureCubeMap(const char* uniform, GLuint texture_id, GLuint unit) {
	this->Use();
	glUniform1i(glGetUniformLocation(id, uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
}

void Shader::Use() {
	glUseProgram(id);
}

void Shader::Reload() {
	GLuint old_id = id;
	Load();
	glDeleteProgram(old_id);
}

Shader::~Shader() {
	glDeleteProgram(id);
}
