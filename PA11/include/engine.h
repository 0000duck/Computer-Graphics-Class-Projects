#ifndef ENGINE_H
#define ENGINE_H

#include <sys/time.h>
#include <assert.h>
#include <imgui.h>

#include "graphics_headers.h"
#include "window.h"
#include "graphics.h"
#include "imgui_impl.h"
#include "sound.h"
#include "world.h"

class Engine {

public:
	Engine(string name, int width, int height);
	~Engine();
	
	// setup all systems, args are the command-line arguments
	bool Initialize(std::vector<std::string> args);
	// run the game loop
	void Run();
	// process SDL events
	void Events();
	// get time delta in milliseconds since last frame
	double getDT();
  
private:

	// process SDL keyboard events
	void KeyboardEvts();
	// process SDL mouse events
	void MouseEvts();
	// process SDL window events
	void WindowEvts();
	// Events
	SDL_Event m_event;

	// Window
	Window *m_window;    
	string m_WINDOW_NAME;
	bool m_FULLSCREEN;

	// Input
	int w, h;
	int mx, my;
	bool mouse_captured;

	Sound* m_sound;
	Graphics *m_graphics;
	World* m_world;
	
	// Timing
	double m_DT = 0.0;
	uint64_t perfcounter = 0;
	bool m_running = false;
};

#endif // ENGINE_H
