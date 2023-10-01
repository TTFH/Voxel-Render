#include "ebo.h"
#include "utils.h"
#include "greedy_mesh.h"

// Based on: https://0fps.net/2012/07/07/meshing-minecraft-part-2/
GreedyMesh generateGreedyMesh(const MV_Shape& shape) {
	uint8_t*** voxels = MatrixInit(shape);
	//TrimShape(voxels, shape.sizex, shape.sizey, shape.sizez);

	auto GetVoxelAt = [&](int x, int y, int z) -> uint8_t {
		if (x < 0 || x >= shape.sizex || y < 0 || y >= shape.sizey || z < 0 || z >= shape.sizez) {
			printf("[ERROR] Out of bounds.");
			return 0;
		}
		return voxels[x][y][z];
	};

	GreedyMesh mesh;
	int dims[3] = { shape.sizex, shape.sizey, shape.sizez };

	// For each axis
	for (int d = 0; d < 3; d++) {
		int u = (d + 1) % 3; // index axis u
		int v = (d + 2) % 3; // index axis v
		int x[3] = { 0, 0, 0 }; // current voxel
		int q[3] = { 0, 0, 0 }; q[d] = 1; // next voxel
		int16_t* mask = new int16_t[dims[u] * dims[v]]; // 2D slice

		// For each slice
		for (x[d] = -1; x[d] < dims[d]; ) {
			int n = 0; // slice index (linear)
			for (x[v] = 0; x[v] < dims[v]; x[v]++) {
				for (x[u] = 0; x[u] < dims[u]; x[u]++) {
					int16_t a = x[d] >= 0		   ? GetVoxelAt(x[0],		 x[1],		  x[2]		 ) : 0;
					int16_t b = x[d] < dims[d] - 1 ? GetVoxelAt(x[0] + q[0], x[1] + q[1], x[2] + q[2]) : 0;
					if ((a != 0) == (b != 0))
						mask[n] = 0;
					else if (a != 0)
						mask[n] = a;
					else
						mask[n] = -b;
					n++;
				}
			}
			x[d]++; n = 0;

			// For every block in slice
			for (int j = 0; j < dims[v]; j++) {
				for (int i = 0; i < dims[u];) {
					int16_t c = mask[n];
					if (c != 0) {
						int h, w;
						// Get width
						for (w = 1; i + w < dims[u] && mask[n + w] == c; w++) { }

						// Get height
						bool done = false;
						for (h = 1; j + h < dims[v]; h++) {
							for (int k = 0; k < w; k++) {
								// Stop if hole or different color
								if (mask[n + k + h * dims[u]] != c) {
									done = true; break;
								}
							}
							if (done) break;
						}

						x[u] = i; x[v] = j;

						// Size and orientation of this face
						int du[3] = { 0, 0, 0 };
						int dv[3] = { 0, 0, 0 };

						if (c < 0) {
							du[u] = w; dv[v] = h;
						} else {
							du[v] = h; dv[u] = w;
						}

						vec3 p0 = vec3(x[0],				 x[1],				   x[2]);
						vec3 p1 = vec3(x[0] + du[0],		 x[1] + du[1],		   x[2] + du[2]);
						vec3 p2 = vec3(x[0] + dv[0],		 x[1] + dv[1],		   x[2] + dv[2]);
						vec3 p3 = vec3(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2]);
						vec3 normal = -cross(p1 - p0, p2 - p0);
						uint8_t index = c > 0 ? c : -c;

						// Generate the indices first
						int start_index = mesh.vertices.size();
						mesh.indices.push_back(start_index + 1);
						mesh.indices.push_back(start_index + 2);
						mesh.indices.push_back(start_index + 3);
						mesh.indices.push_back(start_index + 2);
						mesh.indices.push_back(start_index + 1);
						mesh.indices.push_back(start_index + 0);

						mesh.vertices.push_back({ p0, normal, index });
						mesh.vertices.push_back({ p1, normal, index });
						mesh.vertices.push_back({ p2, normal, index });
						mesh.vertices.push_back({ p3, normal, index });

						// Clear part of the mask to avoid duplicated faces
						for (int l = 0; l < h; l++)
							for (int k = 0; k < w; k++)
								mask[n + k + l * dims[u]] = 0;

						i += w; n += w;
					} else {
						i++; n++;
					}
				}
			}
		}
		delete[] mask;
	}

	MatrixDelete(voxels, shape);
	return mesh;
}

GreedyRender::GreedyRender(const MV_Shape& shape, GLuint texture_id) {
	GreedyMesh mesh = generateGreedyMesh(shape);
	this->texture_id = texture_id;
	index_count = mesh.indices.size();

	vao.Bind();
	VBO vbo(mesh.vertices);
	EBO ebo(mesh.indices);

	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(GM_Vertex), (GLvoid*)0);								// Vertex position
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(GM_Vertex), (GLvoid*)(3 * sizeof(GLfloat)));			// Normal
	vao.LinkAttrib(vbo, 2, 1, GL_UNSIGNED_BYTE, sizeof(GM_Vertex), (GLvoid*)(6 * sizeof(GLfloat))); // Texture coord

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void GreedyRender::setTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void GreedyRender::setWorldTransform(vec3 position, quat rotation) {
	this->world_position = position;
	this->world_rotation = rotation;
}

void GreedyRender::draw(Shader& shader, Camera& camera, float scale) {
	vao.Bind();
	shader.PushMatrix("camera", camera.vpMatrix);
	shader.PushFloat("scale", scale);

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.PushMatrix("position", pos);
	shader.PushMatrix("rotation", rot);

	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	shader.PushMatrix("world_pos", world_pos);
	shader.PushMatrix("world_rot", world_rot);

	shader.PushTexture1D("palette", texture_id, 0);

	//glLineWidth(5.0f);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	vao.Unbind();
}
