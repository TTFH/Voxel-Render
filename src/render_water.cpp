#include <stdio.h>

#include "utils.h"
#include "render_water.h"

#include <glm/gtc/matrix_transform.hpp>

WaterRender::WaterRender(vector<vec2> vertices) {
	vertex_count = vertices.size();
	vao.Bind();
	VBO<vec2> vbo(vertices);
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, sizeof(vec2), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();
}

void WaterRender::setWorldTransform(vec3 position) {
	this->position = position;
}

void WaterRender::draw(Shader& shader, Camera& camera) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);

	mat4 pos = translate(mat4(1.0f), position);
	shader.PushMatrix("position", pos);

	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
