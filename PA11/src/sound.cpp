
#include "sound.h"
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>

bool isRegularFile(std::string path) {
    struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return !!S_ISREG(path_stat.st_mode);
}

Sound::Sound() {

}

Sound::~Sound() {

	for(auto& sound : sounds) {
		Mix_FreeChunk(sound.second);
	}
}

bool Sound::Initialize(std::string sound_dir) {

	if (Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 ) < 0) {
		std::cerr << "Error initializing SDL_mixer: " << Mix_GetError() << std::endl;
		return false;
	}

	DIR *directory;
	dirent *entry;
	std::string dirPath = sound_dir;

	if (dirPath.back() != '/' && dirPath.back() != '\\') {
		dirPath.append("/");
	}

	directory = opendir(dirPath.c_str());
	if (!directory) {
		std::cerr << "Failed to open directory at " << dirPath << std::endl;
		return false;
	}

	while((entry = readdir(directory))) {
		std::string entryName = entry->d_name;

		if(entryName != ".." && entryName != "." && isRegularFile(dirPath + entryName)) {

			std::cout << "Loading sound "<< entryName << std::endl;
			Mix_Chunk* chunk = Mix_LoadWAV((dirPath + entryName).c_str());
			if(chunk) {
				sounds.insert({entryName, chunk});
			} else {
				std::cerr << "Failed to load sound " << entryName << std::endl;
			}
		}
	}

	closedir(directory);

	auto music = sounds.find("music.wav");
	if(music != sounds.end()) {
		Mix_Volume(Mix_PlayChannel(-1, music->second, -1), 50);
	}

	return true;
}

void Sound::Play(std::string id) {

	for (auto entry = playing.cbegin(); entry != playing.cend();) {
		if(!Mix_Playing(entry->second)) {
			playing.erase(entry++);
		} else {
			++entry;
		}
	}

	auto music = sounds.find(id + ".wav");
	if(music != sounds.end()) {
		auto current = playing.find(id);
		if(current == playing.end()) {
			playing.insert({id, Mix_PlayChannel(-1, music->second, 0)});
		} else if(id != "activate_flipper") {
			Mix_PlayChannel(-1, music->second, 0);
		}
	}
}
