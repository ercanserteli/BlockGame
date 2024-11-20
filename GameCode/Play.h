#pragma once
#include "Definitions.h"

struct GameState;
struct ControllerInput;
struct BlockPos;

namespace Play {
void update(GameState *state, float32 time_delta, ControllerInput *controller, const ControllerInput *last_controller, BlockPos &b_pos_pointing,
            uint8 &block_pointing, int32 screen_width, int32 screen_height);
}