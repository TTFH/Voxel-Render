#include <stdexcept>
#include <unordered_map>

#include "ebo.h"
#include "vbo.h"
#include "utils.h"
#include "render_vox_greedy.h"

/*
                                             __----~~~~~~~~~~~------___
                                  .  .   ~~//====......          __--~ ~~
                  -.            \_|//     |||\\  ~~~~~~::::... /~
               ___-==_       _-~o~  \/    |||  \\            _/~~-
       __---~~~.==~||\=_    -_--~/_-~|-   |\\   \\        _/~
   _-~~     .=~    |  \\-_    '-~7  /-   /  ||    \      /
 .~       .~       |   \\ -_    /  /-   /   ||      \   /
/  ____  /         |     \\ ~-_/  /|- _/   .||       \ /
|~~    ~~|--~~~~--_ \     ~==-/   | \~--===~~        .\
         '         ~-|      /|    |-~\~~       __--~~
                     |-~~-_/ |    |   ~\_   _-~            /\
                          /  \     \__   \/~                \__
                      _--~ _/ | .-~~____--~-/                  ~~==.
                     ((->/~   '.|||' -_|    ~~-/ ,              . _||
                                -_     ~\      ~~---l__i__i__i--~~_/
                                _-~-__   ~)  \--______________--~~
                              //.-~~~-~_--~- |-------~~~~~~~~
                                     //.-~~~--\
   _  _ ___ ___ ___   ___ ___   ___  ___    _   ___  ___  _  _ ___ 
  | || | __| _ \ __| | _ ) __| |   \| _ \  /_\ / __|/ _ \| \| / __|
  | __ | _||   / _|  | _ \ _|  | |) |   / / _ \ (_ | (_) | .` \__ \
  |_||_|___|_|_\___| |___/___| |___/|_|_\/_/ \_\___|\___/|_|\_|___/
*/

struct GM_Vertex {
	vec3 position;
	vec3 normal;
	uint8_t index;
};

class GreedyMesh {
private:
	int sizex, sizey, sizez;
	vector<GM_Vertex> vertices;
	vector<GLuint> indices;
	unordered_map<tuple<uint8_t, uint8_t, uint8_t>, uint8_t, VoxelHash> voxels;
	void ComputeMesh();
	uint8_t GetVoxelAt(int x, int y, int z);
	void AddFace(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec3 normal, uint8_t index);
public:
	const vector<GM_Vertex>& getVertices() const;
	const vector<GLuint>& getIndices() const;
	GreedyMesh(const MV_Shape& shape);
	void SaveOBJ(string path, int palette_id) const;
};

const vector<GM_Vertex>& GreedyMesh::getVertices() const {
	return vertices;
}

const vector<GLuint>& GreedyMesh::getIndices() const {
	return indices;
}

uint8_t GreedyMesh::GetVoxelAt(int x, int y, int z) {
	if (x < 0 || x >= sizex || y < 0 || y >= sizey || z < 0 || z >= sizez)
		throw out_of_range("Voxel position out of range");

	const tuple<uint8_t, uint8_t, uint8_t> voxel_pos = make_tuple(x, y, z);
	if (voxels.find(voxel_pos) == voxels.end())
		return 0;
	return voxels[voxel_pos];
}

void GreedyMesh::AddFace(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec3 normal, uint8_t index) {
	int start_index = vertices.size();
	indices.push_back(start_index + 1);
	indices.push_back(start_index + 2);
	indices.push_back(start_index + 3);

	indices.push_back(start_index + 2);
	indices.push_back(start_index + 1);
	indices.push_back(start_index + 0);

	vertices.push_back({ p0, normal, index });
	vertices.push_back({ p1, normal, index });
	vertices.push_back({ p2, normal, index });
	vertices.push_back({ p3, normal, index });
}

GreedyMesh::GreedyMesh(const MV_Shape& shape) {
	for (vector<MV_Voxel>::const_iterator it = shape.voxels.begin(); it != shape.voxels.end(); it++) {
		const MV_Voxel& voxel = *it;
		if (voxel.index != HOLE_INDEX)
			voxels.insert({ make_tuple(voxel.x, voxel.y, voxel.z), voxel.index });
	}
	sizex = shape.sizex;
	sizey = shape.sizey;
	sizez = shape.sizez;
	ComputeMesh();
}

