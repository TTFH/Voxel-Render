#include "skybox.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../lib/stb_image.h"

Skybox::Skybox(Shader& shader, float aspectRatio) {
	this->aspectRatio = aspectRatio;

	GLuint skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

/*
	./cmft --input "day.dds" --output0 "cmft_facelist" --output0params tga,bgra8,facelist
	Convert tga to png
	Rename:
		neg_x	left
		neg_y	bottom
		neg_z	back
		pos_x	right
		pos_y	top
		pos_z	front
	Flip vertically
	Apply gamma correction in shader
*/

	const char* facesCubemap[6] = {
		"skybox/right.png",
		"skybox/left.png",
		"skybox/top.png",
		"skybox/bottom.png",
		"skybox/front.png",
		"skybox/back.png",
	};

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++) {
		int width, height, channels = 3;
		uint8_t* data = stbi_load(facesCubemap[i], &width, &height, &channels, STBI_rgb);
		//printf("Loading texture %s with %d channels\n", facesCubemap[i], channels);
		if (data){
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		} else {
			printf("[ERROR] Failed to load texture: %s\n", facesCubemap[i]);
			stbi_image_free(data);
		}
	}

	shader.Use();
	glUniform1i(glGetUniformLocation(shader.id, "skybox"), 0);
}

void Skybox::Draw(Shader& shader, Camera& camera) {
	glDepthFunc(GL_LEQUAL);
	shader.Use();
	mat4 view = mat4(mat3(lookAt(camera.position, camera.position + camera.orientation, camera.up)));
	mat4 projection = perspective(radians(45.0f), aspectRatio, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "view"), 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "projection"), 1, GL_FALSE, value_ptr(projection));

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}
