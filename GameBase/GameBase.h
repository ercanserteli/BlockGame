#pragma once

#include <glm/gtc/quaternion.hpp>

#include "Chunk.h"
#include "Config.h"
#include "Definitions.h"
#include "Geometry.h"
#include "Utility.h"

struct SDL_Surface;
struct SDL_Window;

struct Particle {
    Vector3f pos = {};
    Vector3f speed = {};
    Vector3f color = {};
    glm::quat rotation = {};
    Vector2f rot_speed = {};
    float32 scale = 1;
    float32 age = 0;

    void initialize(const Vector3f pos_init, const Vector3f color_init, const float32 scale_init) {
        this->scale = scale_init;
        this->age = (rand_float() - 0.5f) * 0.5f;
        this->pos = pos_init;
        this->pos.x += rand_float() - 0.5f;
        this->pos.y += rand_float() - 0.5f;
        this->pos.z += rand_float() - 0.5f;
        this->speed.x = (rand_float() - 0.5f) * 5;
        this->speed.y = rand_float() * 5;
        this->speed.z = (rand_float() - 0.5f) * 5;
        this->color = color_init;
        this->rotation = {1, 0, 0, 0};
        this->rot_speed.x = (rand_float() - 0.5f) * 10;
        this->rot_speed.y = (rand_float() - 0.5f) * 10;
    }
};

struct GameMemory {
    void *permanent_storage;
    uint64 permanent_storage_size;
    void *record_storage;
    uint64 record_storage_size = 0;
    void *transient_storage;
    uint64 transient_storage_size;
};

struct MemoryArena {
    uint8 *base = nullptr;
    size_t size = 0;
    size_t used = 0;

    MemoryArena(uint8 *base, const size_t size) : base(base), size(size) {}
    MemoryArena() = default;
};

#define pushStruct(arena, type) (type *)push_size(arena, sizeof(type))
#define pushArray(arena, count, type) (type *)push_size(arena, (count) * sizeof(type))
inline void *push_size(MemoryArena &arena, const size_t size) {
    ASSERT((arena.used + size) <= arena.size);
    void *result = arena.base + arena.used;
    arena.used += size;
    return result;
}

struct BlockEntity {
    Vector3f pos = {};
    Vector3f speed = {};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    Vector2f rot_speed = {};
    float32 age = 0;

    Vector3f color = {};
    bool bound = false;
    Vector3f bound_offset = {};
};

struct Player {
    Vector3f pos = {-5, 96, 5};
    Vector3f speed = {};
    Vector3f direction = {0, 0, 1};
    float32 pitch = 0;
    float32 yaw = 0;
    int32 selected_block = 0;
    bool on_ground = false;
    float32 fov = 45.f;
    BlockEntity *hand_entity = nullptr;
    bool throw_mode = false;
};

struct Sun {
    Vector3f pos = {};
    Vector3f color = {1.0f, 1.0f, 1.0f};
    glm::quat rot = {1.0f, 0.0f, 0.0f, 0.0f};
    Vector3f sky_color = {};
    float32 diffuse_strength = 0;
    float32 specular_strength = 0;
    float32 star_visibility = 0;
    float32 age = 0;
    float32 speed_boost = 1;
};

struct GameState {
    MemoryArena world_arena;
    MemoryArena scratch_arena;
    char *save_path = nullptr;
    char world_name[32] = "world";
    uint32 particle_count = 0;
    uint32 entity_count = 0;
    uint64 frame_count = 0;
    bool debug_visuals_enabled = false;

    Player player;
    Sun sun;

    Vector3f stars[Config::Game::STAR_COUNT];
    Particle particles[Config::Game::PARTICLE_LIMIT];
    BlockEntity entities[Config::Game::ENTITY_LIMIT];
    ChunkMap chunk_map;

    void get_save_file_name(char *filename) const { ASSERT(snprintf(filename, Config::System::MAX_PATH_LEN, "%s%s/state.erc", save_path, world_name) > 0); }
};

struct ControllerInput {
    float32 dir_left;
    float32 dir_right;
    float32 dir_up;
    float32 dir_down;

    int32 mouse_move_x;
    int32 mouse_move_y;
    int32 mouse_wheel;
    bool button_mouse_l;
    bool button_mouse_r;
    bool button_mouse_m;

    bool button_a;
    bool button_b;
    bool button_c;
    bool button_d;
    bool button_l;
    bool button_r;
    bool button_l2;
    bool button_r2;
    bool button_l3;
    bool button_r3;
    bool button_select;
    bool button_start;
    bool button_f1;
    bool button_f2;
    bool button_f3;
    bool button_f4;
    bool button_f5;
};

enum class ShadowMode { NONE, SHADOW_MAP, SHADOW_VOLUME };

typedef void (*FinalizeFuncType)(GameMemory *);
typedef void (*ReloadInitFuncType)(GameMemory *);
typedef void (*InitializeFuncType)(GameMemory *, char *);
typedef void (*UpdateFuncType)(GameMemory *, SDL_Surface *, SDL_Window *, ControllerInput *, float32);
