#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "scene.h"

struct UniformLocs {
	GLint model, rotate, ambient;
};

class Graphics {

public:
	Graphics();
	~Graphics();

	// setup openGL
	bool Initialize(int width, int height);
	
	// update camera simulation
	void Update(unsigned int dt);
	// setup rendering context for planets
	UniformLocs BeginObjectRender(int w, int h);
	// render skybox
	void RenderSkybox(int w, int h);
	// clear window
	void Clear();
	// render UI
	void UI();
	// end rendering context and render GUI
	void EndRender();
	// move camera based on mouse x & y delta
	void MoveCamera(int dx, int dy);
	// for orbit camera, set distance from center
	void SetCameraDistance(float d);

private:
	// get a pointer to the current camera (orbit_camrea, free_camera, or tracking_camera)
	Camera* GetCamera();

	// cubvemap
	bool CreateCubeMap(std::string file);
	GLuint cubemap_tex, cubemap_vbo, cubemap_vao;
	Shader *m_cubemap_shader;
	
	// cameras
	CameraType 		camera_type;
	OrbitCamera 	orbit_camera;
	FreeCamera  	free_camera;

	// Object rendering
	GLuint object_VAO;
	Shader *m_object_shader;
};

void debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up);

#endif /* GRAPHICS_H */
