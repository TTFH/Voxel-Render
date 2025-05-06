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
	Shader voxel_rtx_shader("editorvox");
	Shader screen_shader("editorlighting");

	Screen screen;
	Camera camera;
	Scene scene(GetScenePath(argc, argv));
	camera.position = scene.spawnpoint.pos;
	camera.position.y += 1.8;
	camera.direction = scene.spawnpoint.rot * vec3(0, 0, 1);

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

		screen.start();
		voxel_rtx_shader.use();
		scene.draw(voxel_rtx_shader, camera, RTX);
		screen.end();

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		screen_shader.use();
		screen.draw(screen_shader, camera);

		sv_shader.use();
		scene.drawShadowVolume(sv_shader, camera);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
