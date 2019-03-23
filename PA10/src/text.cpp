#include "text.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <fstream>

Text::Text() {

	m_shader = nullptr;
	VAO = VBO = IBO = 0;
	w = h = 0;
	transformMatLoc = 0;
}

Text::~Text() {

	if(m_shader) {

		delete m_shader;
		m_shader = nullptr;
	}

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteTextures(1, &texture);
}

bool Text::Initialize(int width, int height, std::string font_path) {

	w = width;
	h = height;

	// load shader 
	{
		m_shader = new Shader();
		if(!m_shader->Initialize()) {

			printf("Shader Failed to Initialize\n");
			return false;
		}
		
		// Add the vertex shader
		if(!m_shader->AddShader(GL_VERTEX_SHADER, "../data/shaders/text.v")) {
		
			printf("Vertex Shader failed to Initialize\n");
			return false;
		}
		
		// Add the fragment shader
		if(!m_shader->AddShader(GL_FRAGMENT_SHADER, "../data/shaders/text.f")) {
		
			printf("Fragment Shader failed to Initialize\n");
			return false;
		}
		
		// Connect the program
		if(!m_shader->Finalize()) {

			printf("Program to Finalize\n");
			return false;
		}

		transformMatLoc = m_shader->GetUniformLocation("transform");
		if (transformMatLoc == INVALID_UNIFORM_LOCATION) {
			
			printf("transformMatLoc not found\n");
			return false;
		}
	}

	// initiaize OpenGL buffers
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenTextures(1, &texture);
	}

	// load and rasterize the font
	{
		std::ifstream fin(font_path, std::ios::binary | std::ios::ate);
		
		if(!fin.good()) {
			printf("Failed to load font file %s\n", font_path.c_str());
			return false;
		}

		std::streamsize size = fin.tellg();
		fin.seekg(0, std::ios::beg);

		std::vector<char> data((int)size);
		fin.read(data.data(), size);

		stbtt_fontinfo font_info;
		stbtt_pack_context pack_context;
		int ascent, descent, linegap;

		stbtt_InitFont(&font_info, (const unsigned char*)data.data(), 0);
		stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &linegap);
		float scale = stbtt_ScaleForPixelHeight(&font_info, font_point);

		line_height = scale * (ascent - descent + linegap);

		unsigned char* baked_bitmap = new unsigned char[TEX_SIZE * TEX_SIZE]();

		// Two pixel-padding needed for scaling (likely because of FP rounding error)
		stbtt_PackBegin(&pack_context, baked_bitmap, TEX_SIZE, TEX_SIZE, 0, 2, nullptr);
		stbtt_PackSetOversampling(&pack_context, 1, 1);

		stbtt_packedchar* chars = new stbtt_packedchar[num_chars]();

		stbtt_PackFontRange(&pack_context, (unsigned char*)data.data(), 0, STBTT_POINT_SIZE(font_point), char_begin, num_chars, chars);
		stbtt_PackEnd(&pack_context);

		for(int i = 0; i < num_chars; i++) {
			glyph g;
			g.codepoint = char_begin + i;
			g.x1      	= chars[i].x0;
			g.y1      	= chars[i].y0;
			g.x2      	= chars[i].x1;
			g.y2      	= chars[i].y1;
			g.xoff1   	= chars[i].xoff;
			g.yoff1   	= chars[i].yoff;
			g.xoff2   	= chars[i].xoff2;
			g.yoff2   	= chars[i].yoff2;
			g.advance 	= chars[i].xadvance;
			glyphs.push_back(g);
		}

		// load rasterized texture
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEX_SIZE, TEX_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, baked_bitmap);
		GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,uv));
		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);

		delete[] baked_bitmap;
		delete[] chars;
	}

	return true;
}

void Text::Begin() {

	Verticies.clear();
	Indicies.clear();
}

void Text::End(Graphics* g) {

	m_shader->Enable();

	glm::mat4 transform = g->GetProj() * g->GetView();
	glUniformMatrix4fv(transformMatLoc, 1, GL_FALSE, glm::value_ptr(transform));

	if(!Verticies.size()) {

		return;
	}

	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * Verticies.size(), &Verticies[0], GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indicies.size(), &Indicies[0], GL_DYNAMIC_DRAW);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_DST_ALPHA, GL_DST_ALPHA);
	glDrawElements(GL_TRIANGLES, Indicies.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

float Text::AddText(std::string text, glm::vec3 pos, glm::vec3 axis, float fp) {

	axis = glm::normalize(axis) / fp;

	glm::vec3 basis(1,0,0);
	float angle = acos(glm::dot(basis, axis) / (glm::length(basis) * glm::length(axis)));
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle + glm::radians(180.0f), glm::vec3(0,1,0));
	rotate = glm::rotate(rotate, glm::radians(-30.0f), glm::vec3(0,0,1));

	for(char c : text) {

		// only ASCII 
		int codepoint = c;

		unsigned int idx = Verticies.size();
		
		assert(codepoint >= char_begin && codepoint < char_end);
		glyph g = glyphs[codepoint - char_begin];

		float w = TEX_SIZE;
		float h = TEX_SIZE;
		glm::vec2 t_tlc(g.x1/w, g.y1/h);
		glm::vec2 t_brc(g.x2/w, g.y2/h);
		glm::vec2 t_trc(g.x2/w, g.y1/h);
		glm::vec2 t_blc(g.x1/w, g.y2/h);

		glm::vec4 tlc = glm::vec4(g.xoff1 / fp, g.yoff1 / fp, 0.0f, 1.0f) * rotate;
		glm::vec4 brc = glm::vec4(g.xoff2 / fp, g.yoff2 / fp, 0.0f, 1.0f) * rotate;
		glm::vec4 trc = glm::vec4(g.xoff2 / fp, g.yoff1 / fp, 0.0f, 1.0f) * rotate;
		glm::vec4 blc = glm::vec4(g.xoff1 / fp, g.yoff2 / fp, 0.0f, 1.0f) * rotate;

		Verticies.push_back({pos + glm::vec3(tlc / tlc.w), t_blc}); 
 		Verticies.push_back({pos + glm::vec3(blc / blc.w), t_tlc});
 		Verticies.push_back({pos + glm::vec3(trc / trc.w), t_brc});
 		Verticies.push_back({pos + glm::vec3(brc / brc.w), t_trc}); 		

 		Indicies.push_back(idx);
 		Indicies.push_back(idx + 1);
 		Indicies.push_back(idx + 2);
 		Indicies.push_back(idx + 1);
 		Indicies.push_back(idx + 2);
 		Indicies.push_back(idx + 3);

		pos += axis * g.advance;
	}

	return line_height;
}
