#pragma once
#include "Definitions.h"

namespace Config {

struct World {
    static constexpr int32 CHUNK_SIZE = 32;
    static constexpr int32 DRAW_RADIUS = 10;
    static constexpr float32 BLOCK_BREAK_COOLDOWN = 0.3f;
    static constexpr float32 BLOCK_PLACE_COOLDOWN = 0.3f;
    static constexpr float32 SUN_DISTANCE = 64.f;
    static constexpr float32 SUN_SPEED = 0.01f;
};

struct Graphics {
    static constexpr int32 DEBUG_WINDOW_WIDTH = 1920;
    static constexpr int32 DEBUG_WINDOW_HEIGHT = 1080;
    static constexpr float32 DEFAULT_FOV = 60.0f;
    static constexpr float32 CULLING_DISTANCE = (World::DRAW_RADIUS + 1) * World::CHUNK_SIZE;

    // CSM
    static constexpr int32 SHADOW_MAP_WIDTH = 4096;
    static constexpr int32 SHADOW_MAP_HEIGHT = 4096;
    static constexpr int32 SHADOW_MAP_CASCADE_COUNT = 3;
    static constexpr float32 SHADOW_NEAR_PLANE = 1.0f;
    static constexpr float32 SHADOW_FAR_PLANE = CULLING_DISTANCE;
};

struct Physics {
    static constexpr float32 GRAVITY = -100.0f;  //-50?
    static constexpr float32 MAX_ENTITY_SPEED = 100.0f;
    static constexpr float32 FRICTION_MULTIPLIER = 5.0f;
    static constexpr float32 FRICTION_CONSTANT = 30.0f;
    static constexpr float32 ACCELERATION_CONSTANT = 100.0f;
};

struct Player {
    static constexpr float32 HEIGHT = 1.8f;
    static constexpr float32 WIDTH = 0.8f;
    static constexpr float32 MOUSE_SENSITIVITY = 0.1f;
    static constexpr float32 JUMP_FORCE = 12.0f;
    static constexpr float32 ACCELERATION = 100.0f;
};

struct Game {
    static constexpr uint32 STAR_COUNT = 64 * 1024;
    static constexpr uint32 PARTICLE_LIMIT = 128;
    static constexpr uint32 ENTITY_LIMIT = 128;
    static constexpr float32 TARGET_FPS = 60.f;
};

struct System {
    static constexpr uint32 MAX_PATH_LEN = 260;
    static constexpr uint32 MAX_CONTROLLERS = 4;
};
}  // namespace Config