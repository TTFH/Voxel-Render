// Proof of concept for a moving shadow volume
// with low RAM and VRAM usage, and
// reusing the shadows on the overlapping areas

#include <time.h>
#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "src/ebo.h"
#include "src/mesh.h"
#include "src/camera.h"
#include "src/shader.h"
#include "src/skybox.h"
#include "src/xml_loader.h"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

const float FAR_PLANE = 500;
const unsigned int WINDOW_WIDTH = 1400;
const unsigned int WINDOW_HEIGHT = 1060;

GLFWwindow* InitOpenGL(const char* window_title) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_SAMPLES, 8); // MSAA
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_title, NULL, NULL);
	if (window == NULL) {
		printf("[GLFW] Failed to initialize OpenGL\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	return window;
}

bool fullscreen = false;

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window; (void)scancode; (void)mods;
	if (action == GLFW_RELEASE) return;
	if (key == GLFW_KEY_F11) {
		// TODO: update skybox aspect ratio
		fullscreen = !fullscreen;
		if (fullscreen) {
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			glViewport(0, 0, mode->width, mode->height);
		} else {
			glfwSetWindowMonitor(window, NULL, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		}
	}
}

GLfloat vertices[] = {
	// Position	// Tex Coord
	-1, -1, 0,	0, 0,
	-1,  1, 0,	0, 1,
	 1, -1, 0,	1, 0,
	 1,  1, 0,	1, 1,
};

GLuint indices[] = {
	0, 1, 2,
	1, 3, 2,
};

int main() {
	GLFWwindow* window = InitOpenGL("OpenGL");
	Shader shader_program("shaders/image_vert.glsl", "shaders/image_frag.glsl");

	VAO vao1;
	vao1.Bind();
	VBO vbo1(vertices, sizeof(vertices));
	EBO ebo1(indices, sizeof(indices));
	vao1.LinkAttrib(vbo1, 0, 3, GL_FLOAT, 5 * sizeof(GLfloat), (GLvoid*)0);
	vao1.LinkAttrib(vbo1, 1, 2, GL_FLOAT, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	vao1.Unbind();
	vbo1.Unbind();
	ebo1.Unbind();

	Texture texture1("lee_top_bw.png", "diffuse", GL_TEXTURE0);
	texture1.texUnit(shader_program, "tex0", 0);
	Texture texture2("lee_top_color.png", "diffuse", GL_TEXTURE0);
	texture2.texUnit(shader_program, "tex0", 0);

	// FPS counter
	double dt = 0;
	double prev_time = 0;
	double actual_time = 0;
	unsigned int counter = 0;

	// Flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetKeyCallback(window, key_press_callback);

	float x = 0;
	float y = 0;

	while (!glfwWindowShouldClose(window)) {
		actual_time = glfwGetTime();
		dt = actual_time - prev_time;
		counter++;
		if (dt >= 1.0) {
			int fps = (1.0 / dt) * counter;
			float ms = dt / counter * 1000;
			char window_title[128];
			sprintf(window_title, "OpenGL %s | FPS: %d | %.1f ms", glGetString(GL_VERSION), fps, ms);
			glfwSetWindowTitle(window, window_title);
			prev_time = actual_time;
			counter = 0;
		}

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			y += 0.01;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			y -= 0.01;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			x += 0.01;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			x -= 0.01;

		shader_program.Use();
		glUniform3f(glGetUniformLocation(shader_program.id, "scale"), 1, 1, 1);
		glUniform3f(glGetUniformLocation(shader_program.id, "offset"), 0, 0, 0);
		texture1.Bind();
		vao1.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glUniform3f(glGetUniformLocation(shader_program.id, "scale"), 0.6, 0.4, 1);
		glUniform3f(glGetUniformLocation(shader_program.id, "offset"), x, y, -0.01);
		texture2.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
