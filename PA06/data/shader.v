#version 330
         
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;
layout (location = 2) in vec2 v_texcoord;

smooth out vec3 f_norm;
smooth out vec2 f_texcoord;

uniform mat4 model, view, proj;

void main(void)
{
	gl_Position = proj * view * model * vec4(v_pos, 1.0);
	f_norm = v_norm;
	f_texcoord = v_texcoord;
}
