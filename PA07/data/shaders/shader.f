
#version 330

smooth in vec2 f_texcoord;
smooth in vec3 f_norm;
smooth in vec3 f_pos;

out vec4 color;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float ambientStrength;

uniform sampler2D tex;

void main() {
	vec3 ambient = ambientStrength * lightColor;
	vec3 lightDir = normalize(lightPos - f_pos);
	float diff = clamp(dot(f_norm, lightDir), 0.0, 1.0);
	vec3 diffuse = diff * lightColor;
	color = vec4(ambient + diffuse, 1.0f) * texture(tex, f_texcoord);
}
