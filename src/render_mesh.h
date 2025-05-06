#ifndef MESH_H
#define MESH_H

#include <vector>

#include "vao.h"
#include "shader.h"
#include "camera.h"

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

using namespace std;
using namespace glm;

struct MeshVertex {
	vec3 position;
	vec3 normal;
	vec2 tex_coord;
};

class Mesh {
private:
	VAO vao;
	vector<MeshVertex> vertices;
	vector<GLuint> textures;
	vec3 color;
	void loadOBJ(const char* path);
	void saveOBJ(const char* path);
	void loadSimpleOBJ(const char* path); // No texture
public:
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	Mesh(const char* path);
	Mesh(const char* path, vec3 color);
	void addTexture(const char* path);
	void handleInputs(GLFWwindow* window);
	void setWorldTransform(vec3 position, float angle = 0);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera);
};

#endif
