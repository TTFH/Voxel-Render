#include <map>
#include <array>
#include <vector>
#include <stdint.h>
#include <string.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "src/vbo.h"
#include "src/ebo.h"
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

static const GLfloat screen_vertices[] = {
	// pos  uv
	-1, -1, 0, 0,
	 1, -1, 1, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,
};

static const GLuint screen_indices[] = {
	0, 1, 2,
	1, 3, 2,
};

class SimpleScreen {
private:
	VAO vao;
	GLuint framebuffer;
	GLuint texture;
	vec2 position;
	vec2 size;
public:
	SimpleScreen(vec2 position, vec2 size) : position(position), size(size) {
		VBO vbo(screen_vertices, sizeof(screen_vertices));
		EBO ebo(screen_indices, sizeof(screen_indices));
		vao.LinkAttrib(0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
		vao.LinkAttrib(1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		vao.Unbind();
		vbo.Unbind();
		ebo.Unbind();

		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
			printf("[ERROR] Framebuffer failed with status %d\n", fboStatus);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void start() {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void end() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void draw(Shader& shader) {
		shader.PushVec2("uPosition", position);
		shader.PushVec2("uSize", size);
		shader.PushTexture2D("uTexture", texture, 0);

		vao.Bind();
		glDrawElements(GL_TRIANGLES, sizeof(screen_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		vao.Unbind();
	}
};

bool intersectPlanes(const Plane& p1, const Plane& p2, const Plane& p3, vec3& outPoint) {
	vec3 n1 = p1.normal;
	vec3 n2 = p2.normal;
	vec3 n3 = p3.normal;
	outPoint = vec3(0, 0, 0);
	float denom = dot(n1, cross(n2, n3));
	if (abs(denom) < 1e-6f)
		return false;

	vec3 result =
		(-p1.distance * cross(n2, n3) -
		p2.distance * cross(n3, n1) -
		p3.distance * cross(n1, n2)) / denom;

	outPoint = result;
	return true;
}

vector<vec3> getFrustumCorners(const Frustum& frustum) {
	vec3 point;
	vector<vec3> corners;
	corners.reserve(8);
	intersectPlanes(frustum.left, frustum.bottom, frustum.near, point); corners.push_back(point);
	intersectPlanes(frustum.left, frustum.top, frustum.near, point); corners.push_back(point);
	intersectPlanes(frustum.right, frustum.top, frustum.near, point); corners.push_back(point);
	intersectPlanes(frustum.right, frustum.bottom, frustum.near, point); corners.push_back(point);
	intersectPlanes(frustum.left, frustum.bottom, frustum.far, point); corners.push_back(point);
	intersectPlanes(frustum.left, frustum.top, frustum.far, point); corners.push_back(point);
	intersectPlanes(frustum.right, frustum.top, frustum.far, point); corners.push_back(point);
	intersectPlanes(frustum.right, frustum.bottom, frustum.far, point); corners.push_back(point);
	return corners;
}

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("OpenGL Test");
	//Shader screen_shader("screen");
	Shader voxel_rtx_shader("editorvox");
	Shader voxbox_shader("shaders/voxbox_vert.glsl", "shaders/voxbox_frag.glsl");
	Shader rope_shader("shaders/rope_vert.glsl", "shaders/rope_frag.glsl");

	/*SimpleScreen left(vec2(-0.5, 0), vec2(0.5, 1));
	SimpleScreen right(vec2(0.5, 0), vec2(0.5, 1));*/

	Camera camera1(vec3(0, 1.8, 10));
	//camera1.updateScreenSize(WINDOW_WIDTH / 2, WINDOW_HEIGHT);
	//Camera camera2(vec3(2.5, 2.5, 10));
	//camera2.updateScreenSize(WINDOW_WIDTH / 2, WINDOW_HEIGHT);

	array<array<int, 4>, 6> face_indices = {{
		{0, 1, 2, 3}, // Near
		{4, 5, 6, 7}, // Far
		{0, 1, 5, 4}, // Left
		{3, 2, 6, 7}, // Right
		{1, 2, 6, 5}, // Top
		{0, 3, 7, 4}, // Bottom
	}};

	Frustum frustum;
	vector<RopeRender*> debug_lines;
	Light light(vec3(30, 40, 50));
	VoxboxRender cube(vec3(10, 10, 10), vec3(0, 0, 1));
	cube.setWorldTransform(vec3(-0.5, 0, -0.5), quat(1, 0, 0, 0));


	VoxLoader vox_file("light.vox");
	RTX_Render shape(vox_file.shapes[0], vox_file.palette_id);
	shape.setWorldTransform(vec3(-0.5, 10, -0.5), quat(1, 0, 0, 0));

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
		camera1.handleInputs(window);
		light.handleInputs(window);

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			for (vector<RopeRender*>::iterator it = debug_lines.begin(); it != debug_lines.end(); it++)
				delete *it;
			debug_lines.clear();
			camera1.updateFarPlane(15);
			frustum = camera1.getFrustum();
			vector<vec3> corners = getFrustumCorners(frustum);
			for (int i = 0; i < 6; i++) {
				vector<vec3> face_vertices;
				for (int j = 0; j < 4; j++)
					face_vertices.push_back(corners[face_indices[i][j]]);
				face_vertices.push_back(corners[face_indices[i][0]]);
				debug_lines.push_back(new RopeRender(face_vertices, vec3((i & 4) >> 2, (i & 2) >> 1, i & 1)));
			}
			camera1.updateFarPlane(500);

			vector<vec3> cube_corners = shape.getOBBCorners();
			debug_lines.push_back(new RopeRender(cube_corners, vec3(1, 1, 1)));
		}
		if (shape.isInFrustum(frustum))
			cube.setColor(vec3(0, 1, 0));
		else
			cube.setColor(vec3(1, 0, 0));

		/*left.start();
		voxbox_shader.Use();
		light.pushUniforms(voxbox_shader);
		cube.draw(voxbox_shader, camera1);
		left.end();

		right.start();
		voxbox_shader.Use();
		light.pushUniforms(voxbox_shader);
		cube.draw(voxbox_shader, camera2);
		rope_shader.Use();
		for (int i = 0; i < 6; i++)
			faces[i]->draw(rope_shader, camera2);
		right.end();*/

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*screen_shader.Use();
		left.draw(screen_shader);
		right.draw(screen_shader);*/

		voxel_rtx_shader.Use();
		shape.draw(voxel_rtx_shader, camera1);

		voxbox_shader.Use();
		light.pushUniforms(voxbox_shader);
		cube.draw(voxbox_shader, camera1);

		rope_shader.Use();
		for (vector<RopeRender*>::iterator it = debug_lines.begin(); it != debug_lines.end(); it++)
			(*it)->draw(rope_shader, camera1);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
