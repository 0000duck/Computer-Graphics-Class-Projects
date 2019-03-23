
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
	m_sound = nullptr;
	m_world = nullptr;
}

Engine::~Engine() {

	ImGui_ImplSdlGL3_Shutdown();
	if(m_window) delete m_window;
	if(m_graphics) delete m_graphics;
	if(m_sound) delete m_sound;
	if(m_world) delete m_world;
	m_window = nullptr;
	m_graphics = nullptr;
	m_sound = nullptr;
	m_world = nullptr;
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

	// set up sound system
	m_sound = new Sound();
	if(!m_sound->Initialize("../data/sounds/")) {
		std::cerr << "The sound system failed to initialize." << std::endl;
		return false;
	}

	m_world = new World(m_graphics->GetCam(), &w, &h);

	ImGui_ImplSdlGL3_Init(m_window->GetWindow());

	SDL_CaptureMouse(SDL_TRUE);
	SDL_ShowCursor(SDL_FALSE);

	// No errors
	return true;
}

void Engine::Run() {

	m_running = true;
	m_world->StartGenerating();

	while(m_running) {

		ImGui_ImplSdlGL3_NewFrame(m_window->GetWindow());

		// Update the DT
		m_DT = getDT();
		Events();

		ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Frametime: %f", m_DT);
		ImGui::Text("FPS: %f", 1.0 / m_DT);
		ImGui::End();

		m_graphics->Update(m_DT);
		m_graphics->Clear();
		m_graphics->RenderSkybox();

		ShaderInfo info = m_graphics->BeginWorld();
		m_world->Simulate(m_DT);
		m_world->Render(info);

		info = m_graphics->BeginScene();
		m_world->RenderPlayer(info);

		m_graphics->UI();
		m_world->UI();
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
				   m_event.type == SDL_MOUSEMOTION || 
				   m_event.type == SDL_MOUSEWHEEL) {

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
		if(m_event.key.keysym.sym == SDLK_LCTRL) {
			mouse_captured = true;
		}
	}

	if(m_event.type == SDL_KEYUP) {

		if(m_event.key.keysym.sym == SDLK_t) {
			m_graphics->ReloadShaders();
		}
		else
		if(m_event.key.keysym.sym == SDLK_LCTRL) {
			mouse_captured = false;
		}
	}
}

void Engine::MouseEvts() {

	auto& io = ImGui::GetIO();

	if(m_event.type == SDL_MOUSEBUTTONUP && m_event.button.button == SDL_BUTTON_RIGHT) {
		if(!io.WantCaptureMouse) {
			m_world->TryPlace();
		}
	}

	if(m_event.type == SDL_MOUSEBUTTONUP && m_event.button.button == SDL_BUTTON_LEFT) {
		if(!io.WantCaptureMouse) {
			m_world->TryDestroy();
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

	if(m_event.type == SDL_MOUSEWHEEL) {
		if(!io.WantCaptureMouse) {
			m_world->Scroll(m_event.wheel.y);
		}
	}
}

double Engine::getDT() {

	uint64_t newperf = SDL_GetPerformanceCounter();
	uint64_t delta = newperf - perfcounter;
	perfcounter = newperf;
	return (double)delta / SDL_GetPerformanceFrequency();
}
