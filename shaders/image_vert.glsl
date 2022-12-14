#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec3 scale;
uniform vec3 offset;

out vec2 tex_coord;

void main() {
	gl_Position = vec4(aPos * scale + offset, 1.0);
	tex_coord = aTexCoord * scale.xy + (offset.xy / 2.0) - (scale.xy / 2.0) + 0.5; // It works ¯\_(ツ)_/¯
}
