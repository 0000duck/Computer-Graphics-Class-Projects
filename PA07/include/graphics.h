#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "scene.h"
#include "solarsystem.h"

struct MatLocs {
	GLint model, rotate, ambient;
};

class Graphics {

public:
	Graphics();
	~Graphics();

	// setup openGL
	bool Initialize(int width, int height);
	
	// update camera simulation
	void Update(unsigned int dt, Planet* tracked);
	// setup rendering context for planets
	MatLocs BeginPlanetRender(int w, int h);
	// setup rendering context for paths
	GLint BeginPathRender(int w, int h);
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
	// check if using a planet-tracking camera
	bool IsTracking();

private:
	// get a pointer to the current camera (orbit_camrea, free_camera, or tracking_camera)
	Camera* GetCamera();

	// set up the skybox cubemap texture
	bool CreateCubeMap(std::string file);
	GLuint cubemap_tex, cubemap_vbo, cubemap_vao;
	
	// VAOs for mesh rendering
	GLuint planet_VAO, path_VAO;

	// get openGL error as string
	std::string ErrorString(GLenum error);

	// cameras
	CameraType 		camera_type;
	OrbitCamera 	orbit_camera;
	FreeCamera  	free_camera;
	TrackingCamera 	tracking_camera;

	// shaders
	Shader *m_planet_shader, *m_cubemap_shader, *m_path_shader;
};

#endif /* GRAPHICS_H */
