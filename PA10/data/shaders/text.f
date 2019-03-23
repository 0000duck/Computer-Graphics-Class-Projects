
#version 330 core

uniform sampler2D font;

in vec2 uv_f;

out vec4 color;

void main() {
	
	color = texture(font, uv_f);
}

