#ifndef GREEDY_MESH_H
#define GREEDY_MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_vox_interface.h"

struct GreedyMesh {
	vector<GM_Vertex> vertices;
	vector<GLuint> indices;
};

class GreedyRender : public IRender {
private:
	VAO vao;
	int palette_id;
	GLuint palette_bank;
	GLsizei index_count = 0;
public:
	GreedyRender(const MV_Shape& shape, GLuint palette_bank, int palette_id);
	void draw(Shader& shader, Camera& camera) override;
};

#endif
