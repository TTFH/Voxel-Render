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
#include "render_vox_greedy.h"
#include "render_vox_hex.h"
#include "render_vox_rtx.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../lib/tinyxml2.h"
using namespace tinyxml2;

enum RenderMethod : uint8_t {
	RTX,
	GREEDY,
	HEXAGON,
};

#ifdef _BLENDER
#define DEFAULT_METHOD GREEDY
#else
#define DEFAULT_METHOD RTX
#endif

struct Transform {
	vec3 pos;
	quat rot;
};

class Scene {
private:
	string parent_folder;
	string child_folder;
	vector<Mesh*> meshes;
	BoundaryRender* boundary;
	vector<RopeRender*> ropes;
	vector<WaterRender*> waters;
	vector<VoxboxRender*> voxboxes;
	ShadowVolume* shadow_volume;
	vector<RTX_Render*> vox_rtx;
	vector<HexRender*> vox_hexagon;
	vector<GreedyRender*> vox_greedy;
	map<string, VoxLoader*> vox_files;
	void recursiveLoad(XMLElement* element, vec3 parent_pos, quat parent_rot);
public:
	Transform spawnpoint;
	Scene(string path);
	~Scene();
	void draw(Shader& shader, Camera& camera, RenderMethod method);
	void drawVoxbox(Shader& shader, Camera& camera);
	void drawMesh(Shader& shader, Camera& camera);
	void drawRope(Shader& shader, Camera& camera);
	void drawWater(Shader& shader, Camera& camera);
	void drawBoundary(Shader& shader, Camera& camera);
	void drawShadowVolume(Shader& shader, Camera& camera);
};

#endif
