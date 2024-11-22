#include "Sound.h"

#include <SDL_filesystem.h>

#include <filesystem>

#include "Definitions.h"

namespace Sound {

Mix_Chunk *break_sound;
Mix_Chunk *place_sound;
Mix_Chunk *step_sound;
Mix_Music *bgm;

void initialize() {
    const std::filesystem::path assets_base = std::filesystem::path(SDL_GetBasePath()) / "assets";

    // Load sound effects
    step_sound = Mix_LoadWAV((assets_base / "step.wav").string().c_str());
    break_sound = Mix_LoadWAV((assets_base / "break.wav").string().c_str());
    place_sound = Mix_LoadWAV((assets_base / "place.wav").string().c_str());
    if (step_sound == nullptr || break_sound == nullptr || place_sound == nullptr) {
        LogError("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    // Load music
    bgm = Mix_LoadMUS((assets_base / "calm_bgm.ogg").string().c_str());
    if (bgm == nullptr) {
        LogError("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
}
}  // namespace Sound
