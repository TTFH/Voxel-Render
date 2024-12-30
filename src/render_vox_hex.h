#ifndef HEX_RENDER_H
#define HEX_RENDER_H

#include <glm/glm.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_vox_interface.h"

using namespace glm;

class HexRender : public IRender {
private:
	VAO vao;
	int palette_id;
	GLuint palette_bank;
	GLsizei voxel_count = 0;
public:
	HexRender(const MV_Shape& shape, GLuint palette_id, int palette_bank);
	void draw(Shader& shader, Camera& camera) override;
};

#endif
