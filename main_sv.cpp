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
#include "src/shadow_map.h"
#include "src/render_mesh.h"
#include "src/scene_loader.h"
#include "src/postprocessing.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

using namespace std;
using namespace glm;

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("Voxel Render");
	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader voxel_hex_shader("shaders/voxel_hex_vert.glsl", "shaders/voxel_frag.glsl");

	Camera camera(vec3(0, 2, 20));
	Light light(vec3(3, 4, 5));

	Mesh crane("meshes/LTM1300.obj");
	crane.addTexture("meshes/LTM1300.png");
	crane.addTexture("meshes/LTM1300_specular.png");
	crane.addTexture("meshes/LTM1300_normal.png");
	crane.setWorldTransform(vec3(20, 0, 0), 180);
	Mesh glass1("meshes/LTM1300_glass.obj");
	glass1.addTexture("meshes/glass.png");
	glass1.setWorldTransform(vec3(20, 0, 0), 180);

	Mesh plane("meshes/cfa-44.obj");
	plane.addTexture("meshes/fa44_02_D.png");
	plane.addTexture("meshes/fa44_02_MREC.png");
	plane.addTexture("meshes/fa44_00_N.png");
	plane.setWorldTransform(vec3(0, 0, 0), 90);
	Mesh glass2("meshes/cfa-44_glass.obj");
	glass2.addTexture("meshes/glass.png");
	glass2.setWorldTransform(vec3(0, 0, 0), 90);

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
		light.handleInputs(window);

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mesh_shader.Use();
		mesh_shader.PushVec3("light_pos", light.getPosition());
		crane.draw(mesh_shader, camera);
		plane.draw(mesh_shader, camera);
		glass1.draw(mesh_shader, camera);
		glass2.draw(mesh_shader, camera);

		voxel_hex_shader.Use();
		voxel_hex_shader.PushInt("side", 2);
		voxel_hex_shader.PushVec3("light_pos", light.getPosition());
		light.draw(voxel_hex_shader, camera, HEXAGON);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
