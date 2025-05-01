#include "utils.h"
#include "overlay.h"

static vector<const char*> skyboxes = {
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
	"village",
	"village_2",
	"village_3",
	"village_4",
	"Ultimate_Skies_4k_0066",
	"Ultimate_Skies_4k_0067",
};

Overlay::Overlay(GLFWwindow* window, const Camera& camera, const Light& light, Skybox& skybox, const map<const char*, Shader*>& shaders) : 
	window(window), camera(camera), light(light), skybox(skybox), shaders(shaders) {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	dialog_flags |= ImGuiWindowFlags_NoResize;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
}

void Overlay::Frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Voxel Render - Settings", NULL, dialog_flags);

		ImGui::Text("Camera position: (%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);
		ImGui::Text("Camera direction: (%.2f, %.2f, %.2f)", camera.direction.x, camera.direction.y, camera.direction.z);
		ImGui::Text("Light position: (%.2f, %.2f, %.2f)", light.position.x, light.position.y, light.position.z);
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

		if (ImGui::BeginCombo("##combo", selected_shader)) {
			for (map<const char*, Shader*>::const_iterator it = shaders.begin(); it != shaders.end(); it++) {
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
			shaders.at(selected_shader)->Reload();

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

		ImGui::ColorEdit3("Clear color", (float*)&clear_color);
		ImGui::Dummy(ImVec2(0, 10));
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("FPS: %.0f", io.Framerate);
		ImGui::Text("Frametime: %.1f ms", 1000.0f / io.Framerate);
		ImGui::End();
	}
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
}

void Overlay::Render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
