#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "vox_loader.h"

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

using namespace std;

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* InitOpenGL(const char* window_title);

void Screenshot(GLFWwindow* window);
void ToggleFullscreen(GLFWwindow* window);
void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

string ReadFile(const char* filename);
string RemoveExtension(const string& path);
string GetScenePath(int argc, char* argv[]);

GLuint LoadTexture2D(const char* path);
void SaveTexture(const char* path, GLuint texture);

int CeilExp2(int value, int n);

#endif
