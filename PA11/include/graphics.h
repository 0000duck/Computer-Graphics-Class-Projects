#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"

struct ShaderInfo {
	Shader* shader = nullptr; 
	glm::vec3 ambient_light, diffuse_light, specular_light;
	bool wireframe = false;
	float const_atten = 1.0f, lin_atten = 0.0f, quad_atten = 0.0f;
	float spot_cutoff = 90.0f, spot_exp = 1.0f;
	glm::vec3 spot_dir = glm::vec3(0, 1, 0);
};

class Graphics {

public:
	Graphics();
	~Graphics();

	// setup openGL
	bool Initialize(int width, int height);
	
	// update camera simulation
	void Update(float dT);
	// render skybox
	void RenderSkybox();
	// clear window
	void Clear();
	// render UI
	void UI();
	// end rendering context and render GUI
	void EndRender();
	// move camera based on mouse x & y delta
	void MoveCamera(int dx, int dy);

	ShaderInfo BeginWorld();
	ShaderInfo BeginScene();

	FreeCamera* GetCam();

	// delete and reload shaders from source files
	bool ReloadShaders();
	// get the current camera's view matrix
	glm::mat4 GetView();
	// get the current camera's projection matrix
	glm::mat4 GetProj();
	// set the window width & height
	void UpdateWH(int w, int h);

private:

	int w, h;

	// cubemap
	bool CreateCubeMap(std::string file);
	GLuint cubemap_tex = 0, cubemap_vbo = 0, cubemap_vao = 0, scene_VAO = 0;
	Shader *m_cubemap_shader = nullptr;
	
	// cameras
	FreeCamera  	free_camera;

	Shader *m_chunk_shader = nullptr;
	Shader *m_scene_shader = nullptr;
};

void debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up);

#endif /* GRAPHICS_H */
