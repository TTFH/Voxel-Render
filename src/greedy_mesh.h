#ifndef GREEDY_MESH_H
#define GREEDY_MESH_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"

struct GreedyMesh {
	vector<GM_Vertex> vertices;
	vector<GLuint> indices;
};

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
	FastRender(const MV_Shape& shape, GLuint texture_id);
	void setTransform(vec3 position, quat rotation);
	void setWorldTransform(vec3 position, quat rotation);
	void draw(Shader& shader, Camera& camera, vec4 clip_plane, float scale = 1);
};

#endif
