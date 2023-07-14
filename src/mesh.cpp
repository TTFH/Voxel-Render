#include "mesh.h"
#include <string.h>
#include <glm/gtc/type_ptr.hpp>

struct Triangle {
	int vertex_index[3];
	int normal_index[3];
	int tex_index[3];
};

void Mesh::LoadOBJ(const char* path) {
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> tex_coords;
	vector<Triangle> mesh;

	FILE* file = fopen(path, "r");
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

#ifdef _WIN32
	printf("Loaded %lld vertices, %lld normals, %lld tex_coords, %lld triangles\n",
		positions.size() - 1, normals.size() - 1, tex_coords.size() - 1, mesh.size() - 1);
#else
	printf("Loaded %ld vertices, %ld normals, %ld tex_coords, %ld triangles\n",
		positions.size() - 1, normals.size() - 1, tex_coords.size() - 1, mesh.size() - 1);
#endif

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
		for (int v = 2; v >= 0; v--) { // Convert to CCW
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

Mesh::Mesh(const char* path, vector<Texture>& textures) {
	string extension = path;
	extension = extension.substr(extension.find_last_of(".") + 1);
	if (extension == "obj")
		LoadOBJ(path);
	else {
		printf("[Warning] Unsupported mesh format: %s\n", extension.c_str());
		return;
	}
	this->textures = textures;

	vao.Bind();
	VBO vbo(vertices);
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)0);					 // Position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)(3 * sizeof(GLfloat))); // Normal
	vao.LinkAttrib(vbo, 2, 2, GL_FLOAT, sizeof(MeshVertex), (GLvoid*)(6 * sizeof(GLfloat))); // TexCoord
	vao.Unbind();
	vbo.Unbind();
}

void Mesh::draw(Shader& shader, Camera& camera, vec3 translation, float angle) {
	shader.Use();
	vao.Bind();

	unsigned int num_diffuse = 0;
	unsigned int num_specular = 0;
	unsigned int num_normal = 0;
	unsigned int num_displacement = 0;

	// Unbind missing texture
	GLuint texUnit = glGetUniformLocation(shader.id, "specular0");
	glUniform1i(texUnit, 0);

	for (unsigned int i = 0; i < textures.size(); i++) {
		string num;
		string type = textures[i].type;
		if (type == "diffuse")
			num = to_string(num_diffuse++);
		else if (type == "specular")
			num = to_string(num_specular++);
		else if (type == "normal")
			num = to_string(num_normal++);
		else if (type == "displacement")
			num = to_string(num_displacement++);
		const char* uniform = (type + num).c_str();
		textures[i].Bind(shader, uniform);
	}

	camera.pushMatrix(shader, "camera");
	glUniform3fv(glGetUniformLocation(shader.id, "camera_pos"), 1, value_ptr(camera.position));
	mat4 trans = translate(mat4(1.0f), translation);
	quat rotation = angleAxis(radians(angle), vec3(0, 1, 0));
	mat4 rot = mat4_cast(rotation);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "position"), 1, GL_FALSE, value_ptr(trans));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "rotation"), 1, GL_FALSE, value_ptr(rot));
	// When rendering to a shadow map, use the next flags to diferentiate between types of objects
	glUniform1f(glGetUniformLocation(shader.id, "scale"), 0); // Flag: not a voxel
	glUniform3f(glGetUniformLocation(shader.id, "size"), 0, 0, 0); // Flag: not a prism
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}
