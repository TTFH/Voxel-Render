#include <stdio.h>
#include <stdexcept>

#include "vox_loader.h"
#include "render_vox_greedy.h"
#include "render_vox_hex.h"
#include "render_vox_rtx.h"
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
	int content_size;
	int children_size;
	long int end;
};

static int ReadInt(FILE* file) {
	int val = 0;
	fread(&val, sizeof(int), 1, file);
	return val;
}

static void ReadChunk(FILE* file, Chunk& chunk) {
	chunk.id = ReadInt(file);
	chunk.content_size = ReadInt(file);
	chunk.children_size = ReadInt(file);
	chunk.end = ftell(file) + chunk.content_size + chunk.children_size;
}

static string ReadString(FILE* file) {
	int size = ReadInt(file);
	char* buffer = new char[size];
	fread(buffer, sizeof(uint8_t), size, file);
	string res(buffer, buffer + size);
	delete[] buffer;
	return res;
}

static DICT ReadDict(FILE* file) {
	DICT dict;
	int dict_entries = ReadInt(file);
	for (int i = 0; i < dict_entries; i++) {
		string key = ReadString(file);
		string value = ReadString(file);
		dict[key] = value;
	}
	return dict;
}

static string GetDictValue(DICT& dict, string key) {
	DICT::iterator it = dict.find(key);
	if (it != dict.end())
		return it->second;
	return "";
}

static TD_Material Convert(const MV_Material& pbr) {
	TD_Material material;
	switch (pbr.type) {
	case METAL:
		material.reflectivity = pbr.sp - 1.0f;
		material.shinyness = 1.0f - pbr.rough;
		material.metalness = pbr.metal;
		break;
	case GLASS:
		material.shinyness = 1.0f - pbr.rough;
		break;
	case EMIT:
		material.emissive = pbr.emit * pow(10, pbr.flux);
		break;
	default:
		break;
	}
	return material;
}

static MV_MatType GetMaterialType(string type) {
	if (type == "_glass")
		return GLASS;
	else if (type == "_metal")
		return METAL;
	else if (type == "_emit")
		return EMIT;
	return DIFFUSE;
}

static float StringToFloat(string str, float default_value) {
	try {
		return stof(str);
	} catch (exception& e) {
		return default_value;
	}
}

static int ReadHeader(FILE* file) {
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

	Chunk main_chunk;
	ReadChunk(file, main_chunk);
	if (main_chunk.id != MAIN) {
		printf("[ERROR] MV Main chunk not found.\n");
		exit(EXIT_FAILURE);
	}
	if (main_chunk.content_size != 0) {
		printf("[ERROR] MV Main chunk content size is not zero.\n");
		exit(EXIT_FAILURE);
	}
	return main_chunk.end;
}

VoxLoader::VoxLoader(const char* filename) {
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
				int voxels_count = ReadInt(file);
				vector<MV_Voxel> voxels;
				if (voxels_count > 0) {
					voxels.resize(voxels_count);
					fread(voxels.data(), sizeof(MV_Voxel), voxels_count, file);
				}
				string id = RemoveExtension(filename) + "_" + to_string(shapes.size());
				MV_Shape shape = { id, sizex, sizey, sizez, voxels };
				shapes.push_back(shape);
				//printf("XYZI shape[%d]: %5d voxels, size = [%3d %3d %3d]\n", (int)shapes.size() - 1, voxels_count, sizex, sizey, sizez);
			}
			break;
		case RGBA:
			fread(palette + 1, sizeof(MV_Color), 255, file);
			fread(&palette, sizeof(MV_Color), 1, file);
			break;
		case nTRN: {
				int node_id = ReadInt(file);
				if (node_id == 0)
					break;
				vec3 position = vec3(0, 0, 0);
				quat rotation = quat(1, 0, 0, 0);
				DICT node_attribs = ReadDict(file);
				string shape_name = GetDictValue(node_attribs, "_name");

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
							mat3 rot_matrix = mat3(0.0f);
							rot_matrix[x][0] = (v >> 4) & 1 ? -1 : 1;
							rot_matrix[y][1] = (v >> 5) & 1 ? -1 : 1;
							rot_matrix[z][2] = (v >> 6) & 1 ? -1 : 1;
							rotation = quat_cast(rot_matrix);
							/*printf("Rot byte: 0x%02X\n", v);
							printf("    [%2d %2d %2d]\nR = [%2d %2d %2d]\n    [%2d %2d %2d]\n\n",
								(int)rot_matrix[0][0], (int)rot_matrix[0][1], (int)rot_matrix[0][2],
								(int)rot_matrix[1][0], (int)rot_matrix[1][1], (int)rot_matrix[1][2],
								(int)rot_matrix[2][0], (int)rot_matrix[2][1], (int)rot_matrix[2][2]);*/
						}
					}
				}
				MV_Model data = { ref_model_id, position, rotation };
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

				MV_Material pbr;
				pbr.type = GetMaterialType(GetDictValue(mat_properties, "_type"));
				pbr.flux = StringToFloat(GetDictValue(mat_properties, "_flux"), 0);
				pbr.rough = StringToFloat(GetDictValue(mat_properties, "_rough"), 1);
				pbr.sp = StringToFloat(GetDictValue(mat_properties, "_sp"), 1);
				pbr.metal = StringToFloat(GetDictValue(mat_properties, "_metal"), 0);
				pbr.emit = StringToFloat(GetDictValue(mat_properties, "_emit"), 0);

				int index = material_id % 256;
				float alpha = StringToFloat(GetDictValue(mat_properties, "_alpha"), 1);
				if (pbr.type == GLASS && alpha < 1.0f)
					palette[index].a = 0.5f;
				material[index] = Convert(pbr);
			}
			break;
		default:
			break;
		}
		fseek(file, sub.end, SEEK_SET);
	}
	fclose(file);

	palette_id = VoxRender::getIndex(palette, material);
}
