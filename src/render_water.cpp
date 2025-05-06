#include <stdlib.h>

#include "vbo.h"
#include "utils.h"
#include "render_water.h"
#include "render_vox_rtx.h"

#include <glm/gtc/matrix_transform.hpp>

static float time = 0.0f;

WaterRender::WaterRender(vector<vec2> vertices) {
	/*vertex_count = vertices.size();
	VBO vbo(vertices);
	vao.linkAttrib(0, 2, GL_FLOAT, sizeof(vec2), (GLvoid*)0); // Vertex position
	vao.unbind();
	vbo.unbind();*/

	vector<vec3> vertices_bis;
	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices_bis.push_back(vec3(vertices[i].x, 0, vertices[i].y));
	vertex_count = vertices_bis.size();
	VBO vbo(vertices_bis);
	vao.linkAttrib(0, 3, GL_FLOAT, sizeof(vec3), (GLvoid*)0); // Vertex position
	vao.unbind();
	vbo.unbind();
}

void WaterRender::setWorldTransform(vec3 position) {
	this->position = position;
}

void WaterRender::draw(Shader& shader, Camera& camera) {
	/*shader.pushMatrix("camera", camera.vp_matrix);
	mat4 pos = translate(mat4(1.0f), position);
	shader.pushMatrix("position", pos);*/

	mat4 mvp_matrix = camera.vp_matrix * translate(mat4(1.0f), position);
	time += 1.0 / 60.0;

	shader.pushFloat("uFar", camera.FAR_PLANE);
	shader.pushFloat("uFoam", 0.5f);
	shader.pushFloat("uInvFar", 1.0f / camera.FAR_PLANE);
	shader.pushFloat("uMotion", 0.5f);
	shader.pushFloat("uRipple", 0.5f);
	shader.pushFloat("uRndFrame", 0.1f * rand() / (float)RAND_MAX);
	shader.pushFloat("uTime", time);
	shader.pushFloat("uVisibility", 3.6f);
	shader.pushFloat("uWave", 0.5f);
	shader.pushInt("uRingCount", 0);
	shader.pushMatrix("uMvpMatrix", mvp_matrix);
	shader.pushMatrix("uOldStableVpMatrix", camera.vp_matrix);
	shader.pushMatrix("uStableVpMatrix", camera.vp_matrix);
	shader.pushTexture2D("uFoamTexture", RTX_Render::foam_texture, 4);
	shader.pushVec2("uPixelSize", vec2(1.0f / camera.screen_width, 1.0f / camera.screen_height));
	shader.pushVec3("uCameraPos", camera.position);
	shader.pushVec4("uRings", vec4(0, 0, 0, 0));
	shader.pushVec4("uWaterColor", vec4(0.025f, 0.125f, 0.15f, 1.0f));

	vao.bind();
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.unbind();
}
