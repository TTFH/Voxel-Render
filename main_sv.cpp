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
	Shader ambient_light_shader("ambientlight");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader denoise_shader("denoise");

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
	screen4.setTexture(framebuffer.depth_texture, 1);

	SimpleScreen fb_light(vec2(0, 0), vec2(1, 1), true);
	SimpleScreen fb_denoise(vec2(0, 0), vec2(1, 1), true);
	GLuint old_texture;
	glGenTextures(1, &old_texture);
	glBindTexture(GL_TEXTURE_2D, old_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

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

		RTX_Render::random_frame++;
		RTX_Render::random_frame %= 60;

		//glClearColor(0.35, 0.54, 0.8, 1);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		framebuffer.start();
		voxel_rtx_shader.use();
		scene.draw(voxel_rtx_shader, camera, RTX);

		voxbox_shader.use();
		scene.drawVoxbox(voxbox_shader, camera);
		framebuffer.end();

		//lighting_shader.use();
		//framebuffer.draw(lighting_shader, camera);

		glCopyImageSubData(fb_denoise.getTexture(), GL_TEXTURE_2D, 0, 0, 0, 0,
			old_texture, GL_TEXTURE_2D, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 1);

		fb_light.start();
		ambient_light_shader.use();
		framebuffer.pushUniforms(ambient_light_shader);
		scene.drawShadowVolume(ambient_light_shader, camera);
		fb_light.end();

		fb_denoise.start();
		denoise_shader.use();
		denoise_shader.pushTexture2D("uNew", fb_light.getTexture(), 5);
		denoise_shader.pushTexture2D("uOld", old_texture, 6);
		framebuffer.pushUniforms(denoise_shader);
		scene.drawShadowVolume(denoise_shader, camera);
		fb_denoise.end();

		screen_shader.use();
		fb_denoise.draw(screen_shader, camera);

		glDisable(GL_DEPTH_TEST);
		screen_shader.use();
		screen1.draw(screen_shader, camera);
		screen2.draw(screen_shader, camera);
		screen3.draw(screen_shader, camera);
		screen4.draw(screen_shader, camera);
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
