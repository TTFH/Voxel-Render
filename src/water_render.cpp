#include <stdio.h>

#include "utils.h"
#include "water_render.h"

#include <glm/gtc/type_ptr.hpp>

static bool water_initialized = false;
static GLuint reflectionTexture;
static GLuint refractionTexture;

static void CreateFBwTexture(GLuint &FBO, GLuint &fbTexture, int width, int height) {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbTexture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

static void CreateDepthBuffer(GLuint &depthBuffer, int width, int height) {
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
}

static void CreateDepthTexture(GLuint &depthTexture, int width, int height) {
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
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
	bounding_box = { min, max };

	if (!water_initialized) {
		water_initialized = true;
		CreateFBwTexture(reflectionFrameBuffer, reflectionTexture, REFLECTION_WIDTH, REFLECTION_HEIGHT);
		CreateDepthBuffer(reflectionDepthBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
		CreateFBwTexture(refractionFrameBuffer, refractionTexture, REFRACTION_WIDTH, REFRACTION_HEIGHT);
		CreateDepthTexture(refractionDepthTexture, REFRACTION_WIDTH, REFRACTION_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	dudv_texture = LoadTexture("water_dudv.png", GL_RGB);
	normal_texture = LoadTexture("water_normal.png", GL_RGB);
}

float WaterRender::GetHeight() {
	return position.y;
}

void WaterRender::BindReflectionFB() {
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFrameBuffer);
	glViewport(0, 0, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

void WaterRender::BindRefractionFB() {
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

	PushTexture(reflectionTexture, shader, "reflectionTexture", 0);
	PushTexture(refractionTexture, shader, "refractionTexture", 1);
	PushTexture(dudv_texture, shader, "dudvMap", 2);
	PushTexture(normal_texture, shader, "normalMap", 3);
	PushTexture(refractionDepthTexture, shader, "depthMap", 4);

	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