// Based on: https://0fps.net/2012/07/07/meshing-minecraft-part-2/
// https://github.com/mikolalysenko/mikolalysenko.github.com/blob/gh-pages/MinecraftMeshes/js/greedy.js
void GreedyMesh::ComputeMesh() {
	int dims[3] = { sizex, sizey, sizez };

	// For each axis
	for (int d = 0; d < 3; d++) {
		int u = (d + 1) % 3; // index axis u
		int v = (d + 2) % 3; // index axis v
		int x[3] = { 0, 0, 0 }; // current voxel
		int q[3] = { 0, 0, 0 }; q[d] = 1; // next voxel
		int16_t* mask = new int16_t[dims[u] * dims[v]]; // 2D slice for index and direction

		// For each slice
		for (x[d] = -1; x[d] < dims[d];) {
			int n = 0; // slice index
			for (x[v] = 0; x[v] < dims[v]; x[v]++) {
				for (x[u] = 0; x[u] < dims[u]; x[u]++) {
					int16_t a = x[d] >= 0		   ? GetVoxelAt(x[0],		 x[1],		  x[2]		 ) : 0;
					int16_t b = x[d] < dims[d] - 1 ? GetVoxelAt(x[0] + q[0], x[1] + q[1], x[2] + q[2]) : 0;
					if (!(a ^ b)) // Told you so, those dragons are scary
						mask[n] = 0;
					else
						mask[n] = a != 0 ? a : -b;
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
						normal = normalize(normal);
						uint8_t index = c > 0 ? c : -c;

						AddFace(p0, p1, p2, p3, normal, index);

						// Clear the used part of the mask
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
}

void GreedyMesh::SaveOBJ(string path, int palette_id) const {
	FILE* output = fopen(path.c_str(), "w");
	for (unsigned int i = 0; i < vertices.size(); i++)
		fprintf(output, "v %d %d %d\n", (int)vertices[i].position.x, (int)vertices[i].position.y, (int)vertices[i].position.z);
	for (unsigned int i = 0; i < vertices.size(); i++)
		fprintf(output, "vn %d %d %d\n", (int)vertices[i].normal.x, (int)vertices[i].normal.y, (int)vertices[i].normal.z);
	for (unsigned int i = 0; i < vertices.size(); i++) {
		float du = (vertices[i].index + 0.5f) / 256.0f;
		float dv = 1.0f - (palette_id + 0.5f) / (float)VoxRender::MAX_PALETTES;
		fprintf(output, "vt %.5f %.5f\n", du, dv);
	}
	for (unsigned int j = 0; j < indices.size(); j += 3) {
		int i0 = indices[j + 0] + 1;
		int i1 = indices[j + 1] + 1;
		int i2 = indices[j + 2] + 1;
		fprintf(output, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", i0, i0, i0, i1, i1, i1, i2, i2, i2);
	}
	fclose(output);
	printf("[INFO] Saved shape to %s\n", path.c_str());
}

GreedyRender::GreedyRender(const MV_Shape& shape, int palette_id) {
	GreedyMesh mesh(shape);
	this->palette_id = palette_id;
	index_count = mesh.getIndices().size();
	shape_size = vec3(shape.sizex, shape.sizey, shape.sizez);

#ifdef _BLENDER
	mesh.SaveOBJ(shape.id + ".obj", palette_id);
#endif

	VBO vbo(mesh.getVertices());
	EBO ebo(mesh.getIndices());

	vao.linkAttrib(0, 3, GL_FLOAT, sizeof(GM_Vertex), (GLvoid*)0);							   // Vertex position
	vao.linkAttrib(1, 3, GL_FLOAT, sizeof(GM_Vertex), (GLvoid*)(3 * sizeof(GLfloat)));		   // Normal
	vao.linkAttrib(2, 1, GL_UNSIGNED_BYTE, sizeof(GM_Vertex), (GLvoid*)(6 * sizeof(GLfloat))); // Texture coord

	vao.unbind();
	vbo.unbind();
	ebo.unbind();
}

void GreedyRender::draw(Shader& shader, Camera& camera) {
	shader.pushMatrix("camera", camera.vp_matrix);

	shader.pushFloat("scale", scale);
	shader.pushInt("side", 0); // SM flag not an hexagon
	shader.pushVec3("size", vec3(0, 0, 0)); // SM flag not a voxagon
	shader.pushTexture2D("uColor", paletteBank, 1); // Texture 0 is SM
	shader.pushInt("uPalette", palette_id);

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.pushMatrix("position", pos);
	shader.pushMatrix("rotation", rot);

	mat4 world_pos = translate(mat4(1.0f), world_position);
	mat4 world_rot = mat4_cast(world_rotation);
	shader.pushMatrix("world_pos", world_pos);
	shader.pushMatrix("world_rot", world_rot);

	vao.bind();
	//glLineWidth(5.0f); // GL_LINES
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	vao.unbind();
}
