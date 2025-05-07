#ifndef LIGHTING_RTX_H
#define LIGHTING_RTX_H

#include "vao.h"
#include "camera.h"
#include "shader.h"

class SimpleScreen {
private:
	VAO vao;
	GLuint framebuffer;
	GLuint texture;
	vec2 position;
	vec2 size;
	void initFrameBuffer(int width, int height);
public:
	SimpleScreen(vec2 position, vec2 size, bool use_framebuffer);
	void start();
	void end();
	void setTexture(GLuint texture);
	void draw(Shader& shader);
};

class Screen {
private:
	VAO vao;
	GLuint sampler;
	GLuint framebuffer;

	GLuint color_texture;
	GLuint normal_texture;
	GLuint material_texture;
	GLuint motion_texture;
	GLuint depth_texture;
	void initFrameBuffer(int width, int height);
public:
	Screen();
	void start();
	void end();
	void draw(Shader& shader, Camera& camera);
	void pushUniforms(Shader& shader);
};

#endif
