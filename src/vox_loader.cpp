#include <stdio.h>

#include "vox_loader.h"
#include "hex_render.h"
#include "greedy_mesh.h"
#include "vox_rtx.h"
#include "utils.h"

constexpr int ID(char a, char b, char c, char d) {
	return a | (b << 8) | (c << 16) | (d << 24);
}

const int VOX  = ID('V', 'O', 'X', ' ');
const int MAIN = ID('M', 'A', 'I', 'N');
const int SIZE = ID('S', 'I', 'Z', 'E');
const int XYZI = ID('X', 'Y', 'Z', 'I');
const int TDCZ = ID('T', 'D', 'C', 'Z');
const int RGBA = ID('R', 'G', 'B', 'A');
const int nTRN = ID('n', 'T', 'R', 'N');
const int nSHP = ID('n', 'S', 'H', 'P');

const int IMAP = ID('I', 'M', 'A', 'P');
const int MATL = ID('M', 'A', 'T', 'L');
const int nGRP = ID('n', 'G', 'R', 'P');
const int LAYR = ID('L', 'A', 'Y', 'R');
const int rOBJ = ID('r', 'O', 'B', 'J');
const int rCAM = ID('r', 'C', 'A', 'M');
const int NOTE = ID('N', 'O', 'T', 'E');

typedef map<string, string> DICT;

struct Chunk {
	int id;
	int contentSize;
	int childrenSize;
	long int end;
};

int ReadInt(FILE* file) {
	int val = 0;
	fread(&val, sizeof(int), 1, file);
	return val;
}

void ReadChunk(FILE* file, Chunk& chunk) {
	chunk.id = ReadInt(file);
	chunk.contentSize = ReadInt(file);
	chunk.childrenSize = ReadInt(file);
	chunk.end = ftell(file) + chunk.contentSize + chunk.childrenSize;
}

string ReadString(FILE* file) {
	int size = ReadInt(file);
	char* buffer = new char[size];
	fread(buffer, sizeof(uint8_t), size, file);
	string res(buffer, buffer + size);
	delete[] buffer;
	return res;
}

DICT ReadDict(FILE* file) {
	DICT dict;
	int dict_entries = ReadInt(file);
	for (int i = 0; i < dict_entries; i++) {
		string key = ReadString(file);
		string value = ReadString(file);
		dict[key] = value;
	}
	return dict;
}

string GetDictValue(DICT& dict, string key) {
	DICT::iterator it = dict.find(key);
	if (it != dict.end()) {
		return it->second;
	}
	return "";
}

int ReadHeader(FILE* file) {
	int magic = ReadInt(file);
	if (magic != VOX) {
		printf("[ERROR] Invalid .vox file format.\n");
		exit(EXIT_FAILURE);
	}

	int version = ReadInt(file);
	if (version != 150 && version != 200) {
		printf("[ERROR] Invalid MV version.\n");
		exit(EXIT_FAILURE);
	}

	Chunk mainChunk;
	ReadChunk(file, mainChunk);
	if (mainChunk.id != MAIN) {
		printf("[ERROR] MV Main chunk not found.\n");
		exit(EXIT_FAILURE);
	}
	if (mainChunk.contentSize != 0) {
		printf("[ERROR] MV Main chunk content size is not zero.\n");
		exit(EXIT_FAILURE);
	}
	return mainChunk.end;
}

VoxLoader::VoxLoader() { }

VoxLoader::VoxLoader(const char* filename) {
	load(filename);
}

static const int MAX_PALETTES = 1024;
static int paletteCount = 0;
static GLuint paletteBank;

static string RemoveExtension(const string& path) {
	size_t last_dot = path.find_last_of(".");
	if (last_dot == string::npos) return path;
	return path.substr(0, last_dot);
}

