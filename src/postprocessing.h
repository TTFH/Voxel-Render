#ifndef LIGHTING_RTX_H
#define LIGHTING_RTX_H

#include "vao.h"
#include "camera.h"
#include "shader.h"

class Screen {
private:
	VAO vao;
	GLuint bluenoise;
	GLuint framebuffer;
	GLuint colorTexture;
	GLuint normalTexture;
	GLuint depthTexture;
	void InitFrameBuffer(int width, int height);
public:
	Screen();
	void start();
	void end();
	void draw(Shader& shader, Camera& camera);
};

#endif
