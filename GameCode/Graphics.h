#pragma once
#include <SDL_video.h>

#include "Definitions.h"

struct Frustum;
struct BlockPos;
struct GameState;

namespace Graphics {
void initialize(const GameState *state);
void draw(GameState *state, int32 screen_width, int32 screen_height, SDL_Window *window, uint8 block_pointing, const BlockPos &b_pos_pointing,
          float32 time_delta);
void take_screenshot(GameState *state, int32 screen_width, int32 screen_height);
void switch_shadow_mode();
}  // namespace Graphics
