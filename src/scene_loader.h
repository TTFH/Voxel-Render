#ifndef XML_LOADER_H
#define XML_LOADER_H

#include <map>
#include <string>
#include <vector>

#include "vox_loader.h"
#include "render_rope.h"
#include "render_mesh.h"
#include "render_water.h"
#include "render_voxbox.h"
#include "shadow_volume.h"
#include "render_boundary.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../lib/tinyxml2.h"
using namespace tinyxml2;

struct shape_t {
	string file;
	string object;
	vec3 position;
	quat rotation;
	float scale;
	vec4 texture;
	RenderMethod method; // TODO: remove
};

struct Transform {
	vec3 pos;
	quat rot;
};

class Scene {
private:
	string parent_folder;
	string child_folder;
	vector<Mesh*> meshes;
	vector<shape_t> shapes;
	BoundaryRender* boundary;
	vector<RopeRender*> ropes;
	vector<WaterRender*> waters;
	vector<VoxboxRender*> voxboxes;
	map<string, VoxLoader*> models;
	void RecursiveLoad(XMLElement* element, vec3 parent_pos, quat parent_rot);
public:
	Transform spawnpoint;
	Scene(string path);
	~Scene();
	void addMesh(Mesh* mesh);
	void push(ShadowVolume& shadow_volume);
	void draw(Shader& shader, Camera& camera, RenderMethod method);
	void drawVoxbox(Shader& shader, Camera& camera);
	void drawMesh(Shader& shader, Camera& camera);
	void drawRope(Shader& shader, Camera& camera);
	void drawWater(Shader& shader, Camera& camera);
	void drawBoundary(Shader& shader, Camera& camera);
};

#endif
