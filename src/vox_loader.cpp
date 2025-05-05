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
	if (it != dict.end())
		return it->second;
	return "";
}

MV_Material::MV_Material(MV_PBR pbr) {
	reflectivity = 0.1f;
	shinyness = 1.0f;
	metalness = 0.0f;
	emissive = 0.0f;

	switch (pbr.type) {
	case METAL:
		reflectivity = pbr.sp;
		shinyness = 1.0f - pbr.rough;
		metalness = pbr.metal;
		break;
	case GLASS:
		shinyness = 1.0f - pbr.rough;
		break;
	case EMIT:
		emissive = pbr.emit * pow(10, pbr.flux);
		break;
	default:
		break;
	}
}

MV_MaterialType GetMaterialType(string type) {
	if (type == "_glass")
		return GLASS;
	else if (type == "_metal")
		return METAL;
	else if (type == "_emit")
		return EMIT;
	return DIFFUSE;
}

float StringToFloat(string str) {
	try {
		return stof(str);
	} catch (const invalid_argument&) {
		return 0.0f;
	} catch (const out_of_range&) {
		return 0.0f;
	}
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
				if (voxels_count > 0) {
					vector<MV_Voxel> voxels;
					voxels.resize(voxels_count);
					fread(voxels.data(), sizeof(MV_Voxel), voxels_count, file);
					string id = RemoveExtension(filename) + "_" + to_string(shapes.size());
					MV_Shape shape = { id, sizex, sizey, sizez, voxels };
					shapes.push_back(shape);
				}
				//printf("XYZI shape[%d]: %5d voxels, size = [%3d %3d %3d]\n", (int)shapes.size() - 1, voxels_count, sizex, sizey, sizez);
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

				MV_PBR pbr;
				pbr.type = GetMaterialType(GetDictValue(mat_properties, "_type"));
				pbr.flux = StringToFloat(GetDictValue(mat_properties, "_flux"));
				pbr.rough = StringToFloat(GetDictValue(mat_properties, "_rough"));
				pbr.sp = StringToFloat(GetDictValue(mat_properties, "_sp"));
				pbr.metal = StringToFloat(GetDictValue(mat_properties, "_metal"));
				pbr.emit = StringToFloat(GetDictValue(mat_properties, "_emit"));

				int index = material_id % 256;
				bool transparent = GetDictValue(mat_properties, "_alpha") != "1.0";
				if (pbr.type == GLASS && transparent)
					palette[index].a = 0.5f;
				material[index] = MV_Material(pbr);
			}
			break;
		default:
			break;
		}
		fseek(file, sub.end, SEEK_SET);
	}
	fclose(file);

	palette_id = VoxRender::getIndex(palette, material);
#ifdef _BLENDER
	VoxRender::SaveTexture();
#endif
}
