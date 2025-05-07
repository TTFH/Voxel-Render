#include <stdexcept>
#include <string.h>

#include "utils.h"
#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

void Shader::create(const char* vertex_source, const char* fragment_source) {
	GLint has_compiled;
	char info_log[1024];

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(vertex_shader);
	
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &has_compiled);
	glGetShaderInfoLog(vertex_shader, 1024, NULL, info_log);
	if (has_compiled == GL_FALSE)
		printf("[ERROR] Vertex shader %s failed to compile\n%s\n", path1.c_str(), info_log);
	else if (strcmp(info_log, "") != 0)
		printf("[Warning] Vertex shader %s compiled with warnings\n%s\n", path1.c_str(), info_log);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &has_compiled);
	glGetShaderInfoLog(fragment_shader, 1024, NULL, info_log);
	if (has_compiled == GL_FALSE)
		printf("[ERROR] Fragment shader %s failed to compile\n%s\n", path2.c_str(), info_log);
	else if (strcmp(info_log, "") != 0)
		printf("[Warning] Fragment shader %s compiled with warnings\n%s\n", path2.c_str(), info_log);

	id = glCreateProgram();
	glAttachShader(id, vertex_shader);
	glAttachShader(id, fragment_shader);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &has_compiled);
	if (has_compiled == GL_FALSE) {
		//glGetProgramInfoLog(id, 1024, NULL, infoLog);
		throw runtime_error("Program linking failed");
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

Shader::Shader(const char* name) {
	string path = "shaders/" + string(name) + ".glsl";
	unified = true;
	path1 = path;
	path2 = path;
	load();
}

Shader::Shader(const char* vertex_path, const char* fragment_path) {
	unified = false;
	path1 = vertex_path;
	path2 = fragment_path;
	load();
}

void Shader::load() {
	if (unified) {
		string shader_code = ReadFile(path1.c_str());
		string vertex_code = string(VERSION) + VERTEX + shader_code;
		string fragment_code = string(VERSION) + FRAGMENT + shader_code;
		create(vertex_code.c_str(), fragment_code.c_str());
	} else {
		string vertex_code = ReadFile(path1.c_str());
		string fragment_code = ReadFile(path2.c_str());
		create(vertex_code.c_str(), fragment_code.c_str());
	}
}

GLint Shader::getLocation(const char* uniform) {
	if (uniforms.find(uniform) == uniforms.end()) {
		GLint location = glGetUniformLocation(id, uniform);
		if (location == -1)
			printf("[Warning] Uniform %s not found or used in %s / %s\n", uniform, path1.c_str(), path2.c_str());
		uniforms[uniform] = location;
	}
	return uniforms[uniform];
}

void Shader::pushInt(const char* uniform, int value) {
	glUniform1i(getLocation(uniform), value);
}

void Shader::pushUInt(const char* uniform, unsigned int value) {
	glUniform1ui(getLocation(uniform), value);
}

void Shader::pushFloat(const char* uniform, float value) {
	glUniform1f(getLocation(uniform), value);
}

void Shader::pushVec2(const char* uniform, vec2 value) {
	glUniform2fv(getLocation(uniform), 1, value_ptr(value));
}

void Shader::pushVec3(const char* uniform, vec3 value) {
	glUniform3fv(getLocation(uniform), 1, value_ptr(value));
}

void Shader::pushVec4(const char* uniform, vec4 value) {
	glUniform4fv(getLocation(uniform), 1, value_ptr(value));
}

void Shader::pushMatrix(const char* uniform, mat4 value) {
	glUniformMatrix4fv(getLocation(uniform), 1, GL_FALSE, value_ptr(value));
}

void Shader::pushTexture1D(const char* uniform, GLuint texture_id, GLuint unit) {
	glUniform1i(getLocation(uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_1D, texture_id);
}

void Shader::pushTexture2D(const char* uniform, GLuint texture_id, GLuint unit) {
	glUniform1i(getLocation(uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

void Shader::pushTexture3D(const char* uniform, GLuint texture_id, GLuint unit) {
	glUniform1i(getLocation(uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_3D, texture_id);
}

void Shader::pushTextureCubeMap(const char* uniform, GLuint texture_id, GLuint unit) {
	glUniform1i(getLocation(uniform), unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
}

void Shader::use() {
	glUseProgram(id);
}

void Shader::reload() {
	GLuint old_id = id;
	try {
		load();
		uniforms.clear();
	} catch (const runtime_error& e) {
		printf("[Warning] Shader %s failed to reload\n", path1.c_str());
		glDeleteProgram(id);
		id = old_id;
	}
}

Shader::~Shader() {
	glDeleteProgram(id);
}
