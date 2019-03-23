#version 330
         
smooth in vec3 f_norm;
smooth in vec2 f_texcoord;

uniform sampler2D tex;

out vec4 color;

void main(void)
{
	color = texture(tex, vec2(f_texcoord.x, 1.0 - f_texcoord.y));
}
