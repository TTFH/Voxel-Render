#include "water_render.h"
#include <glm/gtc/type_ptr.hpp>

WaterRender::WaterRender(vector<vec2> vertices) {
	vertex_count = vertices.size();
	vao.Bind();
	VBO vbo(vertices);
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, sizeof(vec2), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();
}

void WaterRender::setWorldTransform(vec3 position) {
	this->position = position;
}

void WaterRender::draw(Shader& shader, Camera& camera) {
	shader.Use();
	vao.Bind();
	camera.pushMatrix(shader, "camera");

	mat4 pos = translate(mat4(1.0f), position);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "position"), 1, GL_FALSE, value_ptr(pos));

	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
