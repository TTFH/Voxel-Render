uniform vec2 offset;
uniform sampler2D diffuse;

varying vec2 texCoord;

#ifdef VERTEX
attribute vec2 aPos;

void main() {
	texCoord = aPos;
	gl_Position = vec4(aPos / 2.0 + offset, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT
void main() {
	gl_FragColor = texture(diffuse, texCoord);
}
#endif
