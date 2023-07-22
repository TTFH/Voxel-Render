#include <string.h>
#include "texture.h"
#include "../lib/stb_image.h"

Texture::Texture(GLuint tex_id, GLuint slot) {
	unit = slot;
	texture_id = tex_id;
}

Texture::Texture(const char* path, const char* texType, GLuint slot) {
	unit = slot;
	type = texType;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	uint8_t* data = stbi_load(path, &width, &height, &channels, STBI_default);
	printf("Loading texture %s with %d channels\n", path, channels);

	glGenTextures(1, &texture_id);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP_TO_BORDER
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float clampColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	GLenum format = GL_RGB;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 4)
		format = GL_RGBA;

	// diffuse
	GLenum internalformat = GL_RGBA; // GL_SRGB_ALPHA
	if (strcmp(texType, "specular") == 0)
		internalformat = GL_RED;
	else if (strcmp(texType, "dudv") == 0)
		internalformat = GL_RGB;
	else if (strcmp(texType, "normal") == 0)
		internalformat = GL_RGB;
	else if (strcmp(texType, "displacement") == 0)
		internalformat = GL_RED;

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(Shader& shader, const char* uniform) {
	shader.Use();
	glUniform1i(glGetUniformLocation(shader.id, uniform), unit);

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

void Texture::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	//glDeleteTextures(1, &texture_id);
}
