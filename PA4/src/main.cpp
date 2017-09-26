#include <iostream>

#include "engine.h"


int main(int argc, char **argv)
{
	char * vertexShader;
	char * fragmentShader;
	char * model;
	bool modelGiven = false;

  // Start an engine and run it then cleanup after
	for(int i = 0; i < argc; i++) {

		if(string(argv[i]) == "-v") {
			vertexShader = argv[i+1];
		}

		if(string(argv[i]) == "-f") {
			fragmentShader = argv[i+1];
		}

		if(string(argv[i]) == "-m") {
			model = argv[i+1];
			modelGiven = true;
		}
	}
	Engine * engine;
	if(modelGiven){
  	engine = new Engine("PA4", 800, 600, vertexShader, fragmentShader, model);
		cout << "Here" << endl;
	}else {
		engine = new Engine("PA4", 800, 600, vertexShader, fragmentShader);
	}

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
