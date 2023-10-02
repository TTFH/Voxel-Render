#include <time.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "ebo.h"
#include "utils.h"

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"

using namespace std;

GLFWwindow* InitOpenGL(const char* window_title) {
	int major = 4;
	int minor = 6;
#ifdef __APPLE__
	major = 4;
	minor = 1;
#elif __linux__
	major = 4;
	minor = 2;
#endif

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_SAMPLES, 1); // MSAA
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_title, NULL, NULL);
	if (window == NULL) {
		printf("[GLFW] Failed to initialize OpenGL\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	return window;
}

void debug(const char* message) {
	GLenum error;
	do {
		error = glGetError();
		if (error != GL_NO_ERROR)
			printf("ERROR %d %s\n", error, message);
	} while (error != GL_NO_ERROR);
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

void Screenshot(GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	uint8_t* pixels = new uint8_t[width * height * 3];
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	FlipImageVertically(width, height, pixels);
	string filename = "screenshot_" + to_string(time(NULL)) + ".png";
	stbi_write_png(filename.c_str(), width, height, 3, pixels, 3 * width);
	delete[] pixels;
}

static bool g_fullscreen = false;

void ToggleFullscreen(GLFWwindow* window) {
	g_fullscreen = !g_fullscreen;
	Camera* camera = (Camera*)glfwGetWindowUserPointer(window);
	if (g_fullscreen) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		glViewport(0, 0, mode->width, mode->height);
		if (camera != NULL)
			camera->updateScreenSize(mode->width, mode->height);
	} else {
		glfwSetWindowMonitor(window, NULL, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		if (camera != NULL)
			camera->updateScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
}

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window; (void)scancode; (void)mods;
	if (action == GLFW_RELEASE) return;
	if (key == GLFW_KEY_F10)
		Screenshot(window);
	else if (key == GLFW_KEY_F11)
		ToggleFullscreen(window);
}

string GetScenePath(int argc, char* argv[]) {
	string path = "main.xml";
	if (argc > 1) {
		path = argv[1];
		if (path.find(".xml") == string::npos) {
			if (path.back() == '/' || path.back() == '\\')
				path += "main.xml";
			else
				path += "/main.xml";
		}
	}
	return path;
}

/*
	specular	- GL_RED
	displacement- GL_RED
	dudv		- GL_RGB
	normal		- GL_RGB
	diffuse		- GL_RGBA
*/
GLuint LoadTexture(const char* path, GLenum format) {
	GLuint texture_id;
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	uint8_t* data = stbi_load(path, &width, &height, &channels, STBI_default);
	printf("Loading texture %s with %d channels\n", path, channels);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float clampColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	GLenum image_format = GL_RGB;
	if (channels == 1)
		image_format = GL_RED;
	else if (channels == 4)
		image_format = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, image_format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture_id;
}

int RoundTo_nth_Power(int value, int n) {
	int exp = 1 << n;
	int result = value;
	if (value % exp != 0)
		result += exp - (value % exp);
	return result;
}

const uint8_t HOLE = 255;

uint8_t*** MatrixInit(const MV_Shape& shape) {
	uint8_t*** voxels = new uint8_t**[shape.sizex];
	for (int i = 0; i < shape.sizex; i++) {
		voxels[i] = new uint8_t*[shape.sizey];
		for (int j = 0; j < shape.sizey; j++) {
			voxels[i][j] = new uint8_t[shape.sizez];
			for (int k = 0; k < shape.sizez; k++)
				voxels[i][j][k] = 0;
		}
	}

	for (unsigned int i = 0; i < shape.voxels.size(); i++) {
		MV_Voxel v = shape.voxels[i];
		if (v.index != HOLE)
			voxels[v.x][v.y][v.z] = v.index;
	}

	return voxels;
}

void MatrixDelete(uint8_t*** &voxels, const MV_Shape& shape) {
	for (int i = 0; i < shape.sizex; i++) {
		for (int j = 0; j < shape.sizey; j++)
			delete[] voxels[i][j];
		delete[] voxels[i];
	}
	delete[] voxels;
}

// Remove hidden voxels
void TrimShape(uint8_t*** &voxels, int sizex, int sizey, int sizez) {
	bool*** solid = new bool**[sizex];
	for (int i = 0; i < sizex; i++) {
		solid[i] = new bool*[sizey];
		for (int j = 0; j < sizey; j++) {
			solid[i][j] = new bool[sizez];
			for (int k = 0; k < sizez; k++)
				solid[i][j][k] = voxels[i][j][k] != 0;
		}
	}

	int count = 0;
	for (int i = 0; i < sizex; i++)
		for (int j = 0; j < sizey; j++)
			for (int k = 0; k < sizez; k++) {
				if (i > 0 && j > 0 && k > 0 && i < sizex - 1 && j < sizey - 1 && k < sizez - 1) {
					if (solid[i][j][k] &&
					  solid[i - 1][j][k] && solid[i][j - 1][k] && solid[i][j][k - 1] &&
					  solid[i + 1][j][k] && solid[i][j + 1][k] && solid[i][j][k + 1]) {
						voxels[i][j][k] = 0;
						count++;
					}
				}
			}

	//if (count > 0)
	//	printf("Trimmed %d voxels\n", count);

	for (int i = 0; i < sizex; i++) {
		for (int j = 0; j < sizey; j++)
			delete[] solid[i][j];
		delete[] solid[i];
	}
}
