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
	void Load();
	void Create(const char* vertex_source, const char* fragment_source);
	GLint GetLocation(const char* uniform);
public:
	Shader(const char* name);
	Shader(const char* vertex_path, const char* fragment_path);
	void PushInt(const char* uniform, int value);
	void PushUInt(const char* uniform, unsigned int value);
	void PushFloat(const char* uniform, float value);
	void PushVec2(const char* uniform, vec2 value);
	void PushVec3(const char* uniform, vec3 value);
	void PushVec4(const char* uniform, vec4 value);
	void PushMatrix(const char* uniform, mat4 value);
	void PushTexture1D(const char* uniform, GLuint texture_id, GLuint unit);
	void PushTexture2D(const char* uniform, GLuint texture_id, GLuint unit);
	void PushTexture3D(const char* uniform, GLuint texture_id, GLuint unit);
	void PushTextureCubeMap(const char* uniform, GLuint texture_id, GLuint unit);
	void Use();
	void Reload();
	~Shader();
};

#endif
