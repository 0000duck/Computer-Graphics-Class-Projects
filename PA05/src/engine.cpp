
#include "engine.h"

Engine::Engine(string name, int width, int height) {

	m_WINDOW_NAME = name;
	w = width;
	h = height;
	m_FULLSCREEN = false;

	mx = my = 0;
	mouse_captured = false;


}

Engine::~Engine() {

	ImGui_ImplSdlGL3_Shutdown();
	delete m_window;
	delete m_graphics;
	m_window = nullptr;
	m_graphics = nullptr;
}

bool Engine::Initialize(std::vector<std::string> args) {
  	
  	// Start a window
	m_window = new Window();
	if(!m_window->Initialize(m_WINDOW_NAME, &w, &h)) {
		printf("The window failed to initialize.\n");
		return false;
	}
	
	// Start the graphics
	m_graphics = new Graphics();
	if(!m_graphics->Initialize(w, h, args)) {
		printf("The graphics failed to initialize.\n");
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

		// Update and render the graphics
		m_graphics->Update(m_DT);
		m_graphics->Render(w, h);
		
		ImGui::Render();

		// Swap to the Window
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
	}
}

void Engine::KeyboardEvts() {

	if (m_event.key.keysym.sym == SDLK_ESCAPE) {
	  
		m_running = false;
	}
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
			m_graphics->EvtCamera(dx, dy);
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
