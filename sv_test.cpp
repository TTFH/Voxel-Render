#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "src/utils.h"
#include "src/shader.h"
#include "src/vox_loader.h"
#include "src/shadow_volume.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

using namespace std;
using namespace glm;

int main() {
	GLFWwindow* window = InitOpenGL("OpenGL");
	Shader sv_shader("debugvolume");
	Shader voxel_shader("editorvox");

	Camera camera(vec3(0, 2.5, 10));
	VoxLoader test_file("scorpionking.vox");

	ShadowVolume shadow_volume;
	shadow_volume.addShape(test_file.shapes[0]);

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

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		camera.handleInputs(window);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		shadow_volume.draw(sv_shader, camera);
		glDisable(GL_BLEND);
		test_file.draw(voxel_shader, camera, vec4(0, 1, 0, 0), vec3(6.8, 0, 0));

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
