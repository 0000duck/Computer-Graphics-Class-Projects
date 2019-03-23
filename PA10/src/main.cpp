
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "engine.h"

int main(int argc, char **argv) {
  
	// Start an engine and run it then cleanup after
	
	srand(time(0));
	std::vector<std::string> args;
	for(int i = 1; i < argc; i++) {
		args.push_back(string(argv[i]));
	}

	Engine *engine = new Engine("PINBALL", 1280, 720);

	if(!engine->Initialize(args)) {

		printf("The engine failed to start.\n");
		delete engine;
		engine = NULL;
		return 1;
	}

	engine->Run();

	delete engine;
	engine = NULL;
	
	return 0;
}
