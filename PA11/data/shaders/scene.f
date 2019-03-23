
#version 330

smooth in vec2 f_texcoord;

out vec4 out_color;

uniform sampler2D tex;

void main() {
	out_color = texture(tex, f_texcoord);
}
