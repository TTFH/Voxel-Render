#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <string>

#include "../glad/glad.h"
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Shader {
private:
	const char* VERSION = "#version 410 core\n";
	const char* VERTEX = "#define VERTEX\n";
	const char* FRAGMENT = "#define FRAGMENT\n";
	map<string, GLint> uniforms;

	GLuint id;
	bool unified;
	string path1;
	string path2;
	void load();
	void create(const char* vertex_source, const char* fragment_source);
	GLint getLocation(const char* uniform);
public:
	Shader(const char* name);
	Shader(const char* vertex_path, const char* fragment_path);
	void pushInt(const char* uniform, int value);
	void pushUInt(const char* uniform, unsigned int value);
	void pushFloat(const char* uniform, float value);
	void pushVec2(const char* uniform, vec2 value);
	void pushVec3(const char* uniform, vec3 value);
	void pushVec4(const char* uniform, vec4 value);
	void pushMatrix(const char* uniform, mat4 value);
	void pushTexture1D(const char* uniform, GLuint texture_id, GLuint unit);
	void pushTexture2D(const char* uniform, GLuint texture_id, GLuint unit);
	void pushTexture3D(const char* uniform, GLuint texture_id, GLuint unit);
	void pushTextureCubeMap(const char* uniform, GLuint texture_id, GLuint unit);
	void use();
	void reload();
	~Shader();
};

#endif
