
#include <iostream>
#include <string>
#include <vector>

#include "engine.h"

int main(int argc, char **argv) {
  
	// Start an engine and run it then cleanup after
	
	std::vector<std::string> args;
	for(int i = 1; i < argc; i++) {
		args.push_back(string(argv[i]));
	}

	Engine *engine = new Engine("Solar System", 1280, 720);

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
