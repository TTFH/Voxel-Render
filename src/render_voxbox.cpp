#include "ebo.h"
#include "vbo.h"
#include "render_voxbox.h"

//   6--------7
//  /|       /|
// 2--------3 |
// | |      | |
// | 4------|-5
// |/       |/
// 0--------1
static const GLfloat cube_vertices[] = {
	// Position  // Normal
	0, 0, 0,	 0,  0, -1,
	0, 1, 0,	 0,  0, -1,
	1, 0, 0,	 0,  0, -1,
	1, 1, 0,	 0,  0, -1,

	0, 0, 0,	 0, -1,  0,
	1, 0, 0,	 0, -1,  0,
	0, 0, 1,	 0, -1,  0,
	1, 0, 1,	 0, -1,  0,

	0, 0, 0,	-1,  0,  0,
	0, 0, 1,	-1,  0,  0,
	0, 1, 0,	-1,  0,  0,
	0, 1, 1,	-1,  0,  0,

	1, 1, 1,	 0,  0,  1,
	0, 1, 1,	 0,  0,  1,
	1, 0, 1,	 0,  0,  1,
	0, 0, 1,	 0,  0,  1,

	1, 1, 1,	 0,  1,  0,
	1, 1, 0,	 0,  1,  0,
	0, 1, 1,	 0,  1,  0,
	0, 1, 0,	 0,  1,  0,

	1, 1, 1,	 1,  0,  0,
	1, 0, 1,	 1,  0,  0,
	1, 1, 0,	 1,  0,  0,
	1, 0, 0,	 1,  0,  0,
};

static const GLuint cube_indices[] = {
	 0,  1,  2,
	 2,  1,  3,
	 4,  5,  6,
	 6,  5,  7,
	 8,  9, 10,
	10,  9, 11,
	12, 13, 14,
	14, 13, 15,
	16, 17, 18,
	18, 17, 19,
	20, 21, 22,
	22, 21, 23,
};

VoxboxRender::VoxboxRender(vec3 size, vec3 color) {
	this->size = size;
	this->color = color;
	VBO vbo(cube_vertices, sizeof(cube_vertices));
	EBO ebo(cube_indices, sizeof(cube_indices));
	vao.LinkAttrib(0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0);					 // Vertex position
	vao.LinkAttrib(1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Normal
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void VoxboxRender::setColor(vec3 color) {
	this->color = color;
}

void VoxboxRender::setWorldTransform(vec3 position, quat rotation) {
	this->position = position;
	this->rotation = rotation;
}

void VoxboxRender::draw(Shader& shader, Camera& camera) {
	shader.PushMatrix("camera", camera.vpMatrix);

	shader.PushVec3("color", color);
	shader.PushVec3("size", size);
	shader.PushFloat("scale", 0); // SM flag not a voxel

	mat4 pos = translate(mat4(1.0f), position);
	mat4 rot = mat4_cast(rotation);
	shader.PushMatrix("position", pos);
	shader.PushMatrix("rotation", rot);

	vao.Bind();
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}
// TODO: remove
vector<vec3> VoxboxRender::getOBBCorners() {
	vector<vec3> corners;
	vec3 size_meters = size * 0.1f;
	vector<vec3> local_corners = {
		vec3(0, 0, 0),
		vec3(size_meters.x, 0, 0),
		vec3(size_meters.x, size_meters.y, 0),
		vec3(0, size_meters.y, 0),
		vec3(0, 0, size_meters.z),
		vec3(size_meters.x, 0, size_meters.z),
		vec3(size_meters.x, size_meters.y, size_meters.z),
		vec3(0, size_meters.y, size_meters.z)
	};
	for (vector<vec3>::iterator it = local_corners.begin(); it != local_corners.end(); it++) {
		vec3 local = *it;
		vec3 rotated = rotation * local;
		vec3 world = rotated + position;
		corners.push_back(world);
	}
	return corners;
}
// TODO: remove
bool VoxboxRender::isInFrustum(const Frustum& frustum) {
	vector<vec3> corners = getOBBCorners();
	vector<Plane> planes = {
		frustum.near, frustum.far,
		frustum.left, frustum.right,
		frustum.top, frustum.bottom
	};
	for (vector<Plane>::iterator plane = planes.begin(); plane != planes.end(); plane++) {
		int outside_count = 0;
		for (vector<vec3>::iterator corner = corners.begin(); corner != corners.end(); corner++) {
			float dist = dot(plane->normal, *corner) + plane->distance;
			if (dist < 0) outside_count++;
		}
		if (outside_count == 8)
			return false;
	}
	return true;
}
// TODO: remove
void VoxboxRender::handleInputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		position.y += 0.025f;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		position.y -= 0.025f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		position += vec3(rotation * vec4(0, 0, 0.1f, 0));
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		position += vec3(rotation * vec4(0, 0, -0.05f, 0));
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		rotation = angleAxis(radians(-0.5f), vec3(0, 1, 0)) * rotation;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		rotation = angleAxis(radians(0.5f), vec3(0, 1, 0)) * rotation;
}
