#include "water_render.h"
#include <glm/gtc/type_ptr.hpp>

WaterRender::WaterRender(vector<vec2> vertices) {
	vertex_count = vertices.size();
	vao.Bind();
	VBO vbo(vertices);
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, sizeof(vec2), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();

	vec2 min = vertices[0];
	vec2 max = vertices[0];
	for (size_t i = 1; i < vertices.size(); i++) {
		vec2 vertex = vertices[i];
		if (vertex.x < min.x)
			min.x = vertex.x;
		if (vertex.y < min.y)
			min.y = vertex.y;
		if (vertex.x > max.x)
			max.x = vertex.x;
		if (vertex.y > max.y)
			max.y = vertex.y;
	}
	bounding_box = {min, max};
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

	glUniform2fv(glGetUniformLocation(shader.id, "min"), 1, value_ptr(bounding_box.min));
	glUniform2fv(glGetUniformLocation(shader.id, "max"), 1, value_ptr(bounding_box.max));

	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
