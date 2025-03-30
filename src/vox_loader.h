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

struct MV_Diffuse {
	uint8_t r, g, b, a;
};
/*
struct MV_PBR {
	uint8_t flux;
	float rough, sp, metal, emit;
};

struct MV_Material {
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;
}
*/
struct MV_Voxel {
	uint8_t x, y, z, index;
};

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
	MV_Diffuse palette[256];
	multimap<string, MV_Model> models;
	VoxLoader(const char* filename);
};

#endif
