#include "ebo.h"
#include "utils.h"
#include "lighting_rtx.h"

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

	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, depthTexture, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawBuffers);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Screen::Screen() {
	vao.Bind();
	VBO vbo(screen_vertices, sizeof(screen_vertices));
	EBO ebo(screen_indices, sizeof(screen_indices));
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.LinkAttrib(vbo, 1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	InitFrameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
	bluenoise = LoadTexture("textures/bluenoise512rgb.png", GL_RGB);
}

void Screen::start() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::end() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Screen::draw(Shader& shader, Camera& camera) {
	vao.Bind();

	shader.PushFloat("uNear", camera.NEAR_PLANE);
	shader.PushFloat("uFar", camera.FAR_PLANE);
	shader.PushVec2("uPixelSize", vec2(0.0007, 0.00123));
	shader.PushVec3("uLightDir", vec3(0.38, -0.76, 0.53));
	shader.PushVec3("uCameraPos", camera.position);

	shader.PushMatrix("uVpMatrix", camera.vpMatrix);
	shader.PushMatrix("uVpInvMatrix", inverse(camera.vpMatrix));

	shader.PushTexture("uTexture", colorTexture, 0);
	shader.PushTexture("uNormal", normalTexture, 1);
	shader.PushTexture("uDepth", depthTexture, 2);
	shader.PushTexture("uBlueNoise", bluenoise, 3);

	glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
