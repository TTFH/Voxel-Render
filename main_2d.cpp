#include "src/vao.h"
#include "src/ebo.h"
#include "src/camera.h"
#include "src/shader.h"
#include "src/utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

GLfloat vertices[] = {
	-1, -1, 0,
	-1,  1, 0,
	 1, -1, 0,
	 1,  1, 0,
};

GLuint indices[] = {
	0, 1, 2,
	1, 3, 2,
};

int main() {
	GLFWwindow* window = InitOpenGL("OpenGL");
	Shader shader_program("shaders/2d_vert.glsl", "shaders/2d_frag.glsl");

	Camera camera;
	camera.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, vec3(0, 0, -1));

	VAO vao1;
	vao1.Bind();
	VBO vbo1(vertices, sizeof(vertices));
	EBO ebo1(indices, sizeof(indices));
	vao1.LinkAttrib(vbo1, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao1.Unbind();
	vbo1.Unbind();
	ebo1.Unbind();

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

	// Handle screenshoots and fullscreen
	glfwSetWindowUserPointer(window, &camera);
	glfwSetKeyCallback(window, key_press_callback);

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

		shader_program.Use();
		glUniform1f(glGetUniformLocation(shader_program.id, "time"), glfwGetTime());
		glUniform2f(glGetUniformLocation(shader_program.id, "resolution"), camera.screen_width, camera.screen_height);
		vao1.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
