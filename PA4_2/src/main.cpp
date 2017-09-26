#include <iostream>

#include "engine.h"


int main(int argc, char **argv)
{
	char * vertexShader;
	char * fragmentShader;
	char * objectFile;

  // Start an engine and run it then cleanup after
	for(int i = 0; i < argc; i++) {

		if(string(argv[i]) == "-v") {
			vertexShader = argv[i+1];
		}

		if(string(argv[i]) == "-f") {
			fragmentShader = argv[i+1];
		}

		if(string(argv[i]) == "-m") {
			objectFile = argv[i+1];
		}
	}

  Engine *engine = new Engine("PA1", 800, 600, vertexShader, fragmentShader, objectFile);
  if(!engine->Initialize())
  {
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
