#ifndef GREEDY_MESH_H
#define GREEDY_MESH_H

#include <string>
#include <vector>
#include <stdint.h>
#include <glm/glm.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_interface.h"

class GreedyRender : public VoxRender {
private:
	GLsizei index_count = 0;
public:
	GreedyRender(const MV_Shape& shape, int palette_id);
	void draw(Shader& shader, Camera& camera) override;
};

#endif
