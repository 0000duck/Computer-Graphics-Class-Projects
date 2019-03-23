#ifndef SOUND_H
#define SOUND_H

#include <unordered_map>
#include <map>
#include <string>
#include <SDL2/SDL_mixer.h>

class Sound {
public:
	Sound();
	~Sound();

	// initialize with a specified font
	bool Initialize(std::string sound_dir);
	// play a certain sound
	void Play(std::string id);

private:

	std::unordered_map<std::string,Mix_Chunk*> sounds;
	std::map<std::string,int> playing;
};

#endif