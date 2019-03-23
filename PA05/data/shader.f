#version 330
         
smooth in vec3 f_norm;
smooth in vec2 f_texcoord;

out vec4 color;

void main(void)
{
	color = vec4(f_norm, 1.0);
}
