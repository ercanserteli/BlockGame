#pragma once
#include <SDL_mixer.h>

#include "Definitions.h"

namespace Sound {
extern Mix_Chunk *break_sound;
extern Mix_Chunk *place_sound;
extern Mix_Chunk *step_sound;
extern Mix_Music *bgm;

void initialize();
inline void play(Mix_Chunk *sound, int32 channel = -1, int32 loops = 0) { Mix_PlayChannel(channel, sound, loops); }
inline void play(Mix_Music *music, int32 loops = -1) { Mix_PlayMusic(music, loops); }
inline bool is_playing(int32 channel) { return Mix_Playing(channel); }
inline int32 get_volume() { return Mix_Volume(-1, -1); }
inline int32 set_volume(int32 channel, int32 volume) { return Mix_Volume(channel, volume); }
}  // namespace Sound
