#version 330

//matches name of vertex shader
smooth in vec3 color;

out vec4 frag_color;

void main(void)
{
	frag_color = vec4(color.rgb, 1.0);
}
