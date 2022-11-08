#include <stdio.h>
#include "vox_loader.h"

constexpr int ID(char a, char b, char c, char d) {
	return a | (b << 8) | (c << 16) | (d << 24);
}

const int VERSION = 150;
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

int ReadHeader(const char* filename, FILE* file) {
	int magic = ReadInt(file);
	if (magic != VOX) {
		printf("[ERROR] Invalid .vox file format.\n");
		exit(EXIT_FAILURE);
	}

	int version = ReadInt(file);
	if (version != VERSION)
		printf("[Warning] Version mismatch is file %s\n", filename);

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

VoxLoader::VoxLoader(const char* filename) {
	this->filename = filename;
	vector<MV_Voxel> voxels;
	int sizex = 0;
	int sizey = 0;
	int sizez = 0;
	int ref_model_id = -1;
	mv_model_iterator last_inserted = models.end();

	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		printf("[ERROR] File %s not found.\n", filename);
		return;
	}
	int file_size = ReadHeader(filename, file);

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
				MV_Shape shape = { sizex, sizey, sizez, voxels };
				shapes.push_back(shape);
				//printf("XYZI shape[%d]: %5d voxels, size = [%3d %3d %3d]\n", (int)shapes.size() - 1, numVoxels, sizex, sizey, sizez);
			}
			break;
		case RGBA:
			fread(palette + 1, sizeof(MV_Entry), 255, file);
			fread(&palette, sizeof(MV_Entry), 1, file);
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
						} else if (it->first == "_r") {
							int v, x, y, z;
							v = stoi(it->second);
							x = (v >> 0) & 3;
							y = (v >> 2) & 3;
							z = 3 - x - y;
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
		default:
			break;
		}
		fseek(file, sub.end, SEEK_SET);
	}
	fclose(file);
	//printf("File loaded: %s\n", filename);

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_1D, texture_id);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, palette);

	for (unsigned int i = 0; i < shapes.size(); i++) {
	#if GREADY_MESHING_ENABLED
		render.push_back(new FastRender(shapes[i], texture_id));
	#else
		render.push_back(new VoxelRender(shapes[i].voxels, texture_id));
	#endif
	}
}

void VoxLoader::draw(Shader& shader, Camera& camera, vec3 position, quat rotation, float scale) {
	for (mv_model_iterator it = models.begin(); it != models.end(); it++) {
		int index = it->second.shape_index;
		const MV_Shape& shape = shapes[index];
		vec3 pos = it->second.position - (it->second.rotation * vec3(shape.sizex / 2, shape.sizey / 2, shape.sizez / 2));
		render[index]->setTransform(pos, it->second.rotation);
		render[index]->setWorldTransform(position, rotation);
		render[index]->draw(shader, camera, scale);
	}
}

void VoxLoader::draw(Shader& shader, Camera& camera, string shape_name, vec3 position, quat rotation, float scale) {
	if (shape_name == "") {
		printf("[ERROR] Called individual draw for group of shapes.\n");
		exit(EXIT_FAILURE);
	}
	pair<mv_model_iterator, mv_model_iterator> homonym_shapes = models.equal_range(shape_name);
	for (mv_model_iterator it = homonym_shapes.first; it != homonym_shapes.second; it++) {
		int index = it->second.shape_index;
		const MV_Shape& shape = shapes[index];
		vec3 pos = it->second.rotation * vec3(-shape.sizex / 2, -shape.sizey / 2, 0);
		render[index]->setTransform(pos, it->second.rotation);
		render[index]->setWorldTransform(position, rotation);
		render[index]->draw(shader, camera, scale);
	}
}

VoxLoader::~VoxLoader() {
	for (unsigned int i = 0; i < render.size(); i++)
		delete render[i];
}
