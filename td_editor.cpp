#include <map>
#include <stdio.h>
#include <string.h>

#include "../glad/glad.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "src/ebo.h"
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
	void draw(Shader& shader, GLuint texture_id);
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

void Screen::draw(Shader& shader, GLuint texture_id) {
	PushTexture(texture_id, shader, "uTexture", 0);
	vao.Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("Totally not Teardown");

	Shader screen_shader_test("screentest");
	Shader screen_shader("editorlighting");
	Shader shape_shader("editorvox");

	Screen screen;
	Camera camera;
	camera.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, vec3(0, 2.5, 10));
	
	GLuint test_tex = LoadTexture("textures/td_editor.png", GL_RGB);
	//GLuint bluenoise = LoadTexture("textures/bluenoise.png", GL_RGB);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		camera.handleInputs(window);

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		screen.draw(screen_shader_test, test_tex);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
