#include <map>
#include <vector>
#include <fstream>
#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "src/vao.h"
#include "src/ebo.h"
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
	vec3 tint_color = vec3(1, 1, 1);

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
	void setWorldTransform(vec3 pos, quat rot) {
		world_position = pos;
		world_rotation = rot;
	}
	void setUV(vec2 min, vec2 max, int rot) {
		uv_min = min;
		uv_max = max;
		texture_rotation = rot;
	}
	void setTint(vec3 color) {
		tint_color = color;
	}
	void draw(Shader& shader, Camera& camera) {
		vao.Bind();
		shader.PushMatrix("camera", camera.vpMatrix);
		shader.PushTexture("diffuse", texture, 0);
		shader.PushVec2("size", size);
		shader.PushVec3("tint_color", tint_color);
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
	vec3 offset;
	vector<Quad*> quads;
	vec3 from;
	vec3 to;
public:
	Cuboid(vec3 from, vec3 to, vec3 world_pos = vec3(0, 0, 0)) {
		this->from = from;
		this->to = to;
		this->size = to - from;
		this->offset = (from + to) / 2.0f + world_pos;
	}
	void addFace(string orientation, bool tinted, vec4 uv, int tex_rot, GLuint texture) {
		Quad* quad = new Quad(texture);
		vec2 face_size = vec2(16, 16);
		vec3 position = offset;
		quat rotation = quat(1, 0, 0, 0);
		if (orientation == "up") {
			face_size = vec2(size.x, size.z);
			position += vec3(0, size.y / 2.0f, 0);
			rotation = angleAxis(radians(-90.0f), vec3(1, 0, 0));
			if (tex_rot == -1)
				uv = vec4(from.x, 16.0f - to.z, to.x, 16.0f - from.z);
		} else if (orientation == "down") {
			face_size = vec2(size.x, size.z);
			position += vec3(0, -size.y / 2.0f, 0);
			rotation = angleAxis(radians(90.0f), vec3(1, 0, 0));
			if (tex_rot == -1)
				uv = vec4(from.x, from.z, to.x, to.z);
		} else if (orientation == "north") {
			face_size = vec2(size.x, size.y);
			position += vec3(0, 0, -size.z / 2.0f);
			rotation = angleAxis(radians(180.0f), vec3(0, 1, 0));
			if (tex_rot == -1)
				uv = vec4(from.x, from.y, to.x, to.y);
		} else if (orientation == "south") {
			face_size = vec2(size.x, size.y);
			position += vec3(0, 0, size.z / 2.0f);
			if (tex_rot == -1)
				uv = vec4(from.x, from.y, to.x, to.y);
		} else if (orientation == "east") {
			face_size = vec2(size.z, size.y);
			position += vec3(size.x / 2.0f, 0, 0);
			rotation = angleAxis(radians(90.0f), vec3(0, 1, 0));
			if (tex_rot == -1)
				uv = vec4(from.z, from.y, to.z, to.y);
		} else if (orientation == "west") {
			face_size = vec2(size.z, size.y);
			position += vec3(-size.x / 2.0f, 0, 0);
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

		quad->setSize(face_size);
		quad->setTransform(position, rotation);
		quad->setUV(vec2(uv.x, uv.y) / 16.0f, vec2(uv.z, uv.w) / 16.0f, tex_rot);
		if (tinted) quad->setTint(vec3(0.57, 0.74, 0.35));
		quads.push_back(quad);
	}
	void setRotation(float angle, string axis_str, vec3 origin) {
		if (axis_str == "x") {
			vec3 axis = vec3(1, 0, 0);
			vec3 world_pos = vec3(0, -origin.z / 4, origin.y / 2);
			quat world_rot = angleAxis(radians(angle), axis);
			for (vector<Quad*>::iterator it = quads.begin(); it != quads.end(); it++)
				(*it)->setWorldTransform(world_pos, world_rot);
		} else
			printf("[Warning] Unsupported rotation axis %s\n", axis_str.c_str());
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
	vec3 world_pos = vec3(0, 0, 0);
	vector<Cuboid*> cuboids;
	map<string, GLuint> textures;

	void loadFile(const char* path) {
		ifstream file(path);
		if (!file.is_open()) {
			printf("[ERROR] File %s not found\n", path);
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
					if (textures.find(texture_path) == textures.end()) {
						printf("[Warning] Texture %s not found\n", texture_path.c_str());
						break;
					}
					textures[it.key()] = textures[texture_path];
				} else {
					texture_path += ".png";
					if (texture_cache.find(texture_path) == texture_cache.end())
						texture_cache[texture_path] = LoadTexture(texture_path.c_str(), GL_RGBA);
					textures[it.key()] = texture_cache[texture_path];
				}
			}
		}

		json elements_js = block_js["elements"];
		if (cuboids.size() > 0 && elements_js.size() > 0) {
			printf("[Warning] Skipping %s, elements already defined.\n", path);
			return;
		}
		for (json::iterator it = elements_js.begin(); it != elements_js.end(); it++) {
			json element_js = *it;
			json from_js = element_js["from"];
			json to_js = element_js["to"];
			vec3 from = vec3(from_js[0], from_js[1], from_js[2]);
			vec3 to = vec3(to_js[0], to_js[1], to_js[2]);
			Cuboid* cuboid = new Cuboid(from, to, world_pos);
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
				cuboid->addFace(face_name, tinted, uv, texture_rotation, texture_id);
			}

			if (element_js.find("rotation") != element_js.end()) {
				json rotation_js = element_js["rotation"];
				float angle = rotation_js["angle"];
				json origin_js = rotation_js["origin"];
				vec3 origin = vec3(origin_js[0], origin_js[1], origin_js[2]);
				string axis = rotation_js["axis"];
				cuboid->setRotation(angle, axis, origin);
			}
			cuboids.push_back(cuboid);
		}

		if (block_js.find("parent") != block_js.end()) {
			string parent_path = block_js["parent"];
			if (parent_path.find(MC_PREFIX) == 0)
				parent_path = parent_path.substr(MC_PREFIX.size());
			if (parent_path != "block/block") {
				parent_path += ".json";
				loadFile(parent_path.c_str());
			}
		}
	}
