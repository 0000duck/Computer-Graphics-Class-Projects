
#include "graphics.h"
#include <imgui.h>
#include <stb_image.h>
#include <SDL2/SDL.h>

float cubemap_points[] = {
  -10.0f,  10.0f, -10.0f,
  -10.0f, -10.0f, -10.0f,
   10.0f, -10.0f, -10.0f,
   10.0f, -10.0f, -10.0f,
   10.0f,  10.0f, -10.0f,
  -10.0f,  10.0f, -10.0f,
  
  -10.0f, -10.0f,  10.0f,
  -10.0f, -10.0f, -10.0f,
  -10.0f,  10.0f, -10.0f,
  -10.0f,  10.0f, -10.0f,
  -10.0f,  10.0f,  10.0f,
  -10.0f, -10.0f,  10.0f,
  
   10.0f, -10.0f, -10.0f,
   10.0f, -10.0f,  10.0f,
   10.0f,  10.0f,  10.0f,
   10.0f,  10.0f,  10.0f,
   10.0f,  10.0f, -10.0f,
   10.0f, -10.0f, -10.0f,
   
  -10.0f, -10.0f,  10.0f,
  -10.0f,  10.0f,  10.0f,
   10.0f,  10.0f,  10.0f,
   10.0f,  10.0f,  10.0f,
   10.0f, -10.0f,  10.0f,
  -10.0f, -10.0f,  10.0f,
  
  -10.0f,  10.0f, -10.0f,
   10.0f,  10.0f, -10.0f,
   10.0f,  10.0f,  10.0f,
   10.0f,  10.0f,  10.0f,
  -10.0f,  10.0f,  10.0f,
  -10.0f,  10.0f, -10.0f,
  
  -10.0f, -10.0f, -10.0f,
  -10.0f, -10.0f,  10.0f,
   10.0f, -10.0f, -10.0f,
   10.0f, -10.0f, -10.0f,
  -10.0f, -10.0f,  10.0f,
   10.0f, -10.0f,  10.0f
};

Graphics::Graphics() {

	m_planet_shader = nullptr;
}

Graphics::~Graphics() {

	if(m_planet_shader) {

		delete m_planet_shader;
		m_planet_shader = nullptr;
	}
	if(m_cubemap_shader) {

		delete m_cubemap_shader;
		m_cubemap_shader = nullptr;
	}
	if(m_path_shader) {

		delete m_path_shader;
		m_path_shader = nullptr;
	}

	glDeleteBuffers(1, &cubemap_vbo);
	glDeleteTextures(1, &cubemap_tex);
	glDeleteVertexArrays(1, &cubemap_vao);
	glDeleteVertexArrays(1, &planet_VAO);
	glDeleteVertexArrays(1, &path_VAO);
}

bool Graphics::Initialize(int width, int height) {
  
	glewExperimental = GL_TRUE;

	auto status = glewInit();

	// This is here to grab the error that comes from glew init.
	// This error is an GL_INVALID_ENUM that has no effects on the performance
	glGetError();

	//Check for error
	if (status != GLEW_OK) {
	  
		std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
		return false;
	}

	// For OpenGL 3
	glGenVertexArrays(1, &planet_VAO);
	glGenVertexArrays(1, &path_VAO);

	// Init Camera
	camera_type = CameraType::orbit;

	// Set up the shaders
	{
		m_planet_shader = new Shader();
		if(!m_planet_shader->Initialize()) {

			printf("Shader Failed to Initialize\n");
			return false;
		}
		
		// Add the vertex shader
		if(!m_planet_shader->AddShader(GL_VERTEX_SHADER, "../data/shaders/shader.v")) {
		
			printf("Vertex Shader failed to Initialize\n");
			return false;
		}
		
		// Add the fragment shader
		if(!m_planet_shader->AddShader(GL_FRAGMENT_SHADER, "../data/shaders/shader.f")) {
		
			printf("Fragment Shader failed to Initialize\n");
			return false;
		}
		
		// Connect the program
		if(!m_planet_shader->Finalize()) {

			printf("Program to Finalize\n");
			return false;
		}
	}
	{
		m_cubemap_shader = new Shader();
		if(!m_cubemap_shader->Initialize()) {

			printf("Shader Failed to Initialize\n");
			return false;
		}
		
		// Add the vertex shader
		if(!m_cubemap_shader->AddShader(GL_VERTEX_SHADER, "../data/shaders/cubemap.v")) {
		
			printf("Vertex Shader failed to Initialize\n");
			return false;
		}
		
		// Add the fragment shader
		if(!m_cubemap_shader->AddShader(GL_FRAGMENT_SHADER, "../data/shaders/cubemap.f")) {
		
			printf("Fragment Shader failed to Initialize\n");
			return false;
		}
		
		// Connect the program
		if(!m_cubemap_shader->Finalize()) {

			printf("Program to Finalize\n");
			return false;
		}
	}
	{
		m_path_shader = new Shader();
		if(!m_path_shader->Initialize()) {

			printf("Shader Failed to Initialize\n");
			return false;
		}
		
		// Add the vertex shader
		if(!m_path_shader->AddShader(GL_VERTEX_SHADER, "../data/shaders/path.v")) {
		
			printf("Vertex Shader failed to Initialize\n");
			return false;
		}
		
		// Add the fragment shader
		if(!m_path_shader->AddShader(GL_FRAGMENT_SHADER, "../data/shaders/path.f")) {
		
			printf("Fragment Shader failed to Initialize\n");
			return false;
		}
		
		// Connect the program
		if(!m_path_shader->Finalize()) {

			printf("Program to Finalize\n");
			return false;
		}
	}

	// load cube map texture
	if(!CreateCubeMap("../data/textures/starscape.png")) {
		return false;
	}

	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	
	return true;
}

