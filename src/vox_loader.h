#ifndef VOX_LOADER_H
#define VOX_LOADER_H

#include <map>
#include <string>
#include <stdint.h>

#include "camera.h"
#include "shader.h"
#include "shadow_volume.h"
#include "render_interface.h"

using namespace std;

enum RenderMethod {
	GREEDY,
	HEXAGON,
	RTX
};

struct MV_Diffuse {
	uint8_t r, g, b, a;
};
/*
struct MV_PBR { // Unused
	uint8_t flux;
	float rough, sp, metal, emit;
};

struct MV_Material { // Unused
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;
}

struct MV_Voxel { // Defined in vbo.h
	uint8_t x, y, z, index;
};

struct MV_Shape { // Defined in vbo.h
	string id;
	int sizex, sizey, sizez;
	vector<MV_Voxel> voxels;
};
*/
struct MV_Model {
	int shape_index;
	vec3 position;
	quat rotation;
};

typedef multimap<string, MV_Model>::iterator mv_model_iterator;

class VoxLoader {
private:
	string filename;
	MV_Diffuse palette[256];
	vector<MV_Shape> shapes;
	vector<IRender*> renderers[3];
	multimap<string, MV_Model> models;
public:
	VoxLoader();
	VoxLoader(const char* filename);
	void load(const char* filename);
	void draw(Shader& shader, Camera& camera, vec3 position, quat rotation, float scale, vec4 texture, RenderMethod method);
	void draw(Shader& shader, Camera& camera, string shape_name, vec3 position, quat rotation, float scale, vec4 texture, RenderMethod method);
	void push(ShadowVolume& shadow_volume, string shape_name, vec3 position, quat rotation);
	~VoxLoader();
};

#endif
