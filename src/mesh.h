#ifndef MESH_H
#define MESH_H

#include <vector>

#include "vao.h"
#include "shader.h"
#include "camera.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

using namespace std;
using namespace glm;

class Mesh {
private:
	VAO vao;
	vector<MeshVertex> vertices;
	GLuint diffuse_texture;
	GLuint specular_texture = 0;
	void LoadOBJ(const char* path);
	void SaveOBJ(const char* path);
public:
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	Mesh(const char* path, const char* diffuse_path, const char* specular_path = NULL);
	void handleInputs(GLFWwindow* window);
	void setWorldTransform(vec3 position, float angle = 0);
	void draw(Shader& shader, Camera& camera);
};

#endif
