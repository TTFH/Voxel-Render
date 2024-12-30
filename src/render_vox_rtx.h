#ifndef VOX_RTX_H
#define VOX_RTX_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "render_vox_interface.h"

class RTX_Render : public IRender {
private:
	VAO vao;
	int paletteId;
	GLuint paletteBank;
	GLuint volumeTexture;
	vec3 shapeSize;
	void DrawSimple(Shader& shader, Camera& camera);
	void DrawAdvanced(Shader& shader, Camera& camera);
public:
	RTX_Render(const MV_Shape& shape, GLuint paletteBank, int paletteId);
	void draw(Shader& shader, Camera& camera) override;
	~RTX_Render();
};

#endif
