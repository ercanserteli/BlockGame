#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AABB.h"
#include "Chunk.h"
#include "GameBase.h"
#include "Play.h"
#include "Graphics.h"
#include "Save.h"
#include "Sound.h"

extern "C" dll_export void initialize(const GameMemory *memory, char *save_path) {
    auto *state = new (memory->permanent_storage) GameState();
    state->save_path = save_path;
    state->world_arena = MemoryArena((uint8 *)memory->permanent_storage + sizeof(GameState), memory->permanent_storage_size - sizeof(GameState));
    state->scratch_arena = MemoryArena((uint8 *)memory->transient_storage, memory->transient_storage_size);

    state->chunk_map.initialize(state);

    if (!load_state(state)) {
        char save_dir[Config::System::MAX_PATH_LEN];
        ASSERT(snprintf(save_dir, Config::System::MAX_PATH_LEN, "%s%s", state->save_path, state->world_name) > 0);
        create_dir(save_dir);
    }

    Graphics::initialize(state);
    Sound::initialize();
    //Sound::play(Sound::bgm);
}

extern "C" dll_export void reload_init(const GameMemory *memory) {
    // Reinitialize graphics on DLL hot reload
    Graphics::initialize((GameState *)memory->permanent_storage);
}

extern "C" dll_export void finalize(const GameMemory *memory) {
    const auto *state = (GameState *)memory->permanent_storage;
    state->chunk_map.save();
    save_state(state);
}

extern "C" dll_export void game_loop(const GameMemory *memory, const SDL_Surface *screen_surface, SDL_Window *window, ControllerInput *controller,
                                     float32 time_delta) {
    static ControllerInput last_controller;
    auto *state = (GameState *)memory->permanent_storage;

    const int32 screen_width = screen_surface->w;
    const int32 screen_height = screen_surface->h;
    BlockPos b_pos_pointing;
    uint8 block_pointing;

    // Screen shot with F2
    if (!last_controller.button_f2 && controller->button_f2) {
        Graphics::take_screenshot(state, screen_width, screen_height);
    }

    Play::update(state, time_delta, controller, &last_controller, b_pos_pointing, block_pointing);
    Graphics::draw(state, screen_width, screen_height, window, block_pointing, b_pos_pointing, time_delta);

    last_controller = *controller;
    state->frame_count++;
    state->scratch_arena.used = 0;
}
