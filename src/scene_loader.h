#ifndef XML_LOADER_H
#define XML_LOADER_H

#include <map>
#include <string>
#include <vector>

#include "render_mesh.h"
#include "vox_loader.h"
#include "render_rope.h"
#include "render_water.h"
#include "render_voxbox.h"
#include "shadow_volume.h"
#include "../lib/tinyxml2.h"

using namespace tinyxml2;

struct shape_t {
	string file;
	string object;
	vec3 position;
	quat rotation;
	float scale;
	vec4 texture;
	RenderMethod method;
};

class Scene {
private:
	string parent_folder;
	string child_folder;
	vector<shape_t> shapes;
	vector<Mesh*> meshes;
	vector<RopeRender*> ropes;
	vector<VoxboxRender*> voxboxes;
	map<string, VoxLoader*> models;
	void RecursiveLoad(XMLElement* element, vec3 parent_pos, quat parent_rot);
public:
	vector<WaterRender*> waters; // public for water shader
	Scene(string path);
	~Scene();
	void addMesh(Mesh* mesh);
	void push(ShadowVolume& shadow_volume);
	void draw(Shader& shader, Camera& camera, RenderMethod method);
	void drawVoxbox(Shader& shader, Camera& camera);
	void drawMesh(Shader& shader, Camera& camera);
	void drawRope(Shader& shader, Camera& camera);
	void drawWater(Shader& shader, Camera& camera);
};

#endif