void VoxLoader::load(const char* filename) {
	this->filename = filename;
	vector<MV_Voxel> voxels;
	int sizex = 0;
	int sizey = 0;
	int sizez = 0;
	int ref_model_id = -1;
	mv_model_iterator last_inserted = models.end();

	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		printf("[Warning] File %s not found.\n", filename);
		return;
	}
	int file_size = ReadHeader(file);

	while (ftell(file) < file_size) {
		Chunk sub;
		ReadChunk(file, sub);
		switch (sub.id) {
		case SIZE:
			sizex = ReadInt(file);
			sizey = ReadInt(file);
			sizez = ReadInt(file);
			break;
		case XYZI: {
				int numVoxels = ReadInt(file);
				if (numVoxels > 0) {
					voxels.resize(numVoxels);
					fread(voxels.data(), sizeof(MV_Voxel), numVoxels, file);
				} else
					voxels.clear();
				string id = RemoveExtension(filename) + "_" + to_string(shapes.size());
				MV_Shape shape = { id, sizex, sizey, sizez, voxels };
				shapes.push_back(shape);
				//printf("XYZI shape[%d]: %5d voxels, size = [%3d %3d %3d]\n", (int)shapes.size() - 1, numVoxels, sizex, sizey, sizez);
			}
			break;
		case RGBA:
			fread(palette + 1, sizeof(MV_Diffuse), 255, file);
			fread(&palette, sizeof(MV_Diffuse), 1, file);
			break;
		case nTRN: {
				int node_id = ReadInt(file);
				if (node_id == 0)
					break;
				vec3 position = { 0, 0, 0 };
				mat3 rot_matrix = mat3(1.0f);
				string shape_name = "";

				DICT nodeAttribs = ReadDict(file);
				if (nodeAttribs.find("_name") != nodeAttribs.end())
					shape_name = nodeAttribs["_name"];

				ReadInt(file); ReadInt(file); ReadInt(file);
				int num_frames = ReadInt(file);
				for (int i = 0; i < num_frames; i++) {
					DICT frames = ReadDict(file);
					for (DICT::iterator it = frames.begin(); it != frames.end(); it++) {
						if (it->first == "_t") {
							size_t end1, end2;
							string pos = it->second;
							position[0] = stoi(pos, &end1);
							position[1] = stoi(pos.substr(end1), &end2);
							position[2] = stoi(pos.substr(end1 + end2));
							//printf("Position: %g %g %g\n", position[0], position[1], position[2]);
						} else if (it->first == "_r") {
							int v = stoi(it->second);
							int x = (v >> 0) & 3;
							int y = (v >> 2) & 3;
							int z = 3 - x - y;
							rot_matrix = mat3(0.0f);
							rot_matrix[x][0] = (v >> 4) & 1 ? -1 : 1;
							rot_matrix[y][1] = (v >> 5) & 1 ? -1 : 1;
							rot_matrix[z][2] = (v >> 6) & 1 ? -1 : 1;
							/*printf("Rot byte: 0x%02X\n", v);
							printf("    [%2d %2d %2d]\nR = [%2d %2d %2d]\n    [%2d %2d %2d]\n\n",
								(int)rot_matrix[0][0], (int)rot_matrix[0][1], (int)rot_matrix[0][2],
								(int)rot_matrix[1][0], (int)rot_matrix[1][1], (int)rot_matrix[1][2],
								(int)rot_matrix[2][0], (int)rot_matrix[2][1], (int)rot_matrix[2][2]);*/
						}
					}
				}
				MV_Model data = { ref_model_id, position, quat_cast(rot_matrix) };
				last_inserted = models.emplace_hint(last_inserted, shape_name, data);
			}
			break;
		case nSHP: {
				ReadInt(file);
				ReadDict(file);
				int num_models = ReadInt(file);
				for (int i = 0; i < num_models; i++) {
					ref_model_id = ReadInt(file);
					last_inserted->second.shape_index = ref_model_id;
					ReadDict(file);
				}
			}
			break;
		case MATL: {
				int material_id = ReadInt(file);
				DICT mat_properties = ReadDict(file);
				if (GetDictValue(mat_properties, "_type") == "_glass" && GetDictValue(mat_properties, "_alpha") != "1.0")
					palette[material_id % 256].a = 0.5;
			}
			break;
		default:
			break;
		}
		fseek(file, sub.end, SEEK_SET);
	}
	fclose(file);
	//printf("File loaded: %s\n", filename);

	if (paletteCount == 0) { // Create texture
		glGenTextures(1, &paletteBank);
		glBindTexture(GL_TEXTURE_2D, paletteBank);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, MAX_PALETTES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	} else
		glBindTexture(GL_TEXTURE_2D, paletteBank);
	if (paletteCount < MAX_PALETTES)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, paletteCount, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, palette);
	else if (paletteCount == MAX_PALETTES)
		printf("[Warning] Palette limit reached!\n");
	paletteCount++;
	glBindTexture(GL_TEXTURE_2D, 0);

	/*GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_1D, texture_id);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, palette);
	glBindTexture(GL_TEXTURE_1D, 0);*/

	for (unsigned int i = 0; i < shapes.size(); i++) {
		renderers[GREEDY].push_back(new GreedyRender(shapes[i], paletteBank, paletteCount - 1));
		renderers[HEXAGON].push_back(new HexRender(shapes[i], paletteBank, paletteCount - 1));
		renderers[RTX].push_back(new RTX_Render(shapes[i], paletteBank, paletteCount - 1));
	}
