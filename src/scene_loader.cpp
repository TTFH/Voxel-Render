#include <math.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "scene_loader.h"

struct shape_t {
	string file;
	string object;
	vec3 position;
	quat rotation;
	float scale;
	vec4 texture;
	RenderMethod method;
};

// Example on how to use tinyxml2
void iterate_xml(XMLElement* root, int depth) {
	for (int i = 0; i < depth; i++)
		printf("    ");
	printf("<%s", root->Name());
	for (const XMLAttribute* a = root->FirstAttribute(); a != NULL; a = a->Next())
		printf(" %s=\"%s\"", a->Name(), a->Value());
	if (root->NoChildren()) {
		printf("/>\n");
		return;
	}
	printf(">\n");
	for (XMLElement* e = root->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
		iterate_xml(e, depth + 1);
	for (int i = 0; i < depth; i++)
		printf("    ");
	printf("</%s>\n", root->Name());
}

void Scene::RecursiveLoad(XMLElement* element, vec3 parent_pos, quat parent_rot) {
	vec3 position = vec3(0, 0, 0);
	const char* pos = element->Attribute("pos");
	if (pos != NULL) {
		float x, y, z;
		sscanf(pos, "%f %f %f", &x, &y, &z);
		position = vec3(x, y, z);
	}
	quat rotation = quat(1, 0, 0, 0);
	const char* rot = element->Attribute("rot");
	if (rot != NULL) {
		float x, y, z;
		sscanf(rot, "%f %f %f", &x, &y, &z);
		rotation = quat_cast(eulerAngleYZ(radians(y), radians(z)) * eulerAngleX(radians(x)));
	}
	position = parent_rot * position + parent_pos;
	rotation = parent_rot * rotation;

	if (strcmp(element->Name(), "vox") == 0) {
		const char* file = element->Attribute("file");
		if (file == NULL) {
			printf("[Warning] No file specified for vox\n");
			return;
		}
		shape_t vox = { "", "ALL_SHAPES", position, rotation, 1.0f, vec4(0, 0, 1, 1), DEFAULT_METHOD };
		if (strncmp(file, "MOD/", 4) == 0)
			vox.file = parent_folder + string(file + 4);
		else if (strncmp(file, "LEVEL/", 6) == 0)
			vox.file = child_folder + string(file + 6);
		else if (strncmp(file, "BUILT-IN/", 9) == 0)
			vox.file = "built-in/" + string(file + 9);
		else
			vox.file = file;
		if (vox_files.find(vox.file) == vox_files.end()) {
			VoxLoader* vox_file = new VoxLoader(vox.file.c_str());
			vox_files[vox.file] = vox_file;
		}
		const char* object = element->Attribute("object");
		if (object != NULL) vox.object = object;
		const char* scale = element->Attribute("scale");
		if (scale != NULL) vox.scale = atof(scale);
		const char* method = element->Attribute("name");
		if (method != NULL) {
			if (strcmp(method, "rtx") == 0)
				vox.method = RTX;
			else if (strcmp(method, "gm") == 0)
				vox.method = GREEDY;
			else if (strcmp(method, "hex") == 0)
				vox.method = HEXAGON;
		}

		const char* texture = element->Attribute("texture");
		if (texture != NULL) {
			int tile = 0; float weight = 1;
			sscanf(texture, "%d %f", &tile, &weight);
			vox.texture.x = (float)tile;
			vox.texture.z = weight;
		}
		const char* blend_texture = element->Attribute("blendtexture");
		if (blend_texture != NULL) {
			int tile = 0; float weight = 1;
			sscanf(blend_texture, "%d %f", &tile, &weight);
			vox.texture.y = (float)tile;
			vox.texture.w = weight;
		}

		VoxLoader* vox_file = vox_files[vox.file];
		int palette_id = vox_file->palette_id;

		pair<mv_model_iterator, mv_model_iterator> homonym_shapes = vox_file->models.equal_range(vox.object);
		if (vox.object == "ALL_SHAPES")
			homonym_shapes = make_pair(vox_file->models.begin(), vox_file->models.end());
		for (mv_model_iterator it = homonym_shapes.first; it != homonym_shapes.second; it++) {
			int index = it->second.shape_index;
			const MV_Shape& shape = vox_file->shapes[index];
			vec3 pos = it->second.rotation * vec3(-shape.sizex / 2, -shape.sizey / 2, 0);
			if (vox.object == "ALL_SHAPES") {
				pos.z = -shape.sizez / 2;
				pos += it->second.position;
			}

			VoxRender* renderer = NULL;
			switch (vox.method) {
			case RTX:
				renderer = new RTX_Render(shape, palette_id);
				vox_rtx.push_back((RTX_Render*)renderer);
				((RTX_Render*)renderer)->setTexture(vox.texture);
				break;
			case GREEDY:
				renderer = new GreedyRender(shape, palette_id);
				vox_greedy.push_back((GreedyRender*)renderer);
				break;
			case HEXAGON:
				renderer = new HexRender(shape, palette_id);
				vox_hexagon.push_back((HexRender*)renderer);
				break;
			}

			renderer->setTransform(pos, it->second.rotation);
			renderer->setWorldTransform(position, rotation);
			renderer->setScale(vox.scale);

			//mat4 modelMatrix = renderer->getVolumeMatrix();
			//shadow_volume->addShape(shape, modelMatrix);
		}
	} else if (strcmp(element->Name(), "voxbox") == 0) {
		vec3 size = vec3(10, 10, 10);
		const char* size_str = element->Attribute("size");
		if (size_str != NULL) {
			float x, y, z;
			sscanf(size_str, "%f %f %f", &x, &y, &z);
			size = vec3(x, y, z);
		}
		vec3 color = vec3(1, 1, 1);
		const char* color_str = element->Attribute("color");
		if (color_str != NULL) {
			float r, g, b;
			sscanf(color_str, "%f %f %f", &r, &g, &b);
			color = vec3(r, g, b);
		}
		VoxboxRender* voxbox = new VoxboxRender(size, color);
		voxbox->setWorldTransform(position, rotation);
		voxboxes.push_back(voxbox);
	} else if (strcmp(element->Name(), "water") == 0) {
		vector<vec2> water_verts;
		for (XMLElement* e = element->FirstChildElement(); e != NULL; e = e->NextSiblingElement()) {
			if (strcmp(e->Name(), "vertex") == 0) {
				const char* pos = e->Attribute("pos");
				if (pos != NULL) {
					float x, y;
					sscanf(pos, "%f %f", &x, &y);
					water_verts.push_back(vec2(x, y));
				}
			}
		}
		if (water_verts.size() > 2) {
			std::reverse(water_verts.begin(), water_verts.end()); // TD order is CW
			WaterRender* water = new WaterRender(water_verts);
			water->setWorldTransform(position);
			waters.push_back(water);
		}
	} else if (strcmp(element->Name(), "rope") == 0 || strcmp(element->Name(), "voxagon") == 0) {
		vector<vec3> rope_verts;
		for (XMLElement* e = element->FirstChildElement(); e != NULL; e = e->NextSiblingElement()) {
			if (strcmp(e->Name(), "location") == 0 || strcmp(e->Name(), "vertex") == 0) {
				const char* pos = e->Attribute("pos");
				vec3 vert_pos = vec3(0, 0, 0);
				if (pos != NULL) {
					float x, y, z;
					sscanf(pos, "%f %f %f", &x, &y, &z);
					vert_pos = vec3(x, y, z);
				}
				rope_verts.push_back(vert_pos);
			}
		}
		vec3 color = vec3(0, 0, 0);
		const char* color_str = element->Attribute("color");
		if (color_str != NULL) {
			float r, g, b;
			sscanf(color_str, "%f %f %f", &r, &g, &b);
			color = vec3(r, g, b);
		}
		if (rope_verts.size() > 1) {
			RopeRender* rope = new RopeRender(rope_verts, color);
			rope->setWorldTransform(position, rotation);
			ropes.push_back(rope);
		}
	} else if (strcmp(element->Name(), "spawnpoint") == 0) {
		spawnpoint.pos = position;
		spawnpoint.rot = rotation;
	} else if (strcmp(element->Name(), "boundary") == 0) {
		vector<vec2> boundary_verts;
		for (XMLElement* e = element->FirstChildElement(); e != NULL; e = e->NextSiblingElement()) {
			if (strcmp(e->Name(), "vertex") == 0) {
				const char* pos = e->Attribute("pos");
				if (pos != NULL) {
					float x, y;
					sscanf(pos, "%f %f", &x, &y);
					boundary_verts.push_back(vec2(x, y));
				}
			}
		}
		if (boundary_verts.size() > 2)
			boundary = new BoundaryRender(boundary_verts);
	} else if (strcmp(element->Name(), "mesh") == 0) {
		vec3 color = vec3(0, 0, 0);
		const char* color_str = element->Attribute("color");
		if (color_str != NULL) {
			float r, g, b;
			sscanf(color_str, "%f %f %f", &r, &g, &b);
			color = vec3(r, g, b);
		}
		const char* file = element->Attribute("file");
		string path = parent_folder + file;
		Mesh* mesh = new Mesh(path.c_str(), color);
		mesh->setWorldTransform(position, rotation);
		meshes.push_back(mesh);
	} else if (strcmp(element->Name(), "instance") == 0) {
		const char* file = element->Attribute("file");
		string instance_path = parent_folder + file;
		if (strncmp(file, "MOD/", 4) == 0)
			instance_path = parent_folder + string(file + 4);
		else if (strncmp(file, "LEVEL/", 6) == 0)
			instance_path = child_folder + string(file + 6);
		else if (strncmp(file, "BUILT-IN/", 9) == 0)
			instance_path = "built-in/" + string(file + 9);
		XMLDocument instance_file;
		if (instance_file.LoadFile(instance_path.c_str()) != XML_SUCCESS) {
			printf("[Warning] Instance XML file %s not found.\n", instance_path.c_str());
			return;
		}
		XMLElement* instance_root = instance_file.RootElement();
		RecursiveLoad(instance_root, position, rotation);
	}
	for (XMLElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
		RecursiveLoad(child, position, rotation);
}

Scene::Scene(string path) {
	shadow_volume = new ShadowVolume(20, 5, 20);
	XMLDocument xml_file;
	if (xml_file.LoadFile(path.c_str()) != XML_SUCCESS) {
		printf("[Warning] XML file %s not found or corrupted.\n", path.c_str());
		return;
	}
	size_t last_slash_idx = path.find_last_of("\\/");
	parent_folder = path.substr(0, last_slash_idx + 1);
	child_folder = path.substr(0, path.rfind('.')) + "/";

	XMLElement* root = xml_file.RootElement();
	//iterate_xml(root, 0);
	boundary = NULL;
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	spawnpoint = { position, rotation };
	RecursiveLoad(root, position, rotation);
	shadow_volume->updateTexture();
}

void Scene::addMesh(Mesh* mesh) {
	meshes.push_back(mesh);
}

void Scene::draw(Shader& shader, Camera& camera, RenderMethod method) {
	switch (method) {
	case RTX:
		for (vector<RTX_Render*>::iterator it = vox_rtx.begin(); it != vox_rtx.end(); it++)
			(*it)->draw(shader, camera);
		break;
	case GREEDY:
		for (vector<GreedyRender*>::iterator it = vox_greedy.begin(); it != vox_greedy.end(); it++)
			(*it)->draw(shader, camera);
		break;
	case HEXAGON:
		for (vector<HexRender*>::iterator it = vox_hexagon.begin(); it != vox_hexagon.end(); it++)
			(*it)->draw(shader, camera);
		break;
	}
}

void Scene::drawVoxbox(Shader& shader, Camera& camera) {
	for (vector<VoxboxRender*>::iterator it = voxboxes.begin(); it != voxboxes.end(); it++)
		(*it)->draw(shader, camera);
}

void Scene::drawMesh(Shader& shader, Camera& camera) {
	for (vector<Mesh*>::iterator it = meshes.begin(); it != meshes.end(); it++)
		(*it)->draw(shader, camera);
}

void Scene::drawRope(Shader& shader, Camera& camera) {
	for (vector<RopeRender*>::iterator it = ropes.begin(); it != ropes.end(); it++)
		(*it)->draw(shader, camera);
}

void Scene::drawWater(Shader& shader, Camera& camera) {
	for (vector<WaterRender*>::iterator it = waters.begin(); it != waters.end(); it++)
		(*it)->draw(shader, camera);
}

void Scene::drawBoundary(Shader& shader, Camera& camera) {
	if (boundary != NULL)
		boundary->draw(shader, camera);
}

void Scene::drawShadowVolume(Shader& shader, Camera& camera) {
	shadow_volume->draw(shader, camera);
}

Scene::~Scene() {
	delete shadow_volume;
	for (vector<Mesh*>::iterator it = meshes.begin(); it != meshes.end(); it++)
		delete *it;
	for (vector<RopeRender*>::iterator it = ropes.begin(); it != ropes.end(); it++)
		delete *it;
	for (vector<WaterRender*>::iterator it = waters.begin(); it != waters.end(); it++)
		delete *it;
	for (vector<VoxboxRender*>::iterator it = voxboxes.begin(); it != voxboxes.end(); it++)
		delete *it;
	for (vector<GreedyRender*>::iterator it = vox_greedy.begin(); it != vox_greedy.end(); it++)
		delete *it;
	for (vector<HexRender*>::iterator it = vox_hexagon.begin(); it != vox_hexagon.end(); it++)
		delete *it;
	for (vector<RTX_Render*>::iterator it = vox_rtx.begin(); it != vox_rtx.end(); it++)
		delete *it;
	if (boundary != NULL)
		delete boundary;
	for (map<string, VoxLoader*>::iterator it = vox_files.begin(); it != vox_files.end(); it++)
		delete it->second;
}
