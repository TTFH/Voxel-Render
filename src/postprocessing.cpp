#include "ebo.h"
#include "vbo.h"
#include "utils.h"
#include "postprocessing.h"
#include "render_vox_rtx.h"

static const GLfloat screen_vertices[] = {
	// pos  uv
	-1, -1, 0, 0,
	 1, -1, 1, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,
};

static const GLuint screen_indices[] = {
	0, 1, 2,
	1, 3, 2,
};

SimpleScreen::SimpleScreen(vec2 position, vec2 size, bool use_framebuffer) {
	this->position = position;
	this->size = size;

	VBO vbo(screen_vertices, sizeof(screen_vertices));
	EBO ebo(screen_indices, sizeof(screen_indices));
	vao.linkAttrib(0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.linkAttrib(1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.unbind();
	vbo.unbind();
	ebo.unbind();

	if (use_framebuffer) initFrameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void SimpleScreen::initFrameBuffer(int width, int height) {
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SimpleScreen::start() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SimpleScreen::end() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SimpleScreen::setTexture(GLuint texture) {
	this->texture = texture;
}

void SimpleScreen::draw(Shader& shader) {
	shader.pushVec2("uPosition", position);
	shader.pushVec2("uSize", size);
	shader.pushTexture2D("uTexture", texture, 0);

	vao.bind();
	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.unbind();
}

// ----------------------------------------------------------------------------

void Screen::initFrameBuffer(int width, int height) {
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTextureParameteri(color_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(color_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(color_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(color_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);

	glGenTextures(1, &normal_texture);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glTextureParameteri(normal_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(normal_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(normal_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(normal_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture, 0);
	
	glGenTextures(1, &material_texture);
	glBindTexture(GL_TEXTURE_2D, material_texture);
	glTextureParameteri(material_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(material_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(material_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(material_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, material_texture, 0);

	glGenTextures(1, &motion_texture);
	glBindTexture(GL_TEXTURE_2D, motion_texture);
	glTextureParameteri(motion_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(motion_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(motion_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(motion_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, motion_texture, 0);

	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTextureParameteri(depth_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depth_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depth_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depth_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, depth_texture, 0);

	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, draw_buffers);

	GLuint depth_buffer;
	glGenRenderbuffers(1, &depth_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Screen::Screen() {
	VBO vbo(screen_vertices, sizeof(screen_vertices));
	EBO ebo(screen_indices, sizeof(screen_indices));
	vao.linkAttrib(0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.linkAttrib(1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.unbind();
	vbo.unbind();
	ebo.unbind();

	initFrameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void Screen::start() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::end() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Screen::draw(Shader& shader, Camera& camera) {
	shader.pushFloat("uNear", camera.NEAR_PLANE);
	shader.pushFloat("uFar", camera.FAR_PLANE);
	shader.pushVec2("uPixelSize", vec2(1.0f / WINDOW_WIDTH, 1.0f / WINDOW_HEIGHT));
	shader.pushVec3("uLightDir", vec3(0.38f, -0.76f, 0.53f));
	shader.pushVec3("uCameraPos", camera.position);

	shader.pushMatrix("uVpMatrix", camera.vp_matrix);
	shader.pushMatrix("uVpInvMatrix", inverse(camera.vp_matrix));

	shader.pushTexture2D("uTexture", color_texture, 0);
	shader.pushTexture2D("uNormal", normal_texture, 1);
	shader.pushTexture2D("uDepth", depth_texture, 2);
	shader.pushTexture2D("uBlueNoise", RTX_Render::bluenoise, 3);

	vao.bind();
	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.unbind();
}

void Screen::pushUniforms(Shader& shader) {
	shader.pushTexture2D("uColor", color_texture, 1);
	shader.pushTexture2D("uDepth", depth_texture, 2);
	shader.pushTexture2D("uBlueNoise", RTX_Render::bluenoise, 3);
}
