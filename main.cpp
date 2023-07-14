#include "src/mesh.h"
#include "src/light.h"
#include "src/utils.h"
#include "src/shader.h"
#include "src/skybox.h"
#include "src/shadowmap.h"
#include "src/xml_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"
/*
#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#include "imgui/backend/imgui_impl_opengl3.h"
*/

int main(int argc, char* argv[]) {
	GLFWwindow* window = InitOpenGL("OpenGL");
#if GREADY_MESHING_ENABLED
	Shader voxel_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_frag.glsl");
#else
	Shader voxel_shader("shaders/voxel_vert.glsl", "shaders/voxel_frag.glsl");
#endif
	Shader shader_2d("shaders/2d_vert.glsl", "shaders/2d_tex_frag.glsl");
	//Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader rope_shader("shaders/rope_vert.glsl", "shaders/rope_frag.glsl");
	Shader water_shader("shaders/water_vert.glsl", "shaders/water_frag.glsl");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader skybox_shader("shaders/skybox_vert.glsl", "shaders/skybox_frag.glsl");
	Shader shadowmap_shader("shaders/shadowmap_vert.glsl", "shaders/shadowmap_frag.glsl");

	Camera camera;
	UI_Rectangle rect;
	ShadowMap shadow_map;
	Light light(vec3(-35, 130, -132));
	Skybox skybox(skybox_shader, (float)WINDOW_WIDTH / WINDOW_HEIGHT);
	camera.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, vec3(0, 2.5, 10));

/*
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGuiWindowFlags dialog_flags = 0;
	dialog_flags |= ImGuiWindowFlags_NoResize;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");
	ImVec4 clear_color = ImVec4(0.35, 0.54, 0.8, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
*/
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
	Scene scene(path);
/*
	vector<Texture> train_tex = {
		Texture("trains/shinkansen.png", "diffuse", 0),
	};
	Mesh train("trains/shinkansen.obj", train_tex);

	vector<Texture> glass_textures = {
		Texture("meshes/glass.png", "diffuse", 0),
	};
	Mesh glass("meshes/CAT_140M3_glass.obj", glass_textures);
	vector<Texture> model_textures = {
		Texture("meshes/CAT_140M3.png", "diffuse", 0),
		Texture("meshes/CAT_140M3_specular.png", "specular", 1),
		Texture("meshes/CAT_140M3_normal.png", "normal", 2),
	};
	Mesh model("meshes/CAT_140M3.obj", model_textures);
*/

	if (scene.waters.size() != 1) {
		printf("[ERROR] There is no water!\n");
		exit(EXIT_FAILURE);
	}
	WaterRender* water = scene.waters[0];
	vec4 clip_plane_top = vec4(0, 1, 0, -water->GetHeight() + 0.5); // reflection
	vec4 clip_plane_bottom = vec4(0, -1, 0, water->GetHeight()); // refraction

	// FPS counter
	double dt = 0;
	double prev_time = 0;
	double actual_time = 0;
	unsigned int counter = 0;

	// Flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Handle screenshoots and fullscreen
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
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			printf("Camera position: (%.2f, %.2f, %.2f)\n", camera.position.x, camera.position.y, camera.position.z);
			printf("Camera orientation: (%.2f, %.2f, %.2f)\n", camera.orientation.x, camera.orientation.y, camera.orientation.z);
		}

		light.handleInputs(window);
		camera.handleInputs(window);
/*
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!", NULL, dialog_flags);
			ImGui::Text("This is some useless text.");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);

			if (ImGui::Button("Button"))
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.1f ms/frame (%.0f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}
*/
		// Not needed, light pos is send to the shader with shadow_map.PushShadows()
		//light.pushLight(voxel_shader);
		//light.pushLight(mesh_shader);
		//light.pushLight(water_shader);
		//light.pushLight(voxbox_shader);
		//light.pushProjection(shadowmap_shader);

		shadow_map.BindShadowMap();
		light.pushProjection(shadowmap_shader);
		scene.draw(shadowmap_shader, camera);
		scene.drawVoxbox(shadowmap_shader, camera);
		//train.draw(shadowmap_shader, camera, vec3(-10, 5, -10), 0);
		//model.draw(shadowmap_shader, camera, vec3(12, 4.3, 30), 170);
		shadow_map.UnbindShadowMap(camera);

		glEnable(GL_CLIP_DISTANCE0);
		float distance = 2.0 * (camera.position.y - water->GetHeight());

		camera.position.y -= distance;
		camera.orientation.y *= -1;
		camera.updateMatrix(45, 0.1, FAR_PLANE);
		water->BindReflectionFB();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadow_map.PushShadows(voxel_shader, light.getProjection());
		scene.draw(voxel_shader, camera, clip_plane_top);

		camera.position.y += distance;
		camera.orientation.y *= -1;
		camera.updateMatrix(45, 0.1, FAR_PLANE);
		water->BindrefractionFB();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadow_map.PushShadows(voxel_shader, light.getProjection());
		scene.draw(voxel_shader, camera, clip_plane_bottom);

		water->UnbindFB(camera);
		glDisable(GL_CLIP_DISTANCE0);

		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*
		shadow_map.PushShadows(mesh_shader, light.getProjection());
		train.draw(mesh_shader, camera, vec3(-10, 5, -10), 0);
		glass.draw(mesh_shader, camera, vec3(12, 4.3, 30), 170);
		model.draw(mesh_shader, camera, vec3(12, 4.3, 30), 170);
*/
		shadow_map.PushShadows(voxel_shader, light.getProjection());
		scene.draw(voxel_shader, camera);
		shadow_map.PushShadows(voxbox_shader, light.getProjection());
		scene.drawVoxbox(voxbox_shader, camera);
		scene.drawRope(rope_shader, camera);

		light.pushLight(water_shader);
		glUniform1f(glGetUniformLocation(water_shader.id, "time"), glfwGetTime());
		glEnable(GL_BLEND);
		scene.drawWater(water_shader, camera);
		glDisable(GL_BLEND);

		//light.draw(voxel_shader, camera); // Debug light pos
		skybox.Draw(skybox_shader, camera);

		rect.draw(shader_2d, water->reflectionDepthTexture, -0.9, 0.4);
		rect.draw(shader_2d, water->refractionDepthTexture, 0.4, 0.4);
/*
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
*/
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