public:
	MC_Block(const char* path, vec3 world_pos = vec3(0, 0, 0)) {
		this->world_pos = world_pos;
		// TODO: build path from block name
		loadFile(path);	
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

int main(/*int argc, char* argv[]*/) {
	GLFWwindow* window = InitOpenGL("Minecraft Renderer");
	Shader mc_shader("shaders/minecraft_vert.glsl", "shaders/minecraft_frag.glsl");
	Camera camera(vec3(0, 2.5, 10));

	vector<MC_Block*> blocks;
	blocks.push_back(new MC_Block("block/anvil.json"));
	blocks.push_back(new MC_Block("block/bricks.json", vec3(32, 0, 0)));
	blocks.push_back(new MC_Block("block/cauldron.json", vec3(64, 0, 0)));
	blocks.push_back(new MC_Block("block/crafting_table.json", vec3(96, 0, 0)));
	blocks.push_back(new MC_Block("block/grass_block.json", vec3(128, 0, 0)));
	blocks.push_back(new MC_Block("block/lectern.json", vec3(160, 0, 0)));
	blocks.push_back(new MC_Block("block/oak_stairs.json", vec3(192, 0, 0)));
	blocks.push_back(new MC_Block("block/glass.json", vec3(224, 0, 0)));
	blocks.push_back(new MC_Block("block/magenta_glazed_terracotta.json", vec3(256, 0, 0)));

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

		mc_shader.Use();
		for (vector<MC_Block*>::iterator it = blocks.begin(); it != blocks.end(); it++)
			(*it)->draw(mc_shader, camera);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
