uniform vec2 offset;
uniform sampler2D diffuse0;

varying vec2 texCoord;

#ifdef VERTEX
attribute vec2 aPos;

void main() {
	gl_Position = vec4(aPos / 2.0 + offset, 0.0, 1.0);
	texCoord = aPos;
}
#endif

#ifdef FRAGMENT
void main() {
	gl_FragColor = texture(diffuse0, texCoord);
}
#endif
