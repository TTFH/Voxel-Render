#ifndef VOX_RTX_H
#define VOX_RTX_H

#include "vao.h"
#include "camera.h"
#include "shader.h"
#include "vox_loader.h"
#include "render_interface.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class RTX_Render : public VoxRender {
private:
	vec3 shapeSize;
	vec3 matrixSize;
	GLuint volumeTexture;
	vec4 texture = vec4(0, 0, 1, 1);
	void DrawSimple(Shader& shader, Camera& camera);
	void DrawAdvanced(Shader& shader, Camera& camera);
public:
	RTX_Render(const MV_Shape& shape, int palette_id);
	void draw(Shader& shader, Camera& camera) override;
	~RTX_Render();
};

#endif
