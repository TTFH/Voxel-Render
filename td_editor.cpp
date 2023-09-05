#include <map>
#include <list>
#include <stdio.h>
#include <string.h>

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/ebo.h"
#include "src/mesh.h"
#include "src/utils.h"
#include "src/shader.h"
#include "src/camera.h"
#include "src/vox_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

using namespace std;
using namespace glm;

static GLfloat vertices[] = {
	// pos  uv
	-1, -1, 0, 0,
	 1, -1, 1, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,
};

static GLuint indices[] = {
	0, 1, 2,
	1, 3, 2,
};

class Screen {
private:
	VAO vao;
public:
	Screen();
	void draw();
};

Screen::Screen() {
	vao.Bind();
	VBO vbo(vertices, sizeof(vertices));
	EBO ebo(indices, sizeof(indices));
	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
	vao.LinkAttrib(vbo, 1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Screen::draw() {
	vao.Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static GLfloat cube_vertices[] = {
	0, 0, 0,
	0, 0, 1,
	0, 1, 0,
	0, 1, 1,
	1, 0, 0,
	1, 0, 1,
	1, 1, 0,
	1, 1, 1,
};

static GLuint cube_indices[] = {
	2, 1, 0,
	6, 2, 0,
	1, 4, 0,
	4, 6, 0,
	2, 3, 1,
	5, 4, 1,
	7, 5, 1,
	3, 7, 1,
	6, 3, 2,
	6, 7, 3,
	5, 6, 4,
	7, 6, 5,
};

class Cube {
private:
	VAO vao;
public:
	Cube();
	void draw();
};

Cube::Cube() {
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Cube::draw() {
	vao.Bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("Totally not Teardown");
	Shader shape_shader("editorvox");
	Shader screen_shader("editorlighting");
	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");

	Cube cube;
	Screen screen;
	GLuint bluenoise = LoadTexture("textures/bluenoise.png", GL_RGB);
	vec3 lightDir(0.38, -0.76, 0.53);

	Camera camera;
	camera.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, vec3(0, 4.8, 8.8));

	Mesh train1("trains/Inyo.obj", "trains/Inyo.png");
	train1.setWorldTransform(vec3(0, 0, -10));

	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLuint colorTexture;
	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	GLuint normalTexture;
	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);

	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawBuffers);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	VoxLoader car("sportscar.vox");
	MV_Shape shape = car.shapes[0];
	int volume = shape.sizex * shape.sizey * shape.sizez;
	uint8_t* voxels = new uint8_t[volume];
	for (int i = 0; i < volume; i++)
		voxels[i] = 0;
	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		int x = shape.voxels[i].x;
		int y = shape.voxels[i].y;
		int z = shape.voxels[i].z;
		voxels[x + y * shape.sizex + z * shape.sizex * shape.sizey] = shape.voxels[i].index;
	}

	const int MAX_PALETTES = 1;
	GLuint paletteBank;
	glGenTextures(1, &paletteBank);
	glBindTexture(GL_TEXTURE_2D, paletteBank);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, MAX_PALETTES, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	for (int i = 0; i < MAX_PALETTES; i++)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, car.palette);

	GLuint volumeTexture;
	glGenTextures(1, &volumeTexture);
	glBindTexture(GL_TEXTURE_3D, volumeTexture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, shape.sizex, shape.sizey, shape.sizez, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);

	unsigned int maxValue = 255;
	int palette_index = 0;
	vec4 multColor(1, 1, 1, 1);
	float volTexelSize = 0.1;
	vec3 volResolution(shape.sizex, shape.sizey, shape.sizez);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		camera.handleInputs(window);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		train1.draw(mesh_shader, camera);

		// TODO: Push 3D Texture
		glUniform1i(glGetUniformLocation(shape_shader.id, "uVolTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, volumeTexture);

		PushTexture(paletteBank, shape_shader, "uColor", 1);

		vec3 shapePos(0, 0, 0);

		mat4 modelMatrix = scale(mat4(1.0f), vec3(shape.sizex, shape.sizey, shape.sizez) * volTexelSize);
		mat4 vpMatrix = camera.vpMatrix;
		mat4 vpInvMatrix = inverse(vpMatrix); // uVpInvMatrix[3][0], uVpInvMatrix[3][1], uVpInvMatrix[3][2] depends on camera position and rotation
		mat4 mvpMatrix = camera.vpMatrix * modelMatrix; // uMvpMatrix[3][0], uMvpMatrix[3][1], uMvpMatrix[3][2] depends on camera rotation and model position
		mat4 volMatrix = translate(mat4(1.0f), shapePos); // uVolMatrix[3][0], uVolMatrix[3][1], uVolMatrix[3][2] depends world position
		mat4 volMatrixInv = inverse(volMatrix); // uVolMatrixInv[3][0], uVolMatrixInv[3][1], uVolMatrixInv[3][2] depends negative world position

		glUniform1f(glGetUniformLocation(shape_shader.id, "uNear"), camera.NEAR_PLANE);
		glUniform1f(glGetUniformLocation(shape_shader.id, "uFar"), camera.FAR_PLANE);
		glUniform1ui(glGetUniformLocation(shape_shader.id, "uMaxValue"), maxValue);
		glUniform1i(glGetUniformLocation(shape_shader.id, "uPalette"), palette_index);
		glUniform4fv(glGetUniformLocation(shape_shader.id, "uMultColor"), 1, value_ptr(multColor));
		glUniform1f(glGetUniformLocation(shape_shader.id, "uVolTexelSize"), volTexelSize);
		glUniform3fv(glGetUniformLocation(shape_shader.id, "uVolResolution"), 1, value_ptr(volResolution));

		glUniform3fv(glGetUniformLocation(shape_shader.id, "uCameraPos"), 1, value_ptr(camera.position));
		glUniformMatrix4fv(glGetUniformLocation(shape_shader.id, "uModelMatrix"), 1, GL_FALSE, value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shape_shader.id, "uVpMatrix"), 1, GL_FALSE, value_ptr(vpMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shape_shader.id, "uMvpMatrix"), 1, GL_FALSE, value_ptr(mvpMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shape_shader.id, "uVolMatrix"), 1, GL_FALSE, value_ptr(volMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shape_shader.id, "uVolMatrixInv"), 1, GL_FALSE, value_ptr(volMatrixInv));

		cube.draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0, 0, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PushTexture(colorTexture, screen_shader, "uTexture", 0);
		PushTexture(normalTexture, screen_shader, "uNormal", 1);
		PushTexture(depthTexture, screen_shader, "uDepth", 2);
		PushTexture(bluenoise, screen_shader, "uBlueNoise", 3);

		glUniform1f(glGetUniformLocation(screen_shader.id, "uNear"), camera.NEAR_PLANE);
		glUniform1f(glGetUniformLocation(screen_shader.id, "uFar"), camera.FAR_PLANE);
		glUniform2f(glGetUniformLocation(screen_shader.id, "uPixelSize"), 0.0007, 0.00123);
		glUniform3fv(glGetUniformLocation(screen_shader.id, "uLightDir"), 1, value_ptr(lightDir));

		glUniform3fv(glGetUniformLocation(screen_shader.id, "uCameraPos"), 1, value_ptr(camera.position));
		glUniformMatrix4fv(glGetUniformLocation(screen_shader.id, "uVpMatrix"), 1, GL_FALSE, value_ptr(vpMatrix));
		glUniformMatrix4fv(glGetUniformLocation(screen_shader.id, "uVpInvMatrix"), 1, GL_FALSE, value_ptr(vpInvMatrix));

		screen.draw();

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
