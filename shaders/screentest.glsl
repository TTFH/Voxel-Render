uniform sampler2D uTexture;

varying vec2 vTexCoord;

#ifdef VERTEX
attribute vec2 aPosition;
attribute vec2 aTexCoord;

void main() {
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT
void main() {
	gl_FragColor = texture(uTexture, vTexCoord);
}
#endif
