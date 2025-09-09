#ifndef VOX_LOADER_H
#define VOX_LOADER_H

#include <map>
#include <string>
#include <vector>
#include <stdint.h>

#include "camera.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std;

enum MV_MatType {
	DIFFUSE,
	METAL,
	GLASS,
	EMIT
};

struct MV_Material {
	MV_MatType type;
	uint8_t flux;
	float rough, sp, metal, emit;
};

struct TD_Material {
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;

	TD_Material() {
		reflectivity = 0;
		shinyness = 0;
		metalness = 0;
		emissive = 0;
	}
};

struct MV_Color {
	uint8_t r, g, b, a;
};

struct MV_Voxel {
	uint8_t x, y, z, index;
};

struct VoxelHash {
	size_t operator()(const tuple<uint8_t, uint8_t, uint8_t>& pos) const {
		return hash<uint32_t>()((get<0>(pos) << 16) | (get<1>(pos) << 8) | get<2>(pos));
	}
};

const uint8_t SNOW_INDEX = 254;
const uint8_t HOLE_INDEX = 255;

struct MV_Shape {
	string id;
	int sizex, sizey, sizez;
	vector<MV_Voxel> voxels;
};

struct MV_Model {
	int shape_index;
	vec3 position;
	quat rotation;
};

typedef multimap<string, MV_Model>::iterator mv_model_iterator;

class VoxLoader {
public:
	int palette_id;
	vector<MV_Shape> shapes;
	MV_Color palette[256];
	TD_Material material[256];
	multimap<string, MV_Model> models;
	VoxLoader(const char* filename);
};

#endif
