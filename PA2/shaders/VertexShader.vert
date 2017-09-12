#version 330

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;

smooth out vec3 color;

//every shader shares these matrices
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix; 

void main(void)
{
  //1.0 is difference between vectors and points - vectors = 0
	vec4 v = vec4(v_position, 1.0);
	gl_Position = (projectionMatrix * viewMatrix * modelMatrix) * v;
	color = v_color;
}