void Graphics::Update(unsigned int dt, Planet* tracked) {

	static const unsigned char* keys = SDL_GetKeyboardState(NULL);
	
	tracking_camera.set(tracked);
	free_camera.last_update += dt;

	if(camera_type == CameraType::free) {
		float dT = dt / 1000.0f;

		if (keys[SDL_SCANCODE_W]) {
			free_camera.pos += free_camera.front * free_camera.speed * dT;
		}
		if (keys[SDL_SCANCODE_S]) {
			free_camera.pos -= free_camera.front * free_camera.speed * dT;
		}
		if (keys[SDL_SCANCODE_A]) {
			free_camera.pos -= free_camera.right * free_camera.speed * dT;
		}
		if (keys[SDL_SCANCODE_D]) {
			free_camera.pos += free_camera.right * free_camera.speed * dT;
		}

		free_camera.update();
	}
}

bool Graphics::CreateCubeMap(std::string file) {

	glGenBuffers(1, &cubemap_vbo);
	glGenVertexArrays(1, &cubemap_vao);
	glGenTextures(1, &cubemap_tex);

	glBindVertexArray(cubemap_vao);
	glBindBuffer(GL_ARRAY_BUFFER, cubemap_vbo);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex);
	glActiveTexture(GL_TEXTURE0);

	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &cubemap_points, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	int x, y, n;
	unsigned char* image = stbi_load(file.c_str(), &x, &y, &n, 4);
	if (!image) {
		std::cerr << "Error loading cubemap texture " << file << std::endl;
		return false;
	}

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindVertexArray(0);

	return true;
}

Camera* Graphics::GetCamera() {
	switch(camera_type) {
	case CameraType::orbit: 	return &orbit_camera;
	case CameraType::free: 		return &free_camera;
	case CameraType::tracking: 	return &tracking_camera;
	default: return nullptr;
	}
}

void Graphics::MoveCamera(int dx, int dy) {
	
	Camera* c = GetCamera();
	c->move(dx, dy);
}

void Graphics::SetCameraDistance(float d) {
	
	Camera* c = GetCamera();
	c->setDistance(d);
}

bool Graphics::IsTracking() {
	return camera_type == CameraType::tracking;
}

void Graphics::Clear() {
	
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Graphics::RenderSkybox(int w, int h) {

	Camera* c = GetCamera();

	glDepthMask(GL_FALSE);
	glBindVertexArray(cubemap_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex);
	m_cubemap_shader->Enable();
	glUniformMatrix4fv(m_cubemap_shader->GetUniformLocation("proj"), 1, GL_FALSE, glm::value_ptr(c->GetProjection(w, h))); 
	glUniformMatrix4fv(m_cubemap_shader->GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(c->GetViewWithoutTranslate())); 
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}

MatLocs Graphics::BeginPlanetRender(int w, int h) {

	Camera* c = GetCamera();
	glBindVertexArray(planet_VAO);
	m_planet_shader->Enable();
	
	// Send in the projection and view to the shader
	glUniformMatrix4fv(m_planet_shader->GetUniformLocation("proj"), 1, GL_FALSE, glm::value_ptr(c->GetProjection(w, h))); 
	glUniformMatrix4fv(m_planet_shader->GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(c->GetView())); 

	const float lightColor[] = {1.0f, 1.0f, 1.0f};
	const float lightPos[] = {0.0f, 0.0f, 0.0f};
	glUniform3fv(m_planet_shader->GetUniformLocation("lightColor"), 1, lightColor);
	glUniform3fv(m_planet_shader->GetUniformLocation("lightPos"), 1, lightPos);

	MatLocs m;
	m.model = m_planet_shader->GetUniformLocation("model");
	m.rotate = m_planet_shader->GetUniformLocation("rotate");
	m.ambient = m_planet_shader->GetUniformLocation("ambientStrength");
	return m;
}

void Graphics::UI() {

	Camera* c = GetCamera();
	ImGui::Text("Rendering");
	ImGui::SliderFloat("FOV", &c->fov, 10.0f, 150.0f); 
 	const char* names[] = {"Orbit Camera", "Free Camera", "Tracking Camera"};
	ImGui::Combo("Camera Type", (int*)&camera_type, names, 3);

	if(c == &free_camera) {
		ImGui::SliderFloat("Speed", &free_camera.speed, 5.0f, 100.0f);
	}
}

GLint Graphics::BeginPathRender(int w, int h) {

	Camera* c = GetCamera();
	glBindVertexArray(path_VAO);
	m_path_shader->Enable();
	
	// Send in the projection and view to the shader
	glUniformMatrix4fv(m_path_shader->GetUniformLocation("proj"), 1, GL_FALSE, glm::value_ptr(c->GetProjection(w, h))); 
	glUniformMatrix4fv(m_path_shader->GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(c->GetView())); 

	return m_path_shader->GetUniformLocation("model");
}

void Graphics::EndRender() {

	ImGui::Render();

	// Get any errors from OpenGL
	auto error = glGetError();
	if ( error != GL_NO_ERROR ) {

		std::string val = ErrorString( error );
		std::cerr << "OpenGL Error! " << error << ", " << val << std::endl;
	}
}

std::string Graphics::ErrorString(GLenum error) {

	if(error == GL_INVALID_ENUM) {

		return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
	}
	else if(error == GL_INVALID_VALUE) {

		return "GL_INVALID_VALUE: A numeric argument is out of range.";
	}
	else if(error == GL_INVALID_OPERATION) {

		return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
	}
	
	else if(error == GL_INVALID_FRAMEBUFFER_OPERATION) {

		return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
	}
	else if(error == GL_OUT_OF_MEMORY) {

		return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
	}
	else {

		return "None";
	}
}
