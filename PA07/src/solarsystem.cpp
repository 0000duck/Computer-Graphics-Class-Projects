
#include "solarsystem.h"
#include "graphics.h"
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <imgui.h>

bool isRegularFile(std::string path)
{
    struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return !!S_ISREG(path_stat.st_mode);
}

SolarSystem::SolarSystem() {}

SolarSystem::~SolarSystem() {

	for(auto& p : planets) {
		p.DeleteScene();
	}
	planets.clear();
}

bool SolarSystem::LoadPlanets(std::string dir) {

	DIR *directory;
	dirent *entry;
	std::string dirPath = dir;

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

			std::string contents;
			std::ifstream fin(dirPath + entryName);
			getline(fin, contents, '\0');

			Planet p;
			if(p.LoadJSON(contents)) {
				p.LoadScene();
				planets.push_back(p);

				std::cout << "Loaded planet from " << dirPath << entryName << std::endl;
			}
		}
	}

	return true;
}

float SolarSystem::GetCameraScale() {
	if(settings.do_distance_scale) {
		return 600.0f;
	} else {
		return 30.0f;
	}
}

void SolarSystem::Update(unsigned int dT) {

	for(auto& p : planets) {
		p.Update(dT, settings);
	}
}

Planet* SolarSystem::GetTrackedPlanet() {
	return &planets[settings.track_planet_idx];
}

void SolarSystem::RenderPaths(GLint shader_MdlMx) {

	if(settings.do_orbit_path) {
		for(auto& p : planets) {
			p.RenderPath(shader_MdlMx);
		}
	}
}

void SolarSystem::GenPaths() {

	for(auto& p : planets) {
		p.GenerateOrbitTrace(settings);
	}
}

void SolarSystem::Render(const MatLocs& matlocs) {

	for(auto& p : planets) {
		p.Render(matlocs, settings);
	}
}

void SolarSystem::UI(bool track) {
	if(track) {
		std::vector<const char*> names;

		for(auto& p : planets) {
			names.push_back(p.name.c_str());
		}
		ImGui::Combo("Track", &settings.track_planet_idx, names.data(), names.size());
	}

	ImGui::Checkbox("Draw Orbit Paths", &settings.do_orbit_path);

	ImGui::Separator();
	ImGui::Text("Simulation");
	ImGui::SliderFloat("Time Scale", &settings.time_scale, 0.0f, 2500.0f);
	ImGui::Checkbox("Use Time Scale", &settings.do_time_scale);
	
	bool traces_changed = false;
	traces_changed = traces_changed || ImGui::SliderFloat("Distance Scale", &settings.distance_scale, 0.001f, 0.05f);
	traces_changed = traces_changed || ImGui::SliderFloat("Diameter Scale", &settings.diameter_scale, 1.0f, 100.0f);
	traces_changed = traces_changed || ImGui::Checkbox("Use Distance Scales", &settings.do_distance_scale);

	if(traces_changed) {
		GenPaths();
	}
}
