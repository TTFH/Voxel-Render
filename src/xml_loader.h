#ifndef XML_LOADER_H
#define XML_LOADER_H

#include <map>
#include <string>
#include <vector>

#include "vox_loader.h"
#include "rope_render.h"
#include "water_render.h"
#include "voxbox_render.h"
#include "../lib/tinyxml2.h"

using namespace tinyxml2;

struct scene_t {
	string file;
	string object;
	vec3 position;
	quat rotation;
	float scale;
};

class Scene {
private:
	string parent_folder;
	vector<scene_t> shapes;
	map<string, VoxLoader*> models;
	vector<RopeRender*> ropes;
	vector<WaterRender*> waters;
	vector<VoxboxRender*> voxboxes;
	void RecursiveLoad(XMLElement* element, vec3 parent_pos, quat parent_rot);
public:
	Scene(string path);
	~Scene();
	void draw(Shader& shader, Camera& camera);
	void drawRope(Shader& shader, Camera& camera);
	void drawWater(Shader& shader, Camera& camera);
	void drawVoxbox(Shader& shader, Camera& camera);
};

#endif
