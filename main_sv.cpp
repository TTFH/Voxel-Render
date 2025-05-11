#include <map>
#include <vector>
#include <stdint.h>
#include <string.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "src/light.h"
#include "src/utils.h"
#include "src/shader.h"
#include "src/skybox.h"
#include "src/render_mesh.h"
#include "src/scene_loader.h"
#include "src/postprocessing.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

using namespace std;
using namespace glm;

int main(int argc, char* argv[]) {
	GLFWwindow* window = InitOpenGL("Shadow Volume");
	Shader sv_shader("debugvolume");
	Shader screen_shader("screen");
	Shader voxel_rtx_shader("gbuffervox");
	Shader lighting_shader("editorlighting");

	Camera camera;
	Screen framebuffer;
	RTX_Render::initTextures();
	Scene scene(GetScenePath(argc, argv));
	camera.position = scene.spawnpoint.pos;
	camera.position.y += 1.8;
	camera.direction = scene.spawnpoint.rot * vec3(0, 0, 1);

	SimpleScreen screen1(vec2(0.75, 0.75), vec2(0.25, 0.25), false);
	screen1.setTexture(framebuffer.color_texture);
	SimpleScreen screen2(vec2(0.75, 0.25), vec2(0.25, 0.25), false);
	screen2.setTexture(framebuffer.normal_texture);
	SimpleScreen screen3(vec2(0.75, -0.25), vec2(0.25, 0.25), false);
	screen3.setTexture(framebuffer.material_texture);
	SimpleScreen screen4(vec2(0.75, -0.75), vec2(0.25, 0.25), false);
	screen4.setTexture(framebuffer.depth_texture);

	glfwSetWindowUserPointer(window, &camera);
	glfwSetKeyCallback(window, key_press_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		camera.handleInputs(window);

		framebuffer.start();
		voxel_rtx_shader.use();
		scene.draw(voxel_rtx_shader, camera, RTX);
		framebuffer.end();

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//lighting_shader.use();
		//framebuffer.draw(lighting_shader, camera);

		sv_shader.use();
		scene.drawShadowVolume(sv_shader, camera);

		screen_shader.use();
		screen1.draw(screen_shader, camera);
		screen2.draw(screen_shader, camera);
		screen3.draw(screen_shader, camera);
		screen4.draw(screen_shader, camera);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
