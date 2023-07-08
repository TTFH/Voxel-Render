#ifndef UTILS_H
#define UTILS_H

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include "camera.h"

const float FAR_PLANE = 500;
const unsigned int WINDOW_WIDTH = 1024;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* InitOpenGL(const char* window_title);
void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif
