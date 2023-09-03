#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include "vao.h"
#include "vbo.h"
#include "camera.h"

using namespace std;

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

class UI_Rectangle {
private:
	VAO vao;
public:
	UI_Rectangle();
	void draw(Shader& shader, float offset_x, float offset_y);
};

GLFWwindow* InitOpenGL(const char* window_title);

void Screenshot(GLFWwindow* window);
void ToggleFullscreen(GLFWwindow* window);
void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

string GetScenePath(int argc, char* argv[]);

void PushTime(Shader& shader, float offset = 0);
GLuint LoadTexture(const char* path, GLenum format = GL_RGBA);
void PushTexture(GLuint texture_id, Shader& shader, const char* uniform, GLuint unit = 0);

uint8_t*** MatrixInit(const MV_Shape& shape);
void MatrixDelete(uint8_t*** &voxels, const MV_Shape& shape);
void TrimShape(uint8_t*** &voxels, int sizex, int sizey, int sizez);

#endif
