#include <iostream>

#include "engine.h"


int main(int argc, char **argv)
{
	char * vertexShader;
	char * fragmentShader;
	
  // Start an engine and run it then cleanup after
	for(int i = 0; i < argc; i++) {
		cout << i << " arg: " << argv[i] << endl;
		
		if(string(argv[i]) == "-v") {
			cout << "arg -v: " << argv[i]  << endl << "arg +1: " << argv[i+1] << endl;
			vertexShader = argv[i+1];
			cout << "Got Vertex Shader: " << vertexShader << endl;
		}

		if(string(argv[i]) == "-f") {
			cout << "arg -f: " << argv[i]  << endl << "arg +1: " << argv[i+1] << endl;
			fragmentShader = argv[i+1];
			cout << "Got Fragment Shader: " << fragmentShader << endl;
		}
	}

  Engine *engine = new Engine("PA1", 800, 600, vertexShader, fragmentShader);
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
