#include "Sound.h"

#include "Definitions.h"

namespace Sound {

Mix_Chunk *break_sound;
Mix_Chunk *place_sound;
Mix_Chunk *step_sound;
Mix_Music *bgm;

void initialize() {
    // Load sound effects
    step_sound = Mix_LoadWAV("assets/step.wav");
    break_sound = Mix_LoadWAV("assets/break.wav");
    place_sound = Mix_LoadWAV("assets/place.wav");
    if (step_sound == nullptr || break_sound == nullptr || place_sound == nullptr) {
        LogError("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    // Load music
    bgm = Mix_LoadMUS("assets/calm_bgm.ogg");
    if (bgm == nullptr) {
        LogError("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
}
}  // namespace Sound
