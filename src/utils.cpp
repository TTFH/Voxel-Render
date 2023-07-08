#include <stdio.h>
#include <time.h>
#include <string>

#include "utils.h"

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"

using namespace std;

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

void FlipImageVertically(int width, int height, uint8_t* data) {
	uint8_t rgb[3];
	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width; x++) {
			int top = 3 * (x + y * width);
			int bottom = 3 * (x + (height - y - 1) * width);
			memcpy(rgb, data + top, sizeof(rgb));
			memcpy(data + top, data + bottom, sizeof(rgb));
			memcpy(data + bottom, rgb, sizeof(rgb));
		}
	}
}

static bool fullscreen = false;

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window; (void)scancode; (void)mods;
	if (action == GLFW_RELEASE) return;
	if (key == GLFW_KEY_F10) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		uint8_t* pixels = new uint8_t[width * height * 3];
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		FlipImageVertically(width, height, pixels);
		string filename = "screenshot_" + to_string(time(NULL)) + ".png";
		stbi_write_png(filename.c_str(), width, height, 3, pixels, 3 * width);
		delete[] pixels;
	} else if (key == GLFW_KEY_F11) {
		// TODO: update skybox aspect ratio
		fullscreen = !fullscreen;
		Camera& camera = *(Camera*)glfwGetWindowUserPointer(window);
		if (fullscreen) {
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			glViewport(0, 0, mode->width, mode->height);
			camera.updateScreenSize(mode->width, mode->height);
		} else {
			glfwSetWindowMonitor(window, NULL, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			camera.updateScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		}
	}
}
