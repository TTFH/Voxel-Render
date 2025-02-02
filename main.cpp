#include <map>
#include <vector>
#include <stdint.h>
#include <string.h>

#include "src/light.h"
#include "src/utils.h"
#include "src/shader.h"
#include "src/skybox.h"
#include "src/overlay.h"
#include "src/render_mesh.h"
#include "src/scene_loader.h"
#include "src/postprocessing.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

using namespace std;
using namespace glm;

int main(int argc, char* argv[]) {
	GLFWwindow* window = InitOpenGL("Voxel Render");
	Shader boundary_shader("boundary");
	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader rope_shader("shaders/rope_vert.glsl", "shaders/rope_frag.glsl");
	Shader shadowmap_shader("shaders/shadowmap_vert.glsl", "shaders/shadowmap_frag.glsl");
	Shader skybox_shader("shaders/skybox_vert.glsl", "shaders/skybox_frag.glsl");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader voxel_glass_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_glass_frag.glsl");
	Shader voxel_gm_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_frag.glsl");
	Shader voxel_hex_shader("shaders/voxel_hex_vert.glsl", "shaders/voxel_frag.glsl");
	Shader voxel_rtx_shader("gbuffervox");
	Shader water_shader("shaders/water_vert.glsl", "shaders/water_frag.glsl");

	map<const char*, Shader*> shaders = {
		{"boundary_shader", &boundary_shader},
		{"mesh_shader", &mesh_shader},
		{"rope_shader", &rope_shader},
		{"shadowmap_shader", &shadowmap_shader},
		{"skybox_shader", &skybox_shader},
		{"voxbox_shader", &voxbox_shader},
		{"voxel_glass_shader", &voxel_glass_shader},
		{"voxel_gm_shader", &voxel_gm_shader},
		{"voxel_hex_shader", &voxel_hex_shader},
		{"voxel_rtx_shader", &voxel_rtx_shader},
		{"water_shader", &water_shader},
	};

	Skybox skybox("day");
	Camera camera(vec3(0, 2.5, 10));
	Light light(vec3(-35, 130, -132));
	Scene scene(GetScenePath(argc, argv));
	camera.position = scene.spawnpoint.pos;
	camera.position.y += 1.8;
	camera.direction = scene.spawnpoint.rot * vec3(0, 0, 1);
	Overlay overlay(window, camera, light, skybox, shaders);

#ifdef _BLENDER
	ShadowVolume shadow_volume(40, 10, 40);
	scene.push(shadow_volume);
	shadow_volume.updateTexture();
#endif

	Mesh model("meshes/LTM1300.obj");
	model.addTexture("meshes/LTM1300.png");
	model.addTexture("meshes/LTM1300_specular.png");
	model.addTexture("meshes/LTM1300_normal.png");
	Mesh glass("meshes/LTM1300_glass.obj");
	glass.addTexture("textures/glass.png");
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

		overlay.Frame();
		// Shadows
		light.bindShadowMap(shadowmap_shader);
		scene.draw(shadowmap_shader, camera, GREEDY);
		shadowmap_shader.PushInt("side", overlay.hex_orientation);
		scene.draw(shadowmap_shader, camera, HEXAGON);
		scene.drawVoxbox(shadowmap_shader, camera);
		scene.drawMesh(shadowmap_shader, camera);
		light.unbindShadowMap(camera);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		voxel_rtx_shader.Use();
		light.draw(voxel_rtx_shader, camera, RTX);
		scene.draw(voxel_rtx_shader, camera, RTX);

		voxel_gm_shader.Use();
		voxel_gm_shader.PushInt("transparent_glass", overlay.transparent_glass);
		light.pushUniforms(voxel_gm_shader);
		scene.draw(voxel_gm_shader, camera, GREEDY);

		voxel_hex_shader.Use();
		voxel_hex_shader.PushInt("side", overlay.hex_orientation);
		light.pushUniforms(voxel_hex_shader);
		scene.draw(voxel_hex_shader, camera, HEXAGON);

		voxbox_shader.Use();
		light.pushUniforms(voxbox_shader);
		scene.drawVoxbox(voxbox_shader, camera);

		mesh_shader.Use();
		light.pushUniforms(mesh_shader);
		scene.drawMesh(mesh_shader, camera);

		glEnable(GL_BLEND);
		water_shader.Use();
		scene.drawWater(water_shader, camera);

		if (overlay.transparent_glass) {
			voxel_glass_shader.Use();
			voxel_glass_shader.PushVec3("light_pos", light.position);
			scene.draw(voxel_glass_shader, camera, GREEDY);
		}
		glDisable(GL_BLEND);

		rope_shader.Use();
		scene.drawRope(rope_shader, camera);

		skybox_shader.Use();
		skybox.draw(skybox_shader, camera);

		glEnable(GL_BLEND);
		boundary_shader.Use();
		scene.drawBoundary(boundary_shader, camera);
		glDisable(GL_BLEND);

		overlay.Render();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
