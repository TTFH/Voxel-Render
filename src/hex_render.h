#ifndef HEX_RENDER_H
#define HEX_RENDER_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_interface.h"

using namespace glm;

class HexRender : public IRender {
private:
	VAO vao;
	int paletteId;
	GLuint paletteBank;
	GLsizei voxel_count = 0;
public:
	HexRender(const MV_Shape& shape, GLuint paletteBank, int paletteId);
	void draw(Shader& shader, Camera& camera) override;
};

#endif
