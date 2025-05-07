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
	static GLuint albedo_map;
	static GLuint blend_map;
	static GLuint normal_map;
	static GLuint window_albedo;
	static GLuint window_normal;

	vec3 matrix_size;
	GLuint volume_texture;
	vec4 texture = vec4(0, 0, 1, 1);
	void drawSimple(Shader& shader, Camera& camera);
	void drawAdvanced(Shader& shader, Camera& camera);
public:
	static GLuint bluenoise;
	static GLuint foam_texture;
	static void initTextures();

	RTX_Render(const MV_Shape& shape, int palette_id);
	void draw(Shader& shader, Camera& camera) override;
	void setTexture(vec4 texture);
	~RTX_Render();
};

#endif
