
#include "graphics.h"
#include <imgui.h>

Graphics::Graphics() {

	m_camera = nullptr;
	m_shader = nullptr;
	m_scene = nullptr;
	load_failed = false;
}

Graphics::~Graphics() {

	if(m_camera) {

		delete m_camera;
		m_camera = nullptr;
	}

	if(m_shader) {

		delete m_shader;
		m_shader = nullptr;
	}

	if(m_scene) {

		delete m_scene;
		m_scene = nullptr;
	}

	glDeleteVertexArrays(1, &VAO);
}

bool Graphics::Initialize(int width, int height, std::vector<std::string> args) {
  
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
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Init Camera
	m_camera = new Camera();

	// Init Mesh
	scene_path = args.size() ? args[0] : "../data/earth.obj";
	scene_path.resize(200);

	m_scene = new Scene();
	if(!m_scene->Load(scene_path)) {

		printf("Scene failed to Initialize\n");
		return false;
	}
	load_failed = false;

	// Set up the shaders
	m_shader = new Shader();
	if(!m_shader->Initialize()) {

		printf("Shader Failed to Initialize\n");
		return false;
	}
	
	// Add the vertex shader
	if(!m_shader->AddShader(GL_VERTEX_SHADER, "../data/shader.v")) {
	
		printf("Vertex Shader failed to Initialize\n");
		return false;
	}
	
	// Add the fragment shader
	if(!m_shader->AddShader(GL_FRAGMENT_SHADER, "../data/shader.f")) {
	
		printf("Fragment Shader failed to Initialize\n");
		return false;
	}
	
	// Connect the program
	if(!m_shader->Finalize()) {

		printf("Program to Finalize\n");
		return false;
	}
	
	// Locate the projection matrix in the shader
	m_projectionMatrix = m_shader->GetUniformLocation("proj");
	if (m_projectionMatrix == INVALID_UNIFORM_LOCATION)  {
		
		printf("m_projectionMatrix not found\n");
		return false;
	}
	
	// Locate the view matrix in the shader
	m_viewMatrix = m_shader->GetUniformLocation("view");
	if (m_viewMatrix == INVALID_UNIFORM_LOCATION) {
		
		printf("m_viewMatrix not found\n");
		return false;
	}
	
	// Locate the model matrix in the shader
	m_modelMatrix = m_shader->GetUniformLocation("model");
	if (m_modelMatrix == INVALID_UNIFORM_LOCATION) {
		
		printf("m_modelMatrix not found\n");
		return false;
	}
	
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	return true;
}

void Graphics::Update(unsigned int dt) {
	
	ImGui::SetNextWindowPos(ImVec2(20, 20));
	ImGui::SetNextWindowSize(ImVec2(250, 100));
	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::InputText("File", (char*)scene_path.data(), 200);
	if(ImGui::Button("Load")) {
		load_failed = !m_scene->Load(scene_path);
	}
	if(load_failed) {
		ImGui::Text("Failed to load model!");
	}
	ImGui::End();
}

void Graphics::EvtCamera(int dx, int dy) {
	m_camera->move(dx, dy);
}

void Graphics::Render(int w, int h) {
  
	//clear the screen
	glClearColor(0.0, 0.0, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDisable(GL_BLEND);

	glBindVertexArray(VAO);

	// Start the correct program
	m_shader->Enable();
	
	// Send in the projection and view to the shader
	glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection(w, h))); 
	glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView())); 
	
	// Render the object
	glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
	m_scene->Render();

	// Get any errors from OpenGL
	auto error = glGetError();
	if ( error != GL_NO_ERROR ) {

		std::string val = ErrorString( error );
		std::cout << "Error initializing OpenGL! " << error << ", " << val << std::endl;
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
