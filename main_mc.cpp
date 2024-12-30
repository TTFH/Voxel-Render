#include <map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "src/vao.h"
#include "src/ebo.h"
#include "src/render_mesh.h"
#include "src/light.h"
#include "src/utils.h"
#include "src/camera.h"
#include "src/shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#include "lib/json.hpp"

using namespace std;
using namespace glm;
using json = nlohmann::json;

// 2--------3
// | \      |
// |   x    |
// |     \  |
// 0--------1
static GLfloat quad_vertices[] = {
	// pos			// uv
	-0.5, -0.5,		0, 0,
	 0.5, -0.5,		1, 0,
	-0.5,  0.5,		0, 1,
	 0.5,  0.5,		1, 1,
};

static GLuint quad_indices[] = {
	0, 1, 2,
	1, 3, 2,
};

class Quad {
private:
	VAO vao;
	GLuint texture;
	int texture_rotation = 0;
	vec2 size = vec2(1, 1);
	vec2 uv_min = vec2(0, 0);
	vec2 uv_max = vec2(1, 1);
	vec4 tint_color = vec4(1, 1, 1, 1);

	vec3 position = vec3(0, 0, 0);
	quat rotation = quat(1, 0, 0, 0);
	vec3 world_position = vec3(0, 0, 0);
	quat world_rotation = quat(1, 0, 0, 0);
public:
	Quad(GLuint texture) {
		this->texture = texture;
		vao.Bind();
		VBO vbo(quad_vertices, sizeof(quad_vertices));
		EBO ebo(quad_indices, sizeof(quad_indices));
		vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)0);
		vao.LinkAttrib(vbo, 1, 2, GL_FLOAT, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		vao.Unbind();
		vbo.Unbind();
		ebo.Unbind();
	}
	void setSize(vec2 size) {
		this->size = size;
	}
	void setTransform(vec3 pos, quat rot) {
		position = pos;
		rotation = rot;
	}
	void setWorldPosition(vec3 pos) {
		world_position = pos;
	}
	void setWorldRotation(quat rot) {
		world_rotation = rot;
	}
	void setUV(vec2 min, vec2 max, int rot) {
		uv_min = min;
		uv_max = max;
		texture_rotation = rot;
	}
	void setTint(vec4 color) {
		tint_color = color;
	}
	void draw(Shader& shader, Camera& camera) {
		vao.Bind();
		shader.PushMatrix("camera", camera.vpMatrix);
		shader.PushTexture2D("diffuse", texture, 0);
		shader.PushVec2("size", size);
		shader.PushVec4("tint_color", tint_color);
		shader.PushVec2("uv_min", uv_min);
		shader.PushVec2("uv_max", uv_max);
		shader.PushInt("tex_rot", texture_rotation);

		mat4 pos = translate(mat4(1.0f), position);
		mat4 rot = mat4_cast(rotation);
		mat4 w_pos = translate(mat4(1.0f), world_position);
		mat4 w_rot = mat4_cast(world_rotation);
		shader.PushMatrix("position", pos);
		shader.PushMatrix("rotation", rot);
		shader.PushMatrix("world_pos", w_pos);
		shader.PushMatrix("world_rot", w_rot);
		glDrawElements(GL_TRIANGLES, sizeof(quad_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		vao.Unbind();
	}
};

class Cuboid {
private:
	vec3 size;
	vec3 from, to;
	vector<Quad*> quads;
	quat parent_rot = quat(1, 0, 0, 0);
public:
	Cuboid(vec3 from, vec3 to) {
		this->from = from;
		this->to = to;
		this->size = to - from;
	}
	void addFace(string orientation, quat parent_rot, GLuint texture, vec4 uv, int tex_rot, bool tinted) {
		this->parent_rot = parent_rot;
		Quad* quad = new Quad(texture);
		vec2 face_size = vec2(16, 16);
		vec3 position = vec3(0, 0, 0);
		quat rotation = quat(1, 0, 0, 0);
		if (orientation == "up") {
			face_size = vec2(size.x, size.z);
			position = vec3(0, size.y / 2.0f, 0);
			rotation = angleAxis(radians(-90.0f), vec3(1, 0, 0));
			if (tex_rot == -1)
				uv = vec4(from.x, 16.0f - to.z, to.x, 16.0f - from.z);
		} else if (orientation == "down") {
			face_size = vec2(size.x, size.z);
			position = vec3(0, -size.y / 2.0f, 0);
			rotation = angleAxis(radians(90.0f), vec3(1, 0, 0));
			if (tex_rot == -1)
				uv = vec4(from.x, from.z, to.x, to.z);
		} else if (orientation == "north") {
			face_size = vec2(size.x, size.y);
			position = vec3(0, 0, -size.z / 2.0f);
			rotation = angleAxis(radians(180.0f), vec3(0, 1, 0));
			if (tex_rot == -1)
				uv = vec4(from.x, from.y, to.x, to.y);
		} else if (orientation == "south") {
			face_size = vec2(size.x, size.y);
			position = vec3(0, 0, size.z / 2.0f);
			if (tex_rot == -1)
				uv = vec4(from.x, from.y, to.x, to.y);
		} else if (orientation == "east") {
			face_size = vec2(size.z, size.y);
			position = vec3(size.x / 2.0f, 0, 0);
			rotation = angleAxis(radians(90.0f), vec3(0, 1, 0));
			if (tex_rot == -1)
				uv = vec4(from.z, from.y, to.z, to.y);
		} else if (orientation == "west") {
			face_size = vec2(size.z, size.y);
			position = vec3(-size.x / 2.0f, 0, 0);
			rotation = angleAxis(radians(-90.0f), vec3(0, 1, 0));
			if (tex_rot == -1)
				uv = vec4(from.z, from.y, to.z, to.y);
		}

		// Vertical texture coords starts from the top
		if (tex_rot != -1) {
			float temp = uv.y;
			uv.y = 16.0f - uv.w;
			uv.w = 16.0f - temp;
		}

		quad->setSize(face_size / 16.0f);
		quad->setTransform(position / 16.0f, rotation);
		quad->setUV(vec2(uv.x, uv.y) / 16.0f, vec2(uv.z, uv.w) / 16.0f, tex_rot);
		if (tinted) quad->setTint(vec4(0.57f, 0.74f, 0.35f, 1.0f));
		quads.push_back(quad);
	}
	void setTransform(vec3 world_pos, quat world_rot) {
		vec3 offset = (from + to) / 32.0f;
		for (vector<Quad*>::iterator it = quads.begin(); it != quads.end(); it++) {
			(*it)->setWorldPosition(world_pos + world_rot * offset - world_rot * vec3(0.5, 0.5, 0.5));
			(*it)->setWorldRotation(world_rot * parent_rot);
		}
	}
	void draw(Shader& shader, Camera& camera) {
		for (vector<Quad*>::iterator it = quads.begin(); it != quads.end(); it++)
			(*it)->draw(shader, camera);
	}
	~Cuboid() {
		for (vector<Quad*>::iterator it = quads.begin(); it != quads.end(); it++)
			delete *it;
	}
};

map<string, GLuint> texture_cache;

class MC_Block {
private:
	vector<Cuboid*> cuboids;
	map<string, GLuint> textures;

	void loadFile(string path) {
		ifstream file(path);
		if (!file.is_open()) {
			printf("[ERROR] File %s not found\n", path.c_str());
			return;
		}
		json block_js = json::parse(file);
		const string MC_PREFIX = "minecraft:";

		json textures_js = block_js["textures"];
		for (json::iterator it = textures_js.begin(); it != textures_js.end(); it++) {
			if (it.key() != "particle") {
				string texture_path = it.value();
				if (texture_path.find(MC_PREFIX) == 0)
					texture_path = texture_path.substr(MC_PREFIX.size());
				if (texture_path.find("#") == 0) {
					texture_path = texture_path.substr(1);
					if (textures.find(texture_path) == textures.end())
						printf("[Warning] Texture %s not found\n", texture_path.c_str());
					textures[it.key()] = textures[texture_path];
				} else {
					texture_path = "../minecraft/textures/" + texture_path + ".png";
					if (texture_cache.find(texture_path) == texture_cache.end())
						texture_cache[texture_path] = LoadTexture2D(texture_path.c_str());
					if (textures.find(it.key()) == textures.end())
						textures[it.key()] = texture_cache[texture_path];
				}
			}
		}

		json elements_js = block_js["elements"];
		if (cuboids.size() > 0 && elements_js.size() > 0) return; // Already loaded
		for (json::iterator it = elements_js.begin(); it != elements_js.end(); it++) {
			json element_js = *it;
			json from_js = element_js["from"];
			json to_js = element_js["to"];
			vec3 from = vec3(from_js[0], from_js[1], from_js[2]);
			vec3 to = vec3(to_js[0], to_js[1], to_js[2]);
			Cuboid* cuboid = new Cuboid(from, to);

			quat rotation = quat(1, 0, 0, 0);
			if (element_js.find("rotation") != element_js.end()) {
				json rotation_js = element_js["rotation"];
				float angle = rotation_js["angle"];
				string axis_str = rotation_js["axis"];
				//json origin_js = rotation_js["origin"];
				//vec3 origin = vec3(origin_js[0], origin_js[1], origin_js[2]);
				if (axis_str == "x") {
					vec3 axis = vec3(1, 0, 0);
					rotation = angleAxis(radians(angle), axis);
				} else if (axis_str == "y") {
					vec3 axis = vec3(0, 1, 0);
					rotation = angleAxis(radians(angle), axis);
				} else if (axis_str == "z") {
					vec3 axis = vec3(0, 0, 1);
					rotation = angleAxis(radians(angle), axis);
				}
			}

			json faces_js = element_js["faces"];
			for (json::iterator it = faces_js.begin(); it != faces_js.end(); it++) {
				json face_js = *it;
				string face_name = it.key();
				string texture_str = face_js["texture"];
				if (texture_str.find("#") == 0)
					texture_str = texture_str.substr(1);
				if (textures.find(texture_str) == textures.end()) {
					printf("[Warning] Texture %s not found for face %s\n", texture_str.c_str(), face_name.c_str());
					break;
				}
				GLuint texture_id = textures[texture_str];
				bool tinted = face_js.find("tintindex") != face_js.end();
				vec4 uv = vec4(0, 0, 16, 16);
				int texture_rotation = -1;
				if (face_js.find("uv") != face_js.end()) {
					json uv_js = face_js["uv"];
					uv = vec4(uv_js[0], uv_js[1], uv_js[2], uv_js[3]);
					texture_rotation = 0;
				}
				if (face_js.find("rotation") != face_js.end())
					texture_rotation = face_js["rotation"];
				cuboid->addFace(face_name, rotation, texture_id, uv, texture_rotation, tinted);
			}
			cuboids.push_back(cuboid);
		}

		if (block_js.find("parent") != block_js.end()) {
			string parent_path = block_js["parent"];
			if (parent_path.find(MC_PREFIX) == 0)
				parent_path = parent_path.substr(MC_PREFIX.size());
			if (parent_path != "block/block") {
				parent_path = "../minecraft/models/" + parent_path + ".json";
				loadFile(parent_path);
			}
		}
	}
public:
	MC_Block(const char* block_id) {
		string path = "../minecraft/models/" + string(block_id) + ".json";
		loadFile(path);	
	}
	void setTransform(vec3 pos, quat rot) {
		for (vector<Cuboid*>::iterator it = cuboids.begin(); it != cuboids.end(); it++)
			(*it)->setTransform(pos, rot);
	}
	void draw(Shader& shader, Camera& camera) {
		for (vector<Cuboid*>::iterator it = cuboids.begin(); it != cuboids.end(); it++)
			(*it)->draw(shader, camera);
	}
	~MC_Block() {
		for (vector<Cuboid*>::iterator it = cuboids.begin(); it != cuboids.end(); it++)
			delete *it;
	}
};

struct VariantInfo {
	string model;
	//string facing;
	//unordered_set<string> props;
	quat rotation;
};
/*
bool operator==(const VariantInfo& lhs, const VariantInfo& rhs) {
	return lhs.model == rhs.model && lhs.facing == rhs.facing && lhs.props == rhs.props;
}
*/
map<string, MC_Block*> blocks_cache;

class BlockVariant {
private:
	vector<VariantInfo> variants;
	vector<VariantInfo> multipart;

	VariantInfo readVariant(json variant_js) {
		const string MC_PREFIX = "minecraft:";
		string block_id = variant_js["model"];
		if (block_id.find(MC_PREFIX) == 0)
			block_id = block_id.substr(MC_PREFIX.size());
		if (blocks_cache.find(block_id) == blocks_cache.end())
			blocks_cache[block_id] = new MC_Block(block_id.c_str());

		quat rotation = quat(1, 0, 0, 0);
		if (variant_js.find("x") != variant_js.end()) {
			float angle = variant_js["x"];
			vec3 axis = vec3(1, 0, 0);
			rotation = angleAxis(radians(angle), axis);
		}
		if (variant_js.find("y") != variant_js.end()) {
			float angle = variant_js["y"];
			vec3 axis = vec3(0, 1, 0);
			rotation = rotation * angleAxis(radians(angle), axis);
		}
		if (variant_js.find("z") != variant_js.end()) {
			float angle = variant_js["z"];
			vec3 axis = vec3(0, 0, 1);
			rotation = rotation * angleAxis(radians(angle), axis);
		}

		VariantInfo info;
		info.model = block_id;
		info.rotation = rotation;
		return info;
	}

	void loadFile(string path) {
		ifstream file(path);
		if (!file.is_open()) {
			printf("[ERROR] File %s not found\n", path.c_str());
			return;
		}
		json block_js = json::parse(file);

		json variants_js = block_js["variants"];
		for (json::iterator it = variants_js.begin(); it != variants_js.end(); it++) {
			json variant_js = it.value();
			if (variant_js.is_object()) {
				VariantInfo info = readVariant(variant_js);
				variants.push_back(info);	
			} else if (variant_js.is_array()) {
				for (json::iterator it = variant_js.begin(); it != variant_js.end(); it++) {
					VariantInfo info = readVariant(*it);
					variants.push_back(info);
				}
			}
		}

		json multipart_js = block_js["multipart"];
		for (json::iterator it = multipart_js.begin(); it != multipart_js.end(); it++) {
			json part_js = it.value();
			json variant_js = part_js["apply"];
			if (variant_js.is_object()) {
				VariantInfo info = readVariant(variant_js);
				multipart.push_back(info);	
			} else if (variant_js.is_array()) {
				for (json::iterator it = variant_js.begin(); it != variant_js.end(); it++) {
					VariantInfo info = readVariant(*it);
					multipart.push_back(info);
				}
			}
		}
	}
public:
	BlockVariant(const char* block_id) {
		string path = "../minecraft/blockstates/" + string(block_id) + ".json";
		loadFile(path);
	}
	unsigned int getVariantCount() {
		bool is_multipart = multipart.size() > 0;
		return variants.size() + (is_multipart ? 1 : 0);
	}
	void draw(Shader& shader, Camera& camera, unsigned int index, vec3 pos) {
		if (index < variants.size()) {
			VariantInfo	info = variants[index];
			MC_Block* block = blocks_cache[info.model];
			block->setTransform(pos, info.rotation);
			block->draw(shader, camera);
		} else {
			for (vector<VariantInfo>::iterator it = multipart.begin(); it != multipart.end(); it++) {
				VariantInfo	info = *it;
				MC_Block* block = blocks_cache[info.model];
				block->setTransform(pos, info.rotation);
				block->draw(shader, camera);
			}
		}
	}
};

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("Minecraft Renderer");
	Shader mc_shader("shaders/minecraft_vert.glsl", "shaders/minecraft_frag.glsl");

	vector<string> blocks = {
		"anvil",
		"bamboo",
		"beacon",
		"bricks",
		"cauldron",
		"dropper",
		"creeper_head",
		"detector_rail",
		"dragon_egg",
		"end_portal_frame",
		"glass",
		"jukebox",
		"lectern",
		"magenta_glazed_terracotta",
		"note_block",
		"oak_fence",
		"observer",
		"redstone_ore",
		"short_grass",
		"tall_grass",
		"blue_banner",
	};

	vector<BlockVariant*> block_variants;
	for (vector<string>::iterator it = blocks.begin(); it != blocks.end(); it++)
		block_variants.push_back(new BlockVariant(it->c_str()));

	Camera camera(vec3(0, 2.5, 10));
	glfwSetWindowUserPointer(window, &camera);
	glfwSetKeyCallback(window, key_press_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		camera.handleInputs(window);

		glClearColor(0.35, 0.54, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		mc_shader.Use();
		unsigned int i = 0;
		const int WIDTH = 8;
		for (vector<BlockVariant*>::iterator it = block_variants.begin(); it != block_variants.end(); it++) {
			for (unsigned int j = 0; j < (*it)->getVariantCount(); j++) {
				(*it)->draw(mc_shader, camera, j, vec3(2.0f * (i % WIDTH), 0.0f, -2.0f * (i / WIDTH)));
				i++;
			}
		}
		glDisable(GL_BLEND);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
