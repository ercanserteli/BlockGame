#pragma once
#include "Config.h"
#include "Definitions.h"
#include "Frustum.h"
#include "Geometry.h"
#include "Utility.h"

#define BID(x, y, z) (((z) * Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE) + ((y) * Config::World::CHUNK_SIZE) + ((x)))

struct MainShader;
struct GameState;
struct ChunkMap;
struct MemoryArena;

struct Chunk {
    Chunk() = default;
    void fill();
    void initialize(int32 chunk_x, int32 chunk_y, int32 chunk_z, ChunkMap *chunk_map);
    void initialize_open_gl_stuff(bool do_update);
    void after_fill();
    void draw(int32 model_loc) const;
    void update();
    void generate();
    void fill_vertices(int32 i, int32 j, int32 k, int32 f, Vector3f color, uint32 &attr_count, float32 *chunk_vertices) const;
    int32 get_vertex_ao(int32 i, int32 j, int32 k, Vector3f v, Vector3f normal) const;
    AABB get_aabb();
    void save_to_file() const;
    bool load_from_file();
    void get_save_file_name(char *filename) const;
    uint8 get_block(const int32 x, const int32 y, const int32 z) const {
        if (!blocks || x >= Config::World::CHUNK_SIZE || x < 0 || y >= Config::World::CHUNK_SIZE || y < 0 || z >= Config::World::CHUNK_SIZE || z < 0) {
            return 0;
        }
        return blocks[BID(x, y, z)];
    }

    uint32 vbo_chunk = 0;
    uint32 vao_chunk = 0;
    uint32 vertex_count = 0;
    int32 chunk_x = 0;
    int32 chunk_y = 0;
    int32 chunk_z = 0;
    uint8 *blocks = nullptr;
    bool filled = false;
    bool dirty = false;
    ChunkMap *chunk_map = nullptr;
    Chunk *next_in_hash = nullptr;
};

struct BlockPos {
    int32 chunk_x = 0;
    int32 chunk_y = 0;
    int32 chunk_z = 0;
    int32 block_x = 0;
    int32 block_y = 0;
    int32 block_z = 0;

    int32 get_x() const { return chunk_x * Config::World::CHUNK_SIZE + block_x; }
    int32 get_y() const { return chunk_y * Config::World::CHUNK_SIZE + block_y; }
    int32 get_z() const { return chunk_z * Config::World::CHUNK_SIZE + block_z; }
};

struct ChunkMap {
    void initialize(GameState *state);
    void draw_chunks(int32 model_loc, const Frustum &frustum, const Vector3f &player_pos);
    uint8 get_block_at_block_pos(const BlockPos &b_pos, bool create_chunk = false);
    uint8 get_block_at_pos(Vector3f pos);
    void change_block_at_block_pos(const BlockPos &b_pos, uint8 new_block);
    Chunk *get_chunk(int32 chunk_x, int32 chunk_y, int32 chunk_z, bool create = true);
    void push_to_be_filled(Chunk *chunk);
    void fill_next_chunk(const Vector3f &player_pos);
    void save() const;
    void update_all_chunks(const Vector3f &player_pos);

    MemoryArena *world_arena = nullptr;
    float32 *temp_vertex_buffer = nullptr;
    Chunk *chunk_hash[4096] = {};  // todo: pick a better size
    Chunk *to_be_filled[Config::World::DRAW_RADIUS * Config::World::DRAW_RADIUS * Config::World::DRAW_RADIUS * 8] = {};
    uint32 to_be_filled_len = 0;
    GameState *game_state = nullptr;
};

extern float32 block_noise_values[];
extern float32 cube_vertices_with_normal[];
extern Vector3f * block_color_map;

inline void pos_to_block_pos(const Vector3f pos, BlockPos &block_pos) {
    block_pos.chunk_x = (int32)floor((pos.x + 0.5f) / (float32)Config::World::CHUNK_SIZE);
    block_pos.chunk_y = (int32)floor((pos.y + 0.5f) / (float32)Config::World::CHUNK_SIZE);
    block_pos.chunk_z = (int32)floor((pos.z + 0.5f) / (float32)Config::World::CHUNK_SIZE);
    block_pos.block_x = mod((int32)round(pos.x), Config::World::CHUNK_SIZE);
    block_pos.block_y = mod((int32)round(pos.y), Config::World::CHUNK_SIZE);
    block_pos.block_z = mod((int32)round(pos.z), Config::World::CHUNK_SIZE);
}

inline Vector3f block_pos_to_pos(const BlockPos &block_pos) {
    return {
        (float32)((block_pos.chunk_x * Config::World::CHUNK_SIZE) + block_pos.block_x),
        (float32)((block_pos.chunk_y * Config::World::CHUNK_SIZE) + block_pos.block_y),
        (float32)((block_pos.chunk_z * Config::World::CHUNK_SIZE) + block_pos.block_z)
    };
}

inline void canonicalize_block_pos(BlockPos &block_pos) {
    if (block_pos.block_x >= Config::World::CHUNK_SIZE || block_pos.block_x < 0) {
        block_pos.chunk_x += (int32)floor((float32)block_pos.block_x / (float32)Config::World::CHUNK_SIZE);
        block_pos.block_x = mod(block_pos.block_x, Config::World::CHUNK_SIZE);
    }
    if (block_pos.block_y >= Config::World::CHUNK_SIZE || block_pos.block_y < 0) {
        block_pos.chunk_y += (int32)floor((float32)block_pos.block_y / (float32)Config::World::CHUNK_SIZE);
        block_pos.block_y = mod(block_pos.block_y, Config::World::CHUNK_SIZE);
    }
    if (block_pos.block_z >= Config::World::CHUNK_SIZE || block_pos.block_z < 0) {
        block_pos.chunk_z += (int32)floor((float32)block_pos.block_z / (float32)Config::World::CHUNK_SIZE);
        block_pos.block_z = mod(block_pos.block_z, Config::World::CHUNK_SIZE);
    }
}

inline float32 get_noise_at(const int32 chunk_x, const int32 chunk_z, const int32 x, const int32 z, const int32 dx=0, const int32 dz=0) {
    const int32 bx = chunk_x * Config::World::CHUNK_SIZE + x;
    const int32 bz = chunk_z * Config::World::CHUNK_SIZE + z;
    constexpr uint32 B = Config::World::CHUNK_SIZE * 4;

    if (dx == 0 && dz == 0) {
        return block_noise_values[mod(bx, B) * B + mod(bz, B)] * 0.25f;
    }
    return (block_noise_values[mod(bx, B) * B + mod(bz, B)] * 2.0f + block_noise_values[mod(bx + dx, B) * B + mod(bz + dz, B)] +
            block_noise_values[mod(bx + dx, B) * B + mod(bz, B)] + block_noise_values[mod(bx, B) * B + mod(bz + dz, B)]) *
           0.05f;
}
