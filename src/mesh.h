#ifndef MESH_H
#define MESH_H

#include <vector>

#include "vao.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include <glm/gtc/quaternion.hpp>

using namespace std;
using namespace glm;

class Mesh {
private:
	VAO vao;
	vector<MeshVertex> vertices;
	vector<Texture> textures;
	void LoadOBJ(const char* path);
public:
	Mesh(const char* path, vector<Texture>& textures);
	void draw(Shader& shader, Camera& camera, vec3 translation = vec3(0.0f, 0.0f, 0.0f), float angle = 0);
};

#endif
