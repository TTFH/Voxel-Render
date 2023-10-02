#include <math.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include <glm/gtx/euler_angles.hpp>

#include "xml_loader.h"

// Example of how to use tinyxml2
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
		shape_t vox = { "", "ALL_OBJECTS", position, rotation, 1.0f, vec4(0, 0, 1, 1) };
		const char* file = element->Attribute("file");
		if (file == NULL) {
			printf("[ERROR] No file specified for vox\n");
			exit(EXIT_FAILURE);
		}
		if (strncmp(file, "MOD/", 4) == 0)
			vox.file = parent_folder + string(file + 4);
		else if (strncmp(file, "LEVEL/", 6) == 0)
			vox.file = child_folder + string(file + 6);
		else
			vox.file = file;
		if (models.find(vox.file) == models.end()) {
			VoxLoader* model = new VoxLoader(vox.file.c_str());
			models[vox.file] = model;
		}
		const char* object = element->Attribute("object");
		if (object != NULL) vox.object = object;
		const char* scale = element->Attribute("scale");
		if (scale != NULL) vox.scale = atof(scale);

		const char* texture = element->Attribute("texture");
		if (texture != NULL) {
			int tile = 0; float weight = 1;
			sscanf(texture, "%d %f", &tile, &weight);
			vox.texture.x = tile;
			vox.texture.z = weight;
		}
		const char* blend_texture = element->Attribute("blendtexture");
		if (blend_texture != NULL) {
			int tile = 0; float weight = 1;
			sscanf(blend_texture, "%d %f", &tile, &weight);
			vox.texture.y = tile;
			vox.texture.w = weight;
		}
		shapes.push_back(vox);
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
	} else if (strcmp(element->Name(), "rope") == 0) {
		vector<vec3> rope_verts;
		for (XMLElement* e = element->FirstChildElement(); e != NULL; e = e->NextSiblingElement()) {
			if (strcmp(e->Name(), "location") == 0) {
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
	}
	for (XMLElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
		RecursiveLoad(child, position, rotation);
}

Scene::Scene(string path) {
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
	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	RecursiveLoad(root, position, rotation);
	printf("Loaded %d objects, %d water, %d rope\n", (int)shapes.size(), (int)waters.size(), (int)ropes.size());
}

void Scene::addMesh(Mesh* mesh) {
	meshes.push_back(mesh);
}

void Scene::push(ShadowVolume& shadow_volume) {
	for (vector<shape_t>::iterator it = shapes.begin(); it != shapes.end(); it++) {
		VoxLoader* model = models[it->file];
		if (it->object == "ALL_OBJECTS")
			printf("[Warning] All shapes not implemented for shadow volume\n");
		else
			model->push(shadow_volume, it->object, it->position, it->rotation);
	}
}

void Scene::draw(Shader& shader, Camera& camera) {
	for (vector<shape_t>::iterator it = shapes.begin(); it != shapes.end(); it++) {
		VoxLoader* model = models[it->file];
		if (it->object == "ALL_OBJECTS")
			model->draw(shader, camera, it->position, it->rotation, it->scale, it->texture, RTX);
		else
			model->draw(shader, camera, it->object, it->position, it->rotation, it->scale, it->texture, RTX);
	}
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

Scene::~Scene() {
	for (map<string, VoxLoader*>::iterator it = models.begin(); it != models.end(); it++)
		delete it->second;
}
