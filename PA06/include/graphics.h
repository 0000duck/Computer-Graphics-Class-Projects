#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "scene.h"

class Graphics {

public:
	Graphics();
	~Graphics();

	bool Initialize(int width, int height, std::vector<std::string> args);
	
	void Update(unsigned int dt);
	void Render(int w, int h);

	void EvtCamera(int dx, int dy);

private:
	GLuint VAO;

	std::string ErrorString(GLenum error);

	Camera *m_camera;
	Shader *m_shader;
	Scene  *m_scene;
	
	std::string scene_path;
	bool load_failed;

	GLint m_projectionMatrix;
	GLint m_viewMatrix;
	GLint m_modelMatrix;
};

#endif /* GRAPHICS_H */
