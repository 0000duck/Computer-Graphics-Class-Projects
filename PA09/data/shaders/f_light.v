
#version 330

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;
layout (location = 2) in vec2 v_texcood;

smooth out vec2 f_texcoord;
smooth out vec3 f_norm;
smooth out vec4 f_pos;

uniform mat4 model, view, proj;

void main() {

	f_texcoord = v_texcood;
	f_norm = normalize(inverse(transpose(mat3(model))) * v_norm);
	f_pos = model * vec4(v_pos, 1.0);

	gl_Position = proj * view * model * vec4(v_pos, 1.0);
}
