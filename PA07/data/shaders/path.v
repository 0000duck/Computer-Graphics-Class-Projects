
#version 330

layout (location = 0) in vec3 v_pos;

uniform mat4 model, view, proj;

void main() {
	gl_Position = proj * view * model * vec4(v_pos, 1.0);
}
