#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <string>
using namespace std;

class Window {
	
public:
	Window();
	~Window();

	// create window & openGL context
	bool Initialize(const string &name, int* width, int* height);
	// swap window buffers 
	void Swap();

	// get pointer to SDL window
	SDL_Window* GetWindow() const;

	void MakeContextCurrent();

private:
	SDL_Window* gWindow;
	SDL_GLContext gContext;
};

#endif /* WINDOW_H */
