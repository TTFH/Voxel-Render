#include <string.h>

#include "render_mesh.h"
#include "utils.h"

struct Triangle {
	int vertex_index[3];
	int normal_index[3];
	int tex_index[3];
};

void Mesh::LoadSimpleOBJ(const char* path) {
	vector<vec3> positions;
	vector<vec3> normals;
	vector<Triangle> mesh;

	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("[Warning] File not found: %s\n", path);
		return;
	}

	positions.push_back({0, 0, 0});
	normals.push_back({0, 0, 0});
	do {
		char line[128];
		fscanf(file, "%s", line);
		if (strcmp(line, "v") == 0) {
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			positions.push_back(vertex);
		} else if (strcmp(line, "vn") == 0) {
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normals.push_back(normal);
		} else if (strcmp(line, "f") == 0) {
			Triangle triangle;
			fscanf(file, "%d//%d %d//%d %d//%d\n",
				&triangle.vertex_index[0], &triangle.normal_index[0],
				&triangle.vertex_index[1], &triangle.normal_index[1],
				&triangle.vertex_index[2], &triangle.normal_index[2]);
			mesh.push_back(triangle);
		} else {
			char buffer[256];
			fscanf(file, "%[^\n]\n", buffer);
		}
	} while (!feof(file));
	fclose(file);

	for (unsigned int t = 0; t < mesh.size(); t++) {
		for (int v = 0; v < 3; v++) {
			vec3 vert = positions[mesh[t].vertex_index[v]];
			vec3 norm = normals[mesh[t].normal_index[v]];
			MeshVertex vertex = {
				vec3(vert.x, vert.y, vert.z),
				vec3(norm.x, norm.y, norm.z),
				vec2(0, 0),
			};
			vertices.push_back(vertex);
		}
	}
}

void Mesh::LoadOBJ(const char* path) {
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> tex_coords;
	vector<Triangle> mesh;

	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("[Warning] File not found: %s\n", path);
		return;
	}

	// OBJ indices start at 1
	positions.push_back({0, 0, 0});
	normals.push_back({0, 0, 0});
	tex_coords.push_back({0, 0});
	do {
		char line[128];
		fscanf(file, "%s", line);
		if (strcmp(line, "v") == 0) {
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			positions.push_back(vertex);
		} else if (strcmp(line, "vn") == 0) {
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normals.push_back(normal);
		} else if (strcmp(line, "vt") == 0) {
			vec2 tex_coord;
			fscanf(file, "%f %f\n", &tex_coord.x, &tex_coord.y);
			tex_coords.push_back(tex_coord);
		} else if (strcmp(line, "f") == 0) {
			Triangle triangle;
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&triangle.vertex_index[0], &triangle.tex_index[0], &triangle.normal_index[0],
				&triangle.vertex_index[1], &triangle.tex_index[1], &triangle.normal_index[1],
				&triangle.vertex_index[2], &triangle.tex_index[2], &triangle.normal_index[2]);
			mesh.push_back(triangle);
		} else {
			char buffer[256];
			fscanf(file, "%[^\n]\n", buffer);
		}
	} while (!feof(file));
	fclose(file);

	printf("Loaded %d vertices, %d normals, %d tex_coords, %d triangles\n",
		(int)positions.size() - 1, (int)normals.size() - 1, (int)tex_coords.size() - 1, (int)mesh.size());

	// Convert negative indices to positive
	for (unsigned int t = 0; t < mesh.size(); t++) {
		for (unsigned int v = 0; v < 3; v++) {
			if (mesh[t].vertex_index[v] < 0)
				mesh[t].vertex_index[v] += positions.size();
			if (mesh[t].normal_index[v] < 0)
				mesh[t].normal_index[v] += normals.size();
			if (mesh[t].tex_index[v] < 0)
				mesh[t].tex_index[v] += tex_coords.size();
		}
	}

	for (unsigned int t = 0; t < mesh.size(); t++) {
		for (int v = 0; v < 3; v++) {
			vec3 vert = positions[mesh[t].vertex_index[v]];
			vec3 norm = normals[mesh[t].normal_index[v]];
			vec2 tex = tex_coords[mesh[t].tex_index[v]];
			MeshVertex vertex = {
				vec3(vert.x, vert.y, vert.z),
				vec3(norm.x, norm.y, norm.z),
				vec2(tex.x, tex.y),
			};
			vertices.push_back(vertex);
		}
	}
}

void Mesh::SaveOBJ(const char* path) {
	FILE* output = fopen(path, "w");
	if (output == NULL) {
		printf("[Warning] Could not open file %s\n", path);
		return;
	}
	for (unsigned int i = 0; i < vertices.size(); i++)
		fprintf(output, "v %f %f %f\n", vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
	for (unsigned int i = 0; i < vertices.size(); i++)
		fprintf(output, "vn %f %f %f\n", vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z);
	for (unsigned int i = 0; i < vertices.size(); i++)
		fprintf(output, "vt %f %f\n", vertices[i].tex_coord.x, vertices[i].tex_coord.y);
	for (unsigned int i = 0; i < vertices.size(); i += 3)
		fprintf(output, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", i + 1, i + 1, i + 1, i + 2, i + 2, i + 2, i + 3, i + 3, i + 3);
	fclose(output);
}

Mesh::Mesh(const char* path) {
	LoadOBJ(path);

	vao.Bind();
	VBO vbo(vertices);
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)0);					 // Position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)(3 * sizeof(GLfloat))); // Normal
	vao.LinkAttrib(vbo, 2, 2, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)(6 * sizeof(GLfloat))); // TexCoord
	vao.Unbind();
	vbo.Unbind();
}

Mesh::Mesh(const char* path, vec3 color) {
	this->color = color;
	LoadSimpleOBJ(path);

	vao.Bind();
	VBO vbo(vertices);
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)0);					 // Position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)(3 * sizeof(GLfloat))); // Normal
	vao.LinkAttrib(vbo, 2, 2, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)(6 * sizeof(GLfloat))); // TexCoord
	vao.Unbind();
	vbo.Unbind();
}

void Mesh::addTexture(const char* path) {
	GLuint texture_id = LoadTexture2D(path);
	textures.push_back(texture_id);
}

void Mesh::handleInputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		position.y += 0.025f;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		position.y -= 0.025f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		position += vec3(rotation * vec4(0, 0, 0.1f, 0));
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		position += vec3(rotation * vec4(0, 0, -0.05f, 0));
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		rotation = angleAxis(radians(-0.5f), vec3(0, 1, 0)) * rotation;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		rotation = angleAxis(radians(0.5f), vec3(0, 1, 0)) * rotation;
}

void Mesh::setWorldTransform(vec3 position, float angle) {
	this->position = position;
	this->rotation = angleAxis(radians(angle), vec3(0, 1, 0));
}

void Mesh::setWorldTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void Mesh::draw(Shader& shader, Camera& camera) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);
	shader.PushVec3("camera_pos", camera.position);

	shader.PushFloat("scale", 0); // SM flag not a voxel
	shader.PushVec3("size", vec3(0, 0, 0)); // SM flag not a voxagon
	shader.PushVec3("color", color); // No texture

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.PushMatrix("position", pos);
	shader.PushMatrix("rotation", rot);

	for (unsigned int i = 0; i < textures.size(); i++) {
		char name[16];
		sprintf(name, "tex%d", i);
		shader.PushTexture2D(name, textures[i], i + 1); // Texture 0 is SM
	}

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	vao.Unbind();
}
