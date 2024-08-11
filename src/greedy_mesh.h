#ifndef GREEDY_MESH_H
#define GREEDY_MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_interface.h"

struct GreedyMesh {
	vector<GM_Vertex> vertices;
	vector<GLuint> indices;
};

class GreedyRender : public IRender {
private:
	VAO vao;
	int paletteId;
	GLuint paletteBank;
	GLsizei index_count = 0;
public:
	GreedyRender(const MV_Shape& shape, GLuint paletteBank, int paletteId);
	void draw(Shader& shader, Camera& camera) override;
};

#endif
