#include <math.h>

#include "ebo.h"
#include "vbo.h"
#include "utils.h"
#include "render_boundary.h"

#include <glm/gtc/matrix_transform.hpp>

// 1--------3--------5	y = 30
// | \      | \      |
// |   \    |   \    |	uv.x = dist(a, b)
// |   + \  |   + \  |
// 0--------2--------4	y = -5
struct BoundaryVertex {
	vec3 position;
	vec2 texcoord;
};

BoundaryRender::BoundaryRender(vector<vec2> vertices_2d) {
	vector<GLuint> indices;
	vector<BoundaryVertex> vertices;

	float dist = 0.0f;
	for (unsigned int i = 0; i < vertices_2d.size(); i++) {
		vec2 p0 = vertices_2d[i];
		vec2 p1 = i < vertices_2d.size() - 1 ? vertices_2d[i + 1] : vertices_2d[0];
		vertices.push_back({ vec3(p0.x, -5.0f, p0.y), vec2(dist, -5.0f) });
		vertices.push_back({ vec3(p0.x, 30.0f, p0.y), vec2(dist, 30.0f) });
		dist += distance(p0, p1);
	}
	// The first vertex is included again with a different uv
	vec2 p0 = vertices_2d[0];
	vertices.push_back({ vec3(p0.x, -5.0f, p0.y), vec2(dist, -5.0f) });
	vertices.push_back({ vec3(p0.x, 30.0f, p0.y), vec2(dist, 30.0f) });

	int index = 0;
	for (unsigned int i = 0; i < vertices_2d.size(); i++) {
		indices.push_back(index + 1);
		indices.push_back(index);
		indices.push_back(index + 2);

		indices.push_back(index + 1);
		indices.push_back(index + 2);
		indices.push_back(index + 3);
		index += 2;
	}

	VBO vbo(vertices);
	EBO ebo(indices);

	vao.LinkAttrib(0, 3, GL_FLOAT, sizeof(BoundaryVertex), (GLvoid*)0);						// Vertex position
	vao.LinkAttrib(1, 2, GL_FLOAT, sizeof(BoundaryVertex), (GLvoid*)(3 * sizeof(GLfloat)));	// Texture coord

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	index_count = indices.size();
	boundary_texture = LoadTexture2D("textures/boundary.png");
}

void BoundaryRender::draw(Shader& shader, Camera& camera) {
	shader.PushMatrix("uMvpMatrix", camera.vpMatrix);

	shader.PushTexture2D("uTexture", boundary_texture, 1);
	shader.PushFloat("uVisibleDistance", 100.0f);
	shader.PushVec4("uColor", vec4(2.0f, 1.0f, 0.0f, 1.0f));
	shader.PushVec3("uFocus0", camera.position);
	shader.PushFloat("uInvFar", 1.0f / camera.FAR_PLANE);

	vao.Bind();
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	vao.Unbind();
}
