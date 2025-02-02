#ifndef HEX_RENDER_H
#define HEX_RENDER_H

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_interface.h"

#include <glm/glm.hpp>

using namespace glm;

class HexRender : public VoxRender {
private:
	GLsizei voxel_count = 0;
public:
	HexRender(const MV_Shape& shape, int palette_id);
	void draw(Shader& shader, Camera& camera) override;
};

#endif
