#ifndef SHADER_H
#define SHADER_H

#include "../glad/glad.h"
#include <glm/glm.hpp>
#include <string>
#include <map>

using namespace std;
using namespace glm;

string ReadFile(const char* filename);

class Shader {
private:
	const char* VERSION = "#version 410 core\n";
	const char* VERTEX = "#define VERTEX\n";
	const char* FRAGMENT = "#define FRAGMENT\n";
	map<const char*, GLint> uniforms;

	bool unified;
	string path1;
	string path2;
	void Load();
	void Create(const char* vertexSource, const char* fragmentSource);
	GLint GetSetLocation(const char* uniform);
public:
	GLuint id;
	Shader(const char* name);
	Shader(const char* vertexPath, const char* fragmentPath);
	void PushInt(const char* uniform, int value);
	void PushFloat(const char* uniform, float value);
	void PushVec2(const char* uniform, vec2 value);
	void PushVec3(const char* uniform, vec3 value);
	void PushVec4(const char* uniform, vec4 value);
	void PushMatrix(const char* uniform, mat4 value);
	void PushTexture1D(const char* uniform, GLuint texture_id, GLuint unit);
	void PushTexture(const char* uniform, GLuint texture_id, GLuint unit);
	void PushTexture3D(const char* uniform, GLuint texture_id, GLuint unit);
	void PushTextureCubeMap(const char* uniform, GLuint texture_id, GLuint unit);
	void Use();
	void Reload();
	~Shader();
};

#endif
