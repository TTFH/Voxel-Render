#ifndef VOX_LOADER_H
#define VOX_LOADER_H

#define NONE 0
#define GREEDY 1
#define HEXAGON 2
#define RTX 3
#define RENDER_METHOD GREEDY

#include <map>
#include <string>
#include <stdint.h>

#include "camera.h"
#include "shader.h"
#include "hex_render.h"
#include "greedy_mesh.h"

using namespace std;

struct MV_Entry {
	uint8_t r, g, b, a;
};
/*
struct MV_PBR { // Unused
	bool alpha; // Transparency based on V channel of HSV
	uint8_t flux;
	float rough, sp, metal, emit;
};
*/
struct MV_Model {
	int shape_index;
	vec3 position;
	quat rotation;
};
/*
struct MV_Voxel { // Defined in vbo.h
	uint8_t x, y, z, index;
};

struct MV_Shape { // Defined in greedy_mesh.h
	int sizex, sizey, sizez;
	vector<MV_Voxel> voxels;
};
*/
typedef multimap<string, MV_Model>::iterator mv_model_iterator;

class VoxLoader {
private:
	string filename;
	MV_Entry palette[256];
	//MV_PBR pbr[256];
	vector<MV_Shape> shapes;
	multimap<string, MV_Model> models;
#if RENDER_METHOD == GREEDY
	vector<GreedyRender*> render;
#elif RENDER_METHOD == HEXAGON
	vector<HexRender*> render;
#endif
public:
	VoxLoader();
	VoxLoader(const char* filename);
	void load(const char* filename);
	void draw(Shader& shader, Camera& camera, vec4 clip_plane, vec3 position = vec3(0, 0, 0), quat rotation = quat(1, 0, 0, 0), float scale = 1);
	void draw(Shader& shader, Camera& camera, vec4 clip_plane, string shape_name, vec3 position = vec3(0, 0, 0), quat rotation = quat(1, 0, 0, 0), float scale = 1);
	~VoxLoader();
};

#endif
