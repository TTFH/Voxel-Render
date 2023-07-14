#include <stdio.h>

#include "texture.h"
#include "water_render.h"

#include <glm/gtc/type_ptr.hpp>

const int REFLECTION_WIDTH = 320;
const int REFLECTION_HEIGHT = 180;
const int REFRACTION_WIDTH = 1280;
const int REFRACTION_HEIGHT = 720;

static void CreateFramebuffer(GLuint &FBO, GLuint &fbTexture, GLuint &depthBuffer, int width, int height) {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbTexture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
}

static void CreateFramebuffer2(GLuint &FBO, GLuint &fbTexture, GLuint &depthTexture, int width, int height) {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbTexture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
}

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

	CreateFramebuffer(reflectionFrameBuffer, reflectionTexture, reflectionDepthBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
	CreateFramebuffer2(refractionFrameBuffer, refractionTexture, refractionDepthTexture, REFRACTION_WIDTH, REFRACTION_HEIGHT);

	Texture dudvMap("water_dudv.png", "dudv", 2);
	dudv_texture = dudvMap.texture_id;

	Texture normalMap("water_normal.png", "normal", 3);
	normal_texture = normalMap.texture_id;

	printf("Water initialized.\n");
}

float WaterRender::GetHeight() {
	return position.y;
}

void WaterRender::BindReflectionFB() {
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFrameBuffer);
	glViewport(0, 0, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

void WaterRender::BindrefractionFB() {
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFrameBuffer);
	glViewport(0, 0, REFRACTION_WIDTH, REFRACTION_HEIGHT);
}

void WaterRender::UnbindFB(Camera& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, camera.screen_width, camera.screen_height);
}

void WaterRender::setWorldTransform(vec3 position) {
	this->position = position;
}

void WaterRender::draw(Shader& shader, Camera& camera) {
	shader.Use();
	vao.Bind();
	camera.pushMatrix(shader, "camera");
	glUniform3fv(glGetUniformLocation(shader.id, "camera_pos"), 1, value_ptr(camera.position));

	mat4 pos = translate(mat4(1.0f), position);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "position"), 1, GL_FALSE, value_ptr(pos));

	glUniform2fv(glGetUniformLocation(shader.id, "min"), 1, value_ptr(bounding_box.min));
	glUniform2fv(glGetUniformLocation(shader.id, "max"), 1, value_ptr(bounding_box.max));

	glUniform1i(glGetUniformLocation(shader.id, "reflectionTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reflectionTexture);

	glUniform1i(glGetUniformLocation(shader.id, "refractionTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, refractionTexture);

	glUniform1i(glGetUniformLocation(shader.id, "dudvMap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dudv_texture);

	glUniform1i(glGetUniformLocation(shader.id, "normalMap"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normal_texture);

	glUniform1i(glGetUniformLocation(shader.id, "depthMap"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, refractionDepthTexture);

	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
