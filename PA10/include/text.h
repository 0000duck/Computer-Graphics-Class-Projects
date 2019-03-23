#ifndef TEXT_H
#define TEXT_H

#include <vector>
#include <string>
#include "shader.h"
#include "graphics.h"

class Text {
public:
	Text();
	~Text();

	// initialize with a specified font
	bool Initialize(int width, int height, std::string font_path);

	// begin 3D text definitions
	void Begin();
	// render definitions specified since Begin()
	void End(Graphics* g);

	// add text at a position along an axis at a point size
	float AddText(std::string text, glm::vec3 pos, glm::vec3 axis, float fp);

private:
	const float font_point = 40.0f;
	const int TEX_SIZE = 512;
	const int char_begin = 32;
	const int char_end = 127;
	const int num_chars = char_end - char_begin;

	// 3D textured vertex representation (without normal)
	struct vertex {
		glm::vec3 pos;
		glm::vec2 uv;
	};
	// character representation in font texture + kerning info
	struct glyph {
		int codepoint;
		int x1, y1, x2, y2;
		float xoff1, yoff1;
		float xoff2, yoff2;
		float advance;
	};

	int w, h;

	Shader* m_shader;
	
	float line_height;

	std::vector<vertex>  	Verticies;
	std::vector<GLuint> 	Indicies; 
	std::vector<glyph>		glyphs;

	GLuint VAO, VBO, IBO, texture, transformMatLoc;
};

#endif