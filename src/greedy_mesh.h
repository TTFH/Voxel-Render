#ifndef GREEDY_MESH_H
#define GREEDY_MESH_H

#include <vector>
#include <stdint.h>
#include <glm/glm.hpp>

#include "vao.h"
#include "ebo.h"
#include "camera.h"
#include "shader.h"

struct MV_Shape {
	int sizex, sizey, sizez;
	vector<MV_Voxel> voxels;
};

struct GreedyMesh {
	vector<Vertex> vertices;
	vector<GLuint> indices;
};

GreedyMesh generateGreedyMesh(MV_Shape shape);

class FastRender {
private:
	VAO vao;
	GLuint texture_id;
	GLsizei index_count = 0;
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
public:
	FastRender(MV_Shape shape, GLuint texture_id);
	void setTransform(vec3 position, quat rotation);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera, float scale = 1);
};

#endif
