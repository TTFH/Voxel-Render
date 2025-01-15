#include <stdlib.h>

#include "vbo.h"
#include "utils.h"
#include "render_water.h"

#include <glm/gtc/matrix_transform.hpp>

//GLuint foamTexture;
//float time = 0.0f;

WaterRender::WaterRender(vector<vec2> vertices) {
	vertex_count = vertices.size();
	VBO vbo(vertices);
	vao.LinkAttrib(0, 2, GL_FLOAT, sizeof(vec2), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();

	/*vector<vec3> vertices_bis;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices_bis.push_back(vec3(vertices[i].x, 0, vertices[i].y));
	}
	vertex_count = vertices_bis.size();
	VBO vbo(vertices_bis);
	vao.LinkAttrib(0, 3, GL_FLOAT, sizeof(vec3), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();

	foamTexture = LoadTexture2D("textures/foam.png");*/
}

void WaterRender::setWorldTransform(vec3 position) {
	this->position = position;
}

void WaterRender::draw(Shader& shader, Camera& camera) {
	shader.PushMatrix("camera", camera.vpMatrix);

	mat4 pos = translate(mat4(1.0f), position);
	shader.PushMatrix("position", pos);

	/*mat4 mvpMatrix = camera.vpMatrix * translate(mat4(1.0f), position);
	shader.PushMatrix("uMvpMatrix", mvpMatrix);
	shader.PushMatrix("uStableVpMatrix", camera.vpMatrix);
	shader.PushMatrix("uOldStableVpMatrix", camera.vpMatrix);
	time += 1.0 / 60.0;
	shader.PushFloat("uTime", time);
	shader.PushFloat("uWave", 0.5f);
	shader.PushFloat("uRipple", 0.5f);
	shader.PushFloat("uMotion", 0.5f);
	shader.PushFloat("uFoam", 0.5f);
	shader.PushTexture2D("uFoamTexture", foamTexture, 0);
	shader.PushVec4("uWaterColor", vec4(0.025f, 0.125f, 0.15f, 1.0f));
	shader.PushFloat("uVisibility", 3.6f);
	shader.PushInt("uRingCount", 0);

	shader.PushFloat("uRndFrame", 0.1f * rand() / (float)RAND_MAX);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushFloat("uInvFar", 1.0f / camera.FAR_PLANE);
	shader.PushVec3("uCameraPos", camera.position);
	shader.PushVec2("uPixelSize", vec2(1.0f / camera.screen_width, 1.0f / camera.screen_height));*/

	vao.Bind();
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
