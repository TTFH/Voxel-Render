#include <stdio.h>

#include "utils.h"
#include "render_water.h"

#include <glm/gtc/quaternion.hpp>
/*
static bool water_initialized = false;
GLuint dudv_texture;
GLuint normal_texture;
static GLuint reflectionTexture;
static GLuint refractionTexture;

static void CreateFBwTexture(GLuint &FBO, GLuint &fbTexture, int width, int height) {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
}
*/
WaterRender::WaterRender(vector<vec2> vertices) {
	vertex_count = vertices.size();
	vao.Bind();
	VBO vbo(vertices);
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, sizeof(vec2), (GLvoid*)0); // Vertex position
	vao.Unbind();
	vbo.Unbind();
/*
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
		dudv_texture = LoadTexture2D("textures/water_dudv.png");
		normal_texture = LoadTexture2D("textures/water_normal.png");
		CreateFBwTexture(reflectionFrameBuffer, reflectionTexture, REFLECTION_WIDTH, REFLECTION_HEIGHT);
		CreateDepthBuffer(reflectionDepthBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
		CreateFBwTexture(refractionFrameBuffer, refractionTexture, REFRACTION_WIDTH, REFRACTION_HEIGHT);
		CreateDepthTexture(refractionDepthTexture, REFRACTION_WIDTH, REFRACTION_HEIGHT);
		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
			printf("[ERROR] Water framebuffer failed with status %d\n", fboStatus);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}*/
}
/*
float WaterRender::GetHeight() {
	return position.y;
}

void WaterRender::BindReflectionFB() {
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFrameBuffer);
	glViewport(0, 0, REFLECTION_WIDTH, REFLECTION_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WaterRender::BindRefractionFB() {
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFrameBuffer);
	glViewport(0, 0, REFRACTION_WIDTH, REFRACTION_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WaterRender::UnbindFB(Camera& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, camera.screen_width, camera.screen_height);
}
*/
void WaterRender::setWorldTransform(vec3 position) {
	this->position = position;
}

void WaterRender::draw(Shader& shader, Camera& camera) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);
	shader.PushVec3("camera_pos", camera.position);
	//shader.PushVec2("min", bounding_box.min);
	//shader.PushVec2("max", bounding_box.max);

	mat4 pos = translate(mat4(1.0f), position);
	shader.PushMatrix("position", pos);
/*
	shader.PushTexture2D("reflectionTexture", reflectionTexture, 0);
	shader.PushTexture2D("refractionTexture", refractionTexture, 1);
	shader.PushTexture2D("dudvMap", dudv_texture, 2);
	shader.PushTexture2D("normalMap", normal_texture, 3);
	shader.PushTexture2D("depthMap", refractionDepthTexture, 4);
*/
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
	vao.Unbind();
}
