#include "Save.h"

#include <SDL_rwops.h>

#include "Config.h"
#include "GameBase.h"

void save_state(const GameState *state) {
    std::filesystem::path save_filename;
    state->get_save_file_name(save_filename);
    SDL_RWops *fp = SDL_RWFromFile(save_filename.string().c_str(), "wb");
    if (fp != nullptr) {
        size_t num = SDL_RWwrite(fp, &state->player, sizeof(state->player), 1);
        if (num != 1) {
            LogError("Could not save player to file!");
        }
        num = SDL_RWwrite(fp, &state->sun, sizeof(state->sun), 1);
        if (num != 1) {
            LogError("Could not save sun to file!");
        }
        num = SDL_RWwrite(fp, &state->stars, sizeof(state->stars), 1);
        if (num != 1) {
            LogError("Could not save stars to file!");
        }

        SDL_RWclose(fp);
    } else {
        LogError("Could not open save file for saving!");
    }
}

void generate_stars(Vector3f *stars) {
    for (uint32 i = 0; i < Config::Game::STAR_COUNT; i++) {
        constexpr uint32 BOUNDS = 4000;
        constexpr uint32 HALF_BOUNDS = BOUNDS / 2;
        stars[i].x = (float32)(rand() % BOUNDS) - HALF_BOUNDS;
        stars[i].y = (float32)(rand() % BOUNDS) - HALF_BOUNDS;
        stars[i].z = (float32)(rand() % BOUNDS) - HALF_BOUNDS;
    }
}

bool load_state(GameState *state) {
    std::filesystem::path save_filename;
    state->get_save_file_name(save_filename);
    SDL_RWops *fp = SDL_RWFromFile(save_filename.string().c_str(), "rb");
    if (fp == nullptr) {
        LogDebug("Could not open save (state) file for loading!");
        return false;
    }

    bool broken = false;
    size_t num = SDL_RWread(fp, &state->player, sizeof(state->player), 1);
    if (num != 1) {
        state->player = Player();
        LogError("Could not load player from file!");
        broken = true;
    }
    num = SDL_RWread(fp, &state->sun, sizeof(state->sun), 1);
    if (num != 1) {
        state->sun = Sun();
        LogError("Could not load sun from file!");
        broken = true;
    }
    num = SDL_RWread(fp, &state->stars, sizeof(state->stars), 1);
    if (num != 1 || (state->stars[0].x == 0.f && state->stars[0].y == 0.f && state->stars[0].z == 0.f)) {
        generate_stars(state->stars);
        LogError("Could not load stars from file!");
        broken = true;
    }

    SDL_RWclose(fp);
    if (broken) {
        const int32 result = remove(save_filename);
        if (result != 0) {
            LogError("Could not delete the broken save file %s!", save_filename);
        }
        return false;
    }
    return true;
}
