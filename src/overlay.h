#ifndef OVERLAY_H
#define OVERLAY_H

#include <map>
#include <string>
#include <vector>

#include "light.h"
#include "camera.h"
#include "shader.h"
#include "skybox.h"

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include "../imgui/imgui.h"
#include "../imgui/backend/imgui_impl_glfw.h"
#include "../imgui/backend/imgui_impl_opengl3.h"

using namespace std;

class Overlay {
private:
	ImGuiWindowFlags dialog_flags = 0;
	ImVec4 clear_color = ImVec4(0.35, 0.54, 0.8, 1);
	const char* selected_skybox = "day";
	const char* selected_shader = "voxel_rtx_shader";

	GLFWwindow* window;
	Light* light;
	Camera* camera;
	Skybox* skybox;
	map<const char*, Shader*>* shaders;
public:
	bool transparent_glass = true;
	int hex_orientation = 1;

	Overlay(GLFWwindow* window, Camera& camera, Light& light, Skybox& skybox, map<const char*, Shader*>& shaders);
	void Frame();
	void Render();
};

#endif
