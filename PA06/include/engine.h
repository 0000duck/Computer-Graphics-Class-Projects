#ifndef ENGINE_H
#define ENGINE_H

#include <sys/time.h>
#include <assert.h>

#include "window.h"
#include "graphics.h"
#include "imgui_impl.h"
#include <imgui.h>

class Engine {

public:
	Engine(string name, int width, int height);
	~Engine();
	
	bool Initialize(std::vector<std::string> args);
	void Run();
	void Events();
	
	unsigned int getDT();
	long long GetCurrentTimeMillis();
  
private:

	void KeyboardEvts();
	void MouseEvts();
	void WindowEvts();

	// Window related variables
	Window *m_window;    
	string m_WINDOW_NAME;

	int w, h;
	int mx, my;
	bool mouse_captured;
	
	bool m_FULLSCREEN;
	SDL_Event m_event;

	Graphics *m_graphics;
	unsigned int m_DT;
	long long m_currentTimeMillis;
	bool m_running;
};

#endif // ENGINE_H
