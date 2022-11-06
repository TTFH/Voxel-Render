#ifndef VOX_LOADER_H
#define VOX_LOADER_H

#define GREADY_MESHING_ENABLED 1

#include <stdint.h>
#include <string>
#include <map>

#include "camera.h"
#include "shader.h"
#include "voxel_render.h"
#include "greedy_mesh.h"

using namespace std;

struct MV_PBR {
	uint8_t flux;
	float rough, metal, emit, alpha;
};

struct MV_Model {
	int shape_index;
	vec3 position;
	quat rotation;
};

typedef multimap<string, MV_Model>::iterator mv_model_iterator;

class VoxLoader {
private:
	string filename;
	MV_Entry palette[256];
	//MV_PBR pbr[256];
	vector<MV_Shape> shapes;
	multimap<string, MV_Model> models;
#if GREADY_MESHING_ENABLED
	vector<FastRender*> render;
#else
	vector<VoxelRender*> render;
#endif
public:
	VoxLoader(const char* filename);
	void draw(Shader& shader, Camera& camera, vec3 position = vec3(0, 0, 0), quat rotation = quat(1, 0, 0, 0), float scale = 1);
	void draw(Shader& shader, Camera& camera, string shape_name, vec3 position = vec3(0, 0, 0), quat rotation = quat(1, 0, 0, 0), float scale = 1);
	~VoxLoader();
};

#endif
