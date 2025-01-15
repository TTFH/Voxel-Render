#include "ebo.h"
#include "vbo.h"
#include "utils.h"
#include "postprocessing.h"

static GLfloat screen_vertices[] = {
	// pos  uv
	-1, -1, 0, 0,
	 1, -1, 1, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,
};

static GLuint screen_indices[] = {
	0, 1, 2,
	1, 3, 2,
};

void Screen::InitFrameBuffer(int width, int height) {
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTextureParameteri(colorTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(colorTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTextureParameteri(normalTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(normalTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(normalTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(normalTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);
	
	glGenTextures(1, &materialTexture);
	glBindTexture(GL_TEXTURE_2D, materialTexture);
	glTextureParameteri(materialTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(materialTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(materialTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(materialTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, materialTexture, 0);

	glGenTextures(1, &motionTexture);
	glBindTexture(GL_TEXTURE_2D, motionTexture);
	glTextureParameteri(motionTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(motionTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(motionTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(motionTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, motionTexture, 0);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTextureParameteri(depthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, depthTexture, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, drawBuffers);

	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Screen::Screen() {
	VBO vbo(screen_vertices, sizeof(screen_vertices));
	EBO ebo(screen_indices, sizeof(screen_indices));
	vao.LinkAttrib(0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.LinkAttrib(1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	InitFrameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
	bluenoise = LoadTexture2D("textures/bluenoise512rgb.png");
}

void Screen::start() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::end() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Screen::draw(Shader& shader, Camera& camera) {
	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushVec2("uPixelSize", vec2(1.0f / WINDOW_WIDTH, 1.0f / WINDOW_HEIGHT));
	shader.PushVec3("uLightDir", vec3(0.38f, -0.76f, 0.53f));
	shader.PushVec3("uCameraPos", camera.position);

	shader.PushMatrix("uVpMatrix", camera.vpMatrix);
	shader.PushMatrix("uVpInvMatrix", inverse(camera.vpMatrix));

	shader.PushTexture2D("uTexture", colorTexture, 0);
	shader.PushTexture2D("uNormal", normalTexture, 1);
	shader.PushTexture2D("uDepth", depthTexture, 2);
	shader.PushTexture2D("uBlueNoise", bluenoise, 3);

	vao.Bind();
	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

void Screen::pushUniforms(Shader& shader) {
	shader.PushTexture2D("uColor", colorTexture, 1);
	shader.PushTexture2D("uDepth", depthTexture, 2);
	shader.PushTexture2D("uBlueNoise", bluenoise, 3);
}
