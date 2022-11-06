#include "rope_render.h"

RopeRender::RopeRender(vector<vec3> points, vec3 color) {
	this->color = color;
	points_count = points.size();
	vao.Bind();
	VBO vbo(points);
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(vec3), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();
}

void RopeRender::setWorldTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void RopeRender::draw(Shader& shader, Camera& camera) {
	shader.Use();
	vao.Bind();
	camera.pushMatrix(shader, "camera");

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "position"), 1, GL_FALSE, value_ptr(pos));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "rotation"), 1, GL_FALSE, value_ptr(rot));
	glUniform3f(glGetUniformLocation(shader.id, "color"), color.x, color.y, color.z);

	glDrawArrays(GL_LINE_STRIP, 0, points_count);
	vao.Unbind();
}
