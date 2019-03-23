
#version 330

smooth in vec4 f_lighting;
smooth in vec2 f_texcoord;

out vec4 out_color;

uniform sampler2D tex;

void main() {
	out_color = f_lighting * texture(tex, f_texcoord);
}
