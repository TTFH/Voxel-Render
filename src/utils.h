#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "vao.h"
#include "vbo.h"
#include "camera.h"

using namespace std;

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* InitOpenGL(const char* window_title);
void debug(const char* message);

void Screenshot(GLFWwindow* window);
void ToggleFullscreen(GLFWwindow* window);
void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

string GetScenePath(int argc, char* argv[]);
GLuint LoadTexture(const char* path, GLenum format = GL_RGBA);

uint8_t*** MatrixInit(const MV_Shape& shape);
void MatrixDelete(uint8_t*** &voxels, const MV_Shape& shape);
void TrimShape(uint8_t*** &voxels, int sizex, int sizey, int sizez);

#endif
