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
	GLuint texture_id;
	GLsizei voxel_count = 0;
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
public:
	HexRender(const MV_Shape& shape, GLuint texture_id);
	void setTransform(vec3 position, quat rotation) override;
	void setWorldTransform(vec3 position, quat rotation) override;
	void draw(Shader& shader, Camera& camera, float scale, vec4 clip_plane) override;
};

#endif
