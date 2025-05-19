#ifdef VERTEX
vec2 getFullScreenPos(int vertexId) {
	const vec2 vertexPos[6] = vec2[6] (
		vec2( -1.0f, -1.0f ),
		vec2(  1.0f, -1.0f ),
		vec2(  1.0f,  1.0f ),
		vec2(  1.0f,  1.0f ),
		vec2( -1.0f,  1.0f ),
		vec2( -1.0f, -1.0f )
	);
	return vertexPos[vertexId];
}

vec2 getFullScreenTexCoord(int vertexId) {
	const vec2 vertexTexCoord[6] = vec2[6] (
		vec2( 0.0f, 0.0f ),
		vec2( 1.0f, 0.0f ),
		vec2( 1.0f, 1.0f ),
		vec2( 1.0f, 1.0f ),
		vec2( 0.0f, 1.0f ),
		vec2( 0.0f, 0.0f )
	);
	return vertexTexCoord[vertexId];
}

out vec2 vTexCoord;

void main() {
	gl_Position = vec4(getFullScreenPos(gl_VertexID), 0.0, 1.0);
	vTexCoord = getFullScreenTexCoord(gl_VertexID);
}
#endif

#ifdef FRAGMENT
uniform sampler2D uTexture;

in vec2 vTexCoord;

out vec4 FragColor;

void main() {
	FragColor = texture(uTexture, vTexCoord);
}
#endif