#ifdef _BLENDER
	SaveTexture("palette.png", paletteBank);
#endif
}

void VoxLoader::draw(Shader& shader, Camera& camera, vec3 position, quat rotation, float scale, vec4 texture, RenderMethod method) {
	for (mv_model_iterator it = models.begin(); it != models.end(); it++) {
		int index = it->second.shape_index;
		const MV_Shape& shape = shapes[index];
		vec3 pos = it->second.position - (it->second.rotation * vec3(shape.sizex / 2, shape.sizey / 2, shape.sizez / 2));
		renderers[method][index]->setTransform(pos, it->second.rotation);
		renderers[method][index]->setWorldTransform(position, rotation);
		renderers[method][index]->setScale(scale);
		renderers[method][index]->setTexture(texture);
		renderers[method][index]->draw(shader, camera);
	}
}

void VoxLoader::draw(Shader& shader, Camera& camera, string shape_name, vec3 position, quat rotation, float scale, vec4 texture, RenderMethod method) {
	pair<mv_model_iterator, mv_model_iterator> homonym_shapes = models.equal_range(shape_name);
	for (mv_model_iterator it = homonym_shapes.first; it != homonym_shapes.second; it++) {
		int index = it->second.shape_index;
		const MV_Shape& shape = shapes[index];
		vec3 pos = it->second.rotation * vec3(-shape.sizex / 2, -shape.sizey / 2, 0);
		renderers[method][index]->setTransform(pos, it->second.rotation);
		renderers[method][index]->setWorldTransform(position, rotation);
		renderers[method][index]->setScale(scale);
		renderers[method][index]->setTexture(texture);
		renderers[method][index]->draw(shader, camera);
	}
}

void VoxLoader::push(ShadowVolume& shadow_volume, string shape_name, vec3 world_position, quat world_rotation) {
	pair<mv_model_iterator, mv_model_iterator> homonym_shapes = models.equal_range(shape_name);
	for (mv_model_iterator it = homonym_shapes.first; it != homonym_shapes.second; it++) {
		int index = it->second.shape_index;
		const MV_Shape& shape = shapes[index];
		vec3 position = it->second.rotation * vec3(-shape.sizex / 2, -shape.sizey / 2, 0);

		mat4 toWorldCoords = mat4(vec4(1, 0, 0, 0),
								  vec4(0, 0, -1, 0),
								  vec4(0, 1, 0, 0),
								  vec4(0, 0, 0, 1));

		// Coordinate system: x right, z up, y forward, scale 1:1 (in voxels)
		mat4 pos = translate(mat4(1.0f), position);
		mat4 rot = mat4_cast(it->second.rotation);
		mat4 localTr = toWorldCoords * pos * rot;

		// Coordinate system: x right, y up, -z forward, scale 1 meter : 10 voxels
		mat4 world_pos = translate(mat4(1.0f), 10.0f * world_position);
		mat4 world_rot = mat4_cast(world_rotation);
		mat4 worldTr = world_pos * world_rot;

		mat4 modelMatrix = worldTr * localTr;
		shadow_volume.addShape(shape, modelMatrix);
	}
}

VoxLoader::~VoxLoader() {
	for (int i = 0; i < 3; i++)
		for (unsigned int j = 0; j < renderers[i].size(); j++)
			delete renderers[i][j];
}
