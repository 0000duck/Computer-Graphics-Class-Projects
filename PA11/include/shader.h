#ifndef SHADER_H
#define SHADER_H

#include <vector>

#include "graphics_headers.h"

class Shader {
	
public:
	Shader();
	~Shader();

	// setup openGL handles
	bool Initialize();
	// bind shader
	void Enable();
	// add vertex/fragment shader from file
	bool AddShader(GLenum ShaderType, std::string path);
	// compile shader
	bool Finalize();
	// get location of uniform variable
	GLint GetUniformLocation(const char* pUniformName);

private:
	GLuint m_shaderProg;    
	std::vector<GLuint> m_shaderObjList;
};

#endif  /* SHADER_H */
