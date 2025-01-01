#include <glm/gtc/matrix_transform.hpp>
#include "render_rope.h"

RopeRender::RopeRender(vector<vec3> points, vec3 color) {
	this->color = color;
	points_count = points.size();
	vao.Bind();
	VBO<vec3> vbo(points);
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(vec3), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();
}

void RopeRender::setWorldTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void RopeRender::draw(Shader& shader, Camera& camera) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);
	shader.PushVec3("color", color);

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.PushMatrix("position", pos);
	shader.PushMatrix("rotation", rot);

	glDrawArrays(GL_LINE_STRIP, 0, points_count);
	vao.Unbind();
}
