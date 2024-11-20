#include "Sound.h"

#include <SDL_filesystem.h>

#include "Config.h"
#include "Definitions.h"
#include "Utility.h"

namespace Sound {

Mix_Chunk *break_sound;
Mix_Chunk *place_sound;
Mix_Chunk *step_sound;
Mix_Music *bgm;

void initialize() {
    const char *base_path = SDL_GetBasePath();
    char asset_path[Config::System::MAX_PATH_LEN];

    // Load sound effects
    join_path(asset_path, base_path, "assets/step.wav");
    step_sound = Mix_LoadWAV(asset_path);
    join_path(asset_path, base_path, "assets/break.wav");
    break_sound = Mix_LoadWAV(asset_path);
    join_path(asset_path, base_path, "assets/place.wav");
    place_sound = Mix_LoadWAV(asset_path);
    if (step_sound == nullptr || break_sound == nullptr || place_sound == nullptr) {
        LogError("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    // Load music
    join_path(asset_path, base_path, "assets/calm_bgm.ogg");
    bgm = Mix_LoadMUS(asset_path);
    if (bgm == nullptr) {
        LogError("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
}
}  // namespace Sound
