#include <map>
#include <vector>
#include <stdint.h>
#include <string.h>

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "src/mesh.h"
#include "src/light.h"
#include "src/utils.h"
#include "src/shader.h"
#include "src/skybox.h"
#include "src/shadowmap.h"
#include "src/xml_loader.h"
#include "src/lighting_rtx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#include "imgui/backend/imgui_impl_opengl3.h"

using namespace std;
using namespace glm;

int main(int argc, char* argv[]) {
	GLFWwindow* window = InitOpenGL("OpenGL");
	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader rope_shader("shaders/rope_vert.glsl", "shaders/rope_frag.glsl");
	Shader screen_shader("editorlighting");
	Shader shadowmap_shader("shaders/shadowmap_vert.glsl", "shaders/shadowmap_frag.glsl");
	Shader skybox_shader("shaders/skybox_vert.glsl", "shaders/skybox_frag.glsl");
	Shader sv_shader("debugvolume");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader voxel_glass_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_glass_frag.glsl");
	Shader voxel_gm_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_frag.glsl");
	Shader voxel_hex_shader("shaders/voxel_hex_vert.glsl", "shaders/voxel_frag.glsl");
	//Shader voxel_rtx_shader("editorvox");
	Shader voxel_rtx_shader("gbuffervox");
	Shader water_shader("shaders/water_vert.glsl", "shaders/water_frag.glsl");

	map<const char*, Shader*> shaders = {
		{"mesh_shader", &mesh_shader},
		{"rope_shader", &rope_shader},
		{"screen_shader", &screen_shader},
		{"shadowmap_shader", &shadowmap_shader},
		{"skybox_shader", &skybox_shader},
		{"sv_shader", &sv_shader},
		{"voxbox_shader", &voxbox_shader},
		{"voxel_glass_shader", &voxel_glass_shader},
		{"voxel_gm_shader", &voxel_gm_shader},
		{"voxel_hex_shader", &voxel_hex_shader},
		{"voxel_rtx_shader", &voxel_rtx_shader},
		{"water_shader", &water_shader}
	};

	const char* selected_skybox = "day";
	vector<const char*> skyboxes = {
		"arizona_desert",
		"arizona_desert2",
		"arizona_desert_cloudy",
		"arizona_desert_cloudy2",
		"arizona_desert_dawn",
		"arizona_desert_night",
		"arizona_desert_night_clear",
		"arizona_desert_sunset",
		"arizona_desert_sunset2",
		"cannon_2k",
		"CGSkies_0061_8k",
		"CGSkies_0086_8k",
		"CGSkies_0219_8k",
		"CGSkies_0274_8k",
		"cloudy",
		"cold_dramatic_clouds",
		"cold_sunny_evening",
		"cold_sunset",
		"cold_wispy_sky",
		"cool_clear_sunrise",
		"cool_day",
		"day",
		"industrial_sunset_2k",
		"jk2",
		"mod_cloudy",
		"mod_cold_sunny_evening",
		"mod_cold_sunset",
		"mod_cool_clear_sunrise",
		"mod_day",
		"mod_starry_night",
		"mod_starry_sky",
		"mod_sunset",
		"moonlit",
		"night",
		"night_clear",
		"overcast_day",
		"sky_vr_001",
		"sky",
		"sunflowers_2k",
		"sunset",
		"sunset_in_the_chalk_quarry_2k",
		"tc_day_05",
		"tc_day_07",
		"tc_hub_01",
		"tc_night_03",
		"tc_night_04",
		"tc_night_06",
		"tc_overcast_02",
		"tc_stormy_03",
		"tc_sunrise_clear_07",
		"tc_sunset_03",
		"tc_tornado_01",
		"tornado",
		"Ultimate_Skies_4k_0066",
		"Ultimate_Skies_4k_0067",
	};

	Skybox skybox(selected_skybox);
	Screen screen;
	ShadowMap shadow_map;
	Camera camera(vec3(0, 2.5, 10));
	Light light(vec3(-35, 130, -132));
	Scene scene(GetScenePath(argc, argv));
	bool transparent_glass = true;
	int hex_orientation = 2;

	//ShadowVolume shadow_volume(40, 10, 40);
	//scene.push(shadow_volume);
	//shadow_volume.updateTexture();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGuiWindowFlags dialog_flags = 0;
	dialog_flags |= ImGuiWindowFlags_NoResize;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
	ImVec4 clear_color = ImVec4(0.35, 0.54, 0.8, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

	//GLuint texture_test = LoadTexture("skyboxes/day/right.png", GL_RGBA, false);
	Mesh model("meshes/LTM1300.obj", "meshes/LTM1300.png", "meshes/LTM1300_specular.png");
	Mesh glass("meshes/LTM1300_glass.obj", "meshes/glass.png");
	scene.addMesh(&model);
	scene.addMesh(&glass);

	// FPS counter
	double dt = 0;
	double prev_time = 0;
	double actual_time = 0;
	unsigned int counter = 0;

	// Flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Handle screenshoot and fullscreen keys
	glfwSetWindowUserPointer(window, &camera);
	glfwSetKeyCallback(window, key_press_callback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
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

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		camera.handleInputs(window);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			light.handleInputs(window);
		else {
			model.handleInputs(window);
			glass.position = model.position;
			glass.rotation = model.rotation;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::Begin("Voxel Render - Settings", NULL, dialog_flags);

			ImGui::Text("Camera position: (%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);
			ImGui::Text("Camera direction: (%.2f, %.2f, %.2f)", camera.direction.x, camera.direction.y, camera.direction.z);
			ImGui::Text("Light position: (%.2f, %.2f, %.2f)", light.getPosition().x, light.getPosition().y, light.getPosition().z);
			ImGui::Dummy(ImVec2(0, 10));

			ImGui::Checkbox("Transparent glass", &transparent_glass);
			ImGui::Text("Hex Voxel Orientation: ");
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::Combo("##orientation", &hex_orientation, "Cube\0Top\0Front\0Side\0");
			ImGui::PopItemWidth();

			if (ImGui::Button("Toggle fullscreen"))
				ToggleFullscreen(window);
			ImGui::SameLine();
			if (ImGui::Button("Screenshot"))
				Screenshot(window);

			static const char* selected_shader = "voxel_rtx_shader";
			if (ImGui::BeginCombo("##combo", selected_shader)) {
				for (map<const char*, Shader*>::iterator it = shaders.begin(); it != shaders.end(); it++) {
					bool is_selected = strcmp(selected_shader, it->first) == 0;
					if (ImGui::Selectable(it->first, is_selected))
						selected_shader = it->first;
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload") && selected_shader != NULL)
				shaders[selected_shader]->Reload();

			if (ImGui::BeginCombo("##combosk", selected_skybox)) {
				for (vector<const char*>::iterator it = skyboxes.begin(); it != skyboxes.end(); it++) {
					bool is_selected = strcmp(selected_skybox, *it) == 0;
					if (ImGui::Selectable(*it, is_selected)) {
						selected_skybox = *it;
						skybox.ReloadTexture(selected_skybox);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			//ImGui::Image((void*)(intptr_t)texture_test, ImVec2(256, 256));

			ImGui::ColorEdit3("Clear color", (float*)&clear_color);
			ImGui::Dummy(ImVec2(0, 10));
			ImGui::Text("FPS: %.0f", io.Framerate);
			ImGui::Text("Frametime: %.1f ms", 1000.0f / io.Framerate);
			ImGui::End();
		}

		// Shadows
		shadow_map.BindShadowMap();
		shadowmap_shader.Use();
		shadowmap_shader.PushInt("side", hex_orientation);
		shadowmap_shader.PushMatrix("lightProjection", light.getProjection());
		scene.draw(shadowmap_shader, camera, HEXAGON);
		shadowmap_shader.PushInt("side", 0);
		scene.draw(shadowmap_shader, camera, GREEDY);
		scene.drawVoxbox(shadowmap_shader, camera);
		scene.drawMesh(shadowmap_shader, camera);
		shadow_map.UnbindShadowMap(camera);

		//screen.start();
		//scene.draw(voxel_rtx_shader, camera, RTX);
		//screen.end();
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//screen.draw(screen_shader, camera);

		voxel_rtx_shader.Use();
		light.draw(voxel_rtx_shader, camera); // TODO: param
		scene.draw(voxel_rtx_shader, camera, RTX);

		voxel_hex_shader.Use();
		voxel_hex_shader.PushInt("side", hex_orientation);
		voxel_hex_shader.PushVec3("lightpos", light.getPosition());
		voxel_hex_shader.PushMatrix("lightProjection", light.getProjection());
		shadow_map.PushShadows(voxel_hex_shader); // TODO: push texture sm.getTexture()
		scene.draw(voxel_hex_shader, camera, HEXAGON);

		voxel_gm_shader.Use();
		voxel_gm_shader.PushVec3("lightpos", light.getPosition());
		voxel_gm_shader.PushMatrix("lightProjection", light.getProjection());
		voxel_gm_shader.PushInt("transparent_glass", transparent_glass);
		shadow_map.PushShadows(voxel_gm_shader);
		scene.draw(voxel_gm_shader, camera, GREEDY);

		voxbox_shader.Use();
		voxbox_shader.PushVec3("lightpos", light.getPosition());
		voxbox_shader.PushMatrix("lightProjection", light.getProjection());
		shadow_map.PushShadows(voxbox_shader);
		scene.drawVoxbox(voxbox_shader, camera);

		mesh_shader.Use();
		mesh_shader.PushVec3("lightpos", light.getPosition());
		mesh_shader.PushMatrix("lightProjection", light.getProjection());
		shadow_map.PushShadows(mesh_shader);
		scene.drawMesh(mesh_shader, camera);

		glEnable(GL_BLEND);
		water_shader.Use();
		scene.drawWater(water_shader, camera);

		if (transparent_glass) {
			voxel_glass_shader.Use();
			voxel_glass_shader.PushVec3("lightpos", light.getPosition());
			scene.draw(voxel_glass_shader, camera, GREEDY);
		}
		glDisable(GL_BLEND);

		rope_shader.Use();
		scene.drawRope(rope_shader, camera);

		skybox_shader.Use();
		skybox.draw(skybox_shader, camera);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
