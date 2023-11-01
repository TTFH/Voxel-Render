#include "skybox.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../lib/stb_image.h"
#include "ebo.h"

static GLfloat cube_vertices[] = {
	 1,  1,  1,
	 1,  1, -1,
	 1, -1,  1,
	 1, -1, -1,
	-1,  1,  1,
	-1,  1, -1,
	-1, -1,  1,
	-1, -1, -1,
};

static GLuint cube_indices[] = {
	0, 2, 6,
	3, 7, 6,
	4, 6, 5,
	1, 3, 2,
	2, 3, 6,
	1, 5, 7,
	0, 4, 1,
	0, 6, 4,
	1, 7, 3,
	0, 1, 2,
	1, 4, 5,
	5, 6, 7,
};

void Skybox::ReloadTexture(const char* name) {
	if (texture != 0)
		glDeleteTextures(1, &texture);

	string facesCubemap[6] = {
		"skyboxes/" + string(name) + "/right.png",
		"skyboxes/" + string(name) + "/left.png",
		"skyboxes/" + string(name) + "/top.png",
		"skyboxes/" + string(name) + "/bottom.png",
		"skyboxes/" + string(name) + "/front.png",
		"skyboxes/" + string(name) + "/back.png",
	};

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++) {
		int width, height, channels = 3;
		uint8_t* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &channels, STBI_rgb);
		printf("Loading texture %s with %d channels\n", facesCubemap[i].c_str(), channels);
		if (data != NULL) {
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		} else
			printf("[ERROR] Failed to load texture: %s\n", facesCubemap[i].c_str());
	}
}

Skybox::Skybox(const char* name) {
	vao.Bind();
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
	ReloadTexture(name);
}

void Skybox::draw(Shader& shader, Camera& camera) {
	mat4 view = mat4(mat3(lookAt(camera.position, camera.position + camera.direction, camera.up)));
	mat4 projection = perspective(radians(45.0f), 16.0f / 9, 0.1f, 1000.0f);
	shader.PushMatrix("vpMatrix", projection * view);

	glDepthFunc(GL_LEQUAL);
	vao.Bind();
	shader.PushTextureCubeMap("skybox", texture, 0);
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
	glDepthFunc(GL_LESS);
}

Skybox::~Skybox() {
	glDeleteTextures(1, &texture);
}
