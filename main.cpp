#include <time.h>
#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "src/mesh.h"
#include "src/camera.h"
#include "src/shader.h"
#include "src/skybox.h"
#include "src/xml_loader.h"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

const float FAR_PLANE = 500;
const unsigned int WINDOW_WIDTH = 1024;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* InitOpenGL(const char* window_title) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_SAMPLES, 8); // MSAA
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_title, NULL, NULL);
	if (window == NULL) {
		printf("[GLFW] Failed to initialize OpenGL\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	return window;
}

void FlipImageVertically(int width, int height, uint8_t* data) {
	uint8_t rgb[3];
	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width; x++) {
			int top = 3 * (x + y * width);
			int bottom = 3 * (x + (height - y - 1) * width);

			memcpy(rgb, data + top, sizeof(rgb));
			memcpy(data + top, data + bottom, sizeof(rgb));
			memcpy(data + bottom, rgb, sizeof(rgb));
		}
	}
}

Camera camera;
bool fullscreen = false;

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window; (void)scancode; (void)mods;
	if (action == GLFW_RELEASE) return;
	if (key == GLFW_KEY_F10) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		uint8_t* pixels = new uint8_t[width * height * 3];
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		FlipImageVertically(width, height, pixels);
		string filename = "screenshot_" + to_string(time(NULL)) + ".png";
		stbi_write_png(filename.c_str(), width, height, 3, pixels, 3 * width);
		delete[] pixels;
	} else if (key == GLFW_KEY_F11) {
		// TODO: update skybox aspect ratio
		fullscreen = !fullscreen;
		if (fullscreen) {
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			glViewport(0, 0, mode->width, mode->height);
			camera.updateScreenSize(mode->width, mode->height);
		} else {
			glfwSetWindowMonitor(window, NULL, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			camera.updateScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		}
	}
}

class ShadowMap {
private:
	GLuint shadowMap;
	GLuint shadowMapFBO;
	const unsigned int shadowMapWidth = 4096;
	const unsigned int shadowMapHeight = 4096;
public:
	ShadowMap() {
		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

		glGenFramebuffers(1, &shadowMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
			printf("[ERROR] Shadow Map framebuffer failed with status %d\n", fboStatus);
	}

	void BindShadowMap() {
		glViewport(0, 0, shadowMapWidth, shadowMapHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void UnbindShadowMap(Camera& camera) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, camera.screen_width, camera.screen_height);
	}

	void PushShadows(Shader& shader, mat4& lightProjection) {
		shader.Use();
		glUniformMatrix4fv(glGetUniformLocation(shader.id, "lightProjection"), 1, GL_FALSE, value_ptr(lightProjection));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glUniform1i(glGetUniformLocation(shader.id, "shadowMap"), 1);
	}
	~ShadowMap() {
		glDeleteTextures(1, &shadowMap);
		glDeleteFramebuffers(1, &shadowMapFBO);
	}
};

int main(int argc, char* argv[]) {
	GLFWwindow* window = InitOpenGL("OpenGL");
#if GREADY_MESHING_ENABLED
	Shader voxel_shader("shaders/voxel_gm_vert.glsl", "shaders/voxel_frag.glsl");
#else
	Shader voxel_shader("shaders/voxel_vert.glsl", "shaders/voxel_frag.glsl");
#endif
	Shader mesh_shader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
	Shader rope_shader("shaders/rope_vert.glsl", "shaders/rope_frag.glsl");
	Shader water_shader("shaders/water_vert.glsl", "shaders/water_frag.glsl");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader skybox_shader("shaders/skybox_vert.glsl", "shaders/skybox_frag.glsl");
	Shader shadowmap_shader("shaders/shadowmap_vert.glsl", "shaders/shadowmap_frag.glsl");

	ShadowMap shadow_map;
	Skybox skybox(skybox_shader, (float)WINDOW_WIDTH / WINDOW_HEIGHT);
	camera.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, vec3(0, 2.5, 10));

	vec3 lightPos = vec3(40, 180, -165);
	voxel_shader.Use();
	glUniform3fv(glGetUniformLocation(voxel_shader.id, "lightpos"), 1, value_ptr(lightPos));
	mesh_shader.Use();
	glUniform3fv(glGetUniformLocation(mesh_shader.id, "lightpos"), 1, value_ptr(lightPos));
	water_shader.Use();
	glUniform3fv(glGetUniformLocation(water_shader.id, "lightpos"), 1, value_ptr(lightPos));
	voxbox_shader.Use();
	glUniform3fv(glGetUniformLocation(voxbox_shader.id, "lightpos"), 1, value_ptr(lightPos));

	mat4 orthgonalProjection = ortho(-45.0f, 45.0f, -45.0f, 45.0f, 0.1f, 500.0f);
	mat4 lightView = lookAt(0.25f * lightPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 lightProjection = orthgonalProjection * lightView;
	shadowmap_shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shadowmap_shader.id, "lightProjection"), 1, GL_FALSE, value_ptr(lightProjection));

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
	vector<Texture> glass_textures = {
		Texture("meshes/glass.png", "diffuse", 0),
	};
	Mesh glass("meshes/CAT_140M3_glass.obj", glass_textures);
	vector<Texture> model_textures = {
		Texture("meshes/CAT_140M3.png", "diffuse", 0),
		Texture("meshes/CAT_140M3_specular.png", "specular", 1),
		//Texture("meshes/CAT_140M3_normal.png", "normal", 2),
	};
	Mesh model("meshes/CAT_140M3.obj", model_textures);
*/
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

	glfwSetKeyCallback(window, key_press_callback);

	while (!glfwWindowShouldClose(window)) {
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

		shadow_map.BindShadowMap();
		scene.draw(shadowmap_shader, camera);
		//model.draw(shadowmap_shader, camera, vec3(12, 4.3, 30), angleAxis(radians(170.0f), vec3(0, 1, 0)));
		shadow_map.UnbindShadowMap(camera);

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			printf("Camera position: (%.2f, %.2f, %.2f)\r", camera.position.x, camera.position.y, camera.position.z);

		camera.handleInputs(window);
		camera.updateMatrix(45, 0.1, FAR_PLANE);

		//glass.draw(mesh_shader, camera, vec3(12, 4.3, 30), angleAxis(radians(170.0f), vec3(0, 1, 0)));
		//model.draw(mesh_shader, camera, vec3(12, 4.3, 30), angleAxis(radians(170.0f), vec3(0, 1, 0)));

		shadow_map.PushShadows(voxel_shader, lightProjection);
		scene.draw(voxel_shader, camera);
		scene.drawVoxbox(voxbox_shader, camera);
		scene.drawRope(rope_shader, camera);
		glEnable(GL_BLEND);
		scene.drawWater(water_shader, camera);
		glDisable(GL_BLEND);

		skybox.Draw(skybox_shader, camera);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
