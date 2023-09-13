#include <map>
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
#if RENDER_METHOD == GREEDY
	Shader voxel_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_frag.glsl");
#elif RENDER_METHOD == HEXAGON
	Shader voxel_shader("shaders/voxel_hex_vert.glsl", "shaders/voxel_frag.glsl");
#elif RENDER_METHOD == RTX
	Shader voxel_shader("editorvox");
#endif
	Shader screen_shader("editorlighting");
	Shader voxel_glass_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_glass_frag.glsl");
	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader rope_shader("shaders/rope_vert.glsl", "shaders/rope_frag.glsl");
	Shader water_shader("shaders/water_vert.glsl", "shaders/water_frag.glsl");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader skybox_shader("shaders/skybox_vert.glsl", "shaders/skybox_frag.glsl");
	Shader shadowmap_shader("shaders/shadowmap_vert.glsl", "shaders/shadowmap_frag.glsl");

	map<const char*, Shader*> shaders = {
		{"voxel_shader", &voxel_shader},
		{"voxel_glass_shader", &voxel_glass_shader},
		{"screen_shader", &screen_shader},
		{"mesh_shader", &mesh_shader},
		{"rope_shader", &rope_shader},
		{"water_shader", &water_shader},
		{"voxbox_shader", &voxbox_shader},
		{"skybox_shader", &skybox_shader},
		{"shadowmap_shader", &shadowmap_shader}
	};

	Skybox skybox;
	Screen screen;
	ShadowMap shadow_map;
	Camera camera(vec3(0, 2.5, 10));
	Light light(vec3(-35, 130, -132));
	Scene scene(GetScenePath(argc, argv));
	bool transparent_glass = false;

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

		light.handleInputs(window);
		camera.handleInputs(window);

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
			static int hex_orientation = 0;
			ImGui::Combo("##orientation", &hex_orientation, "Cube\0Top\0Front\0Side\0");
		#if RENDER_METHOD == GREEDY
			voxel_shader.PushInt("transparent_glass", transparent_glass);
		#elif RENDER_METHOD == HEXAGON
			shadowmap_shader.PushInt("side", hex_orientation);
			voxel_shader.PushInt("side", hex_orientation);
		#endif
			ImGui::PopItemWidth();

			if (ImGui::Button("Toggle fullscreen"))
				ToggleFullscreen(window);
			ImGui::SameLine();
			if (ImGui::Button("Screenshot"))
				Screenshot(window);

			static const char* selected_shader = "voxel_shader";
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

			ImGui::ColorEdit3("Clear color", (float*)&clear_color);
			ImGui::Dummy(ImVec2(0, 10));
			ImGui::Text("FPS: %.0f", io.Framerate);
			ImGui::Text("Frametime: %.1f ms", 1000.0f / io.Framerate);
			ImGui::End();
		}
#if RENDER_METHOD == RTX
		screen.start();
		scene.draw(voxel_shader, camera);
		screen.end();
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screen.draw(screen_shader, camera);

		mesh_shader.PushVec3("lightpos", light.getPosition());
		voxbox_shader.PushVec3("lightpos", light.getPosition());
		mesh_shader.PushMatrix("lightProjection", light.getProjection());
		voxbox_shader.PushMatrix("lightProjection", light.getProjection());
		shadowmap_shader.PushMatrix("lightProjection", light.getProjection());

		shadow_map.BindShadowMap();
		scene.drawMesh(shadowmap_shader, camera);
		scene.drawVoxbox(shadowmap_shader, camera);
		shadow_map.UnbindShadowMap(camera);

		shadow_map.PushShadows(mesh_shader);
		scene.drawMesh(mesh_shader, camera);
		shadow_map.PushShadows(voxbox_shader);
		scene.drawVoxbox(voxbox_shader, camera);

		glEnable(GL_BLEND);
		scene.drawWater(water_shader, camera);
		glDisable(GL_BLEND);
		scene.drawRope(rope_shader, camera);
		skybox.draw(skybox_shader, camera);
#else
		voxel_shader.PushVec3("lightpos", light.getPosition());
		voxel_glass_shader.PushVec3("lightpos", light.getPosition());
		mesh_shader.PushVec3("lightpos", light.getPosition());
		voxbox_shader.PushVec3("lightpos", light.getPosition());

		voxel_shader.PushMatrix("lightProjection", light.getProjection());
		mesh_shader.PushMatrix("lightProjection", light.getProjection());
		voxbox_shader.PushMatrix("lightProjection", light.getProjection());
		shadowmap_shader.PushMatrix("lightProjection", light.getProjection());

		// Shadows
		shadow_map.BindShadowMap();
		scene.draw(shadowmap_shader, camera);
		scene.drawVoxbox(shadowmap_shader, camera);
		scene.drawMesh(shadowmap_shader, camera);
		shadow_map.UnbindShadowMap(camera);

		// Water shader
		if (scene.waters.size() > 0) {
			WaterRender* water = scene.waters[0];
			const vec4 clip_plane_top(0, 1, 0, -water->GetHeight()); // reflection
			const vec4 clip_plane_bottom(0, -1, 0, water->GetHeight()); // refraction
			float distance = 2.0 * (camera.position.y - water->GetHeight());
			glEnable(GL_CLIP_DISTANCE0);

			camera.translateAndInvertPitch(-distance);
			water->BindReflectionFB();
			shadow_map.PushShadows(mesh_shader);
			scene.drawMesh(mesh_shader, camera);
			shadow_map.PushShadows(voxel_shader);
			scene.draw(voxel_shader, camera, clip_plane_top);

			camera.translateAndInvertPitch(distance);
			water->BindRefractionFB();
			shadow_map.PushShadows(voxel_shader);
			scene.draw(voxel_shader, camera, clip_plane_bottom);

			water->UnbindFB(camera);
			glDisable(GL_CLIP_DISTANCE0);
		}

		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shadow_map.PushShadows(mesh_shader);
		scene.drawMesh(mesh_shader, camera);
		shadow_map.PushShadows(voxel_shader);
		scene.draw(voxel_shader, camera);
		shadow_map.PushShadows(voxbox_shader);
		scene.drawVoxbox(voxbox_shader, camera);

		water_shader.PushFloat("time", glfwGetTime());
		glEnable(GL_BLEND);
		scene.drawWater(water_shader, camera);
	#if RENDER_METHOD == GREEDY
		if (transparent_glass)
			scene.draw(voxel_glass_shader, camera);
	#endif
		glDisable(GL_BLEND);

		scene.drawRope(rope_shader, camera);
		light.draw(voxel_shader, camera);
		skybox.draw(skybox_shader, camera);
#endif
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
