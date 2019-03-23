
#include "engine.h"

Engine::Engine(string name, int width, int height) {

	m_WINDOW_NAME = name;
	w = width;
	h = height;
	m_FULLSCREEN = false;

	mx = my = 0;
	mouse_captured = false;

	m_window = nullptr;
	m_graphics = nullptr;
	m_world = nullptr;
	m_text = nullptr;
	m_sound = nullptr;
}

Engine::~Engine() {

	ImGui_ImplSdlGL3_Shutdown();
	if(m_window) delete m_window;
	if(m_graphics) delete m_graphics;
	if(m_world) delete m_world;
	if(m_text) delete m_text;
	if(m_sound) delete m_sound;
	m_window = nullptr;
	m_graphics = nullptr;
	m_world = nullptr;
	m_text = nullptr;
	m_sound = nullptr;
}

bool Engine::Initialize(std::vector<std::string> args) {
  	
  	// Start a window
	m_window = new Window();
	if(!m_window->Initialize(m_WINDOW_NAME, &w, &h)) {
		std::cerr << "The window failed to initialize." << std::endl;
		return false;
	}
	
	// Start the graphics
	m_graphics = new Graphics();
	if(!m_graphics->Initialize(w, h)) {
		std::cerr << "The graphics failed to initialize." << std::endl;
		return false;
	}
	
	// Set up text rendering
	m_text = new Text();
	if(!m_text->Initialize(w, h, "../data/fonts/onemorelight.ttf")) {
		std::cerr << "The font system failed to initialize." << std::endl;
		return false;
	}

	// set up sound system
	m_sound = new Sound();
	if(!m_sound->Initialize("../data/sounds/")) {
		std::cerr << "The sound system failed to initialize." << std::endl;
		return false;
	}

	// Start the physics world
	m_world = new World();
	if(!m_world->Initialize(m_sound)) {
		std::cerr << "The physics world failed to initialize." << std::endl;
		return false;
	}
	if(!m_world->LoadObjects("../data/objects")) {
		std::cerr << "Failed to load physics objects." << std::endl;
		return false;
	}

	// Set the time
	m_currentTimeMillis = GetCurrentTimeMillis();
	
	ImGui_ImplSdlGL3_Init(m_window->GetWindow());

	// No errors
	return true;
}

void Engine::Run() {
	
	m_running = true;

	while(m_running) {

		ImGui_ImplSdlGL3_NewFrame(m_window->GetWindow());

		// Update the DT
		m_DT = getDT();
		Events();

		ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		m_graphics->Update(m_DT);
		m_world->Update(m_DT);

		m_graphics->Clear();
		m_graphics->RenderSkybox();
		
		ShaderInfo info = m_graphics->BeginObjectRender();
		m_world->Render(info);

		m_graphics->UI();

		m_text->Begin();
		m_world->UI(m_text);

		m_text->End(m_graphics);
		ImGui::End();
		m_graphics->EndRender();
		m_window->Swap();
	}
}

void Engine::Events() {

	while(SDL_PollEvent(&m_event) != 0) {

		ImGui_ImplSdlGL3_ProcessEvent(&m_event);

		if(m_event.type == SDL_QUIT) {

			m_running = false;

		} else if (m_event.type == SDL_KEYDOWN ||
				   m_event.type == SDL_KEYUP) {

			KeyboardEvts();

		} else if (m_event.type == SDL_MOUSEBUTTONDOWN ||
				   m_event.type == SDL_MOUSEBUTTONUP ||
				   m_event.type == SDL_MOUSEMOTION) {

			MouseEvts();

		} else if (m_event.type == SDL_WINDOWEVENT) {

			WindowEvts();
		}
	}
}

void Engine::WindowEvts() {

	if (m_event.window.event == SDL_WINDOWEVENT_RESIZED ||
		m_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
		
		w = m_event.window.data1;
		h = m_event.window.data2;
		glViewport(0, 0, w, h);
		glScissor(0, 0, w, h);

		m_graphics->UpdateWH(w,h);
	}
}

void Engine::KeyboardEvts() {

	if (m_event.key.keysym.sym == SDLK_ESCAPE) {
	  
		m_running = false;
	}

	if(m_event.type == SDL_KEYDOWN) {

		if(m_event.key.keysym.sym == SDLK_r) {
			
			delete m_world;
			m_world = new World();
			if(!m_world->Initialize(m_sound)) {
				std::cerr << "The physics world failed to initialize." << std::endl;
			}
			if(!m_world->LoadObjects("../data/objects")) {
				std::cerr << "Failed to load physics objects." << std::endl;
			}
		}
	}

	if(m_event.type == SDL_KEYUP) {
	
		if(m_event.key.keysym.sym == SDLK_t) {
			m_graphics->ReloadShaders();
		}
	}

	m_world->KeyboardEvts(m_event);
}

void Engine::MouseEvts() {

	auto& io = ImGui::GetIO();

	if(m_event.type == SDL_MOUSEBUTTONDOWN) {
		if(!io.WantCaptureMouse) {
			mouse_captured = true;
		}
	}

	if(m_event.type == SDL_MOUSEBUTTONUP) {
		if(!io.WantCaptureMouse) {
			mouse_captured = false;
		}
	}

	if(m_event.type == SDL_MOUSEMOTION) {
		if(mouse_captured) {
			int dx = (m_event.motion.x - mx);
			int dy = (m_event.motion.y - my);
			m_graphics->MoveCamera(dx, dy);
		}
		mx = m_event.motion.x;
		my = m_event.motion.y;
	}
}

unsigned int Engine::getDT() {

	long long TimeNowMillis = GetCurrentTimeMillis();
	assert(TimeNowMillis >= m_currentTimeMillis);
	unsigned int DeltaTimeMillis = (unsigned int)(TimeNowMillis - m_currentTimeMillis);
	m_currentTimeMillis = TimeNowMillis;
	return DeltaTimeMillis;
}

long long Engine::GetCurrentTimeMillis() {

	return SDL_GetTicks();
}
