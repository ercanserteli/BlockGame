#pragma once

struct GameState;

void save_state(const GameState *state);
bool load_state(GameState *state);
