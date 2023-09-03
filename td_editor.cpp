#include <map>
#include <stdio.h>
#include <string.h>

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/ebo.h"
#include "src/mesh.h"
#include "src/light.h"
#include "src/utils.h"
#include "src/shader.h"

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

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("Totally not Teardown");

	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader screen_shader("editorlighting");
	Shader shape_shader("editorvox");

	Screen screen;
	//GLuint test_tex = LoadTexture("textures/td_editor.png", GL_RGB);
	GLuint bluenoise = LoadTexture("textures/bluenoise.png", GL_RGB);
	vec3 lightDir(0.38, -0.76, 0.53);
	Light light(vec3(-35, 130, -132)); // remove

	const int NEAR_PLANE = 0.1;
	const int FAR_PLANE = 1000;

	Camera camera;
	vec3 cameraPos(0, 4.8, 8.8);
	camera.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, cameraPos);
	camera.updateMatrix(45, NEAR_PLANE, FAR_PLANE);

	mat4 vpMatrix = camera.cameraMatrix;
	mat4 vpInvMatrix = inverse(vpMatrix);

	Mesh train1("trains/Inyo.obj", "trains/Inyo.png");
	train1.setWorldTransform(vec3(0, 0, -5));
/*
	printf("vpMatrix\n");
	printf("%.2f %.2f %.2f %.2f\n", vpMatrix[0][0], vpMatrix[0][1], vpMatrix[0][2], vpMatrix[0][3]);
	printf("%.2f %.2f %.2f %.2f\n", vpMatrix[1][0], vpMatrix[1][1], vpMatrix[1][2], vpMatrix[1][3]);
	printf("%.2f %.2f %.2f %.2f\n", vpMatrix[2][0], vpMatrix[2][1], vpMatrix[2][2], vpMatrix[2][3]);
	printf("%.2f %.2f %.2f %.2f\n", vpMatrix[3][0], vpMatrix[3][1], vpMatrix[3][2], vpMatrix[3][3]);
	printf("\n");
*/
	glUniform1f(glGetUniformLocation(screen_shader.id, "uNear"), NEAR_PLANE);
	glUniform1f(glGetUniformLocation(screen_shader.id, "uFar"), FAR_PLANE);
	glUniform2f(glGetUniformLocation(screen_shader.id, "uPixelSize"), 0.0007, 0.00123);
	glUniform3fv(glGetUniformLocation(screen_shader.id, "uCameraPos"), 1, value_ptr(cameraPos));
	glUniform3fv(glGetUniformLocation(screen_shader.id, "uLightDir"), 1, value_ptr(lightDir));
	glUniformMatrix4fv(glGetUniformLocation(screen_shader.id, "uVpMatrix"), 1, GL_FALSE, value_ptr(vpMatrix));
	glUniformMatrix4fv(glGetUniformLocation(screen_shader.id, "uVpInvMatrix"), 1, GL_FALSE, value_ptr(vpInvMatrix));

	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLuint colorTexture;
	GLuint normalTexture;
	GLuint depthTexture;
	glGenTextures(1, &colorTexture);
	glGenTextures(1, &normalTexture);
	glGenTextures(1, &depthTexture);

	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);

	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawBuffers);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

/*
	mat4 modelMatrix();
	mat4 mvpMatrix();

	mat4 volMatrix();
	mat4 volMatrixInv();

	unsigned int maxValue = 255;
	int palette_index = 0;
	vec4 multColor(1, 1, 1, 1);
	float volTexelSize = 0.1;
	vec3 volResolution();
*/
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

		light.pushLight(mesh_shader);
		light.pushProjection(mesh_shader);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		train1.draw(mesh_shader, camera);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0, 0, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PushTexture(colorTexture, screen_shader, "uTexture", 0);
		PushTexture(normalTexture, screen_shader, "uNormal", 1);
		PushTexture(depthTexture, screen_shader, "uDepth", 2);
		PushTexture(bluenoise, screen_shader, "uBlueNoise", 3);
		screen.draw();

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
