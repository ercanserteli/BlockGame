#include "Chunk.h"

#include <SDL_rwops.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AABB.h"
#include "Shader.h"
#include "glad/glad.h"
#include "noise/SimplexNoise.h"

// clang-format off
float32 cube_vertices_with_normal[] = {
    // -z
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    // +z
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    // -x
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

    // +x
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    // -y
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    // +y
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

Vector3f block_color_map[] = {
    {0.0f, 0.0f, 0.0f},
    {0.1f, 0.1f, 0.9f},
    {0.6f, 0.5f, 0.3f},
    {0.1f, 0.9f, 0.1f},
    {0.9f, 0.9f, 0.1f},
    {0.9f, 0.1f, 0.1f},
    {0.9f, 0.1f, 0.9f},
    {0.1f, 0.9f, 0.9f},
    {0.4f, 0.4f, 0.4f},
    {0.9f, 0.9f, 0.9f}
};
// clang-format on

float32 block_noise_values[Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE * 4 * 4];

void Chunk::initialize_open_gl_stuff(const bool do_update) {
    glGenVertexArrays(1, &vao_chunk);
    glBindVertexArray(vao_chunk);
    glGenBuffers(1, &vbo_chunk);
    if (do_update) {
        update();
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo_chunk);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float32), (void *)nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float32), (void *)(3 * sizeof(float32)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float32), (void *)(6 * sizeof(float32)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float32), (void *)(9 * sizeof(float32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}

void Chunk::draw(const int32 model_loc) const {
    if (vertex_count > 0) {
        const glm::vec3 position = {chunk_x * Config::World::CHUNK_SIZE, chunk_y * Config::World::CHUNK_SIZE, chunk_z * Config::World::CHUNK_SIZE};
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0f), position)));
        glBindVertexArray(vao_chunk);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }
}

int32 Chunk::get_vertex_ao(const int32 i, const int32 j, const int32 k, const Vector3f v, const Vector3f normal) const {
    int32 side1, side2, corner;
    int32 offset0;
    int32 offset1;
    int32 offset2;
    if (normal.x != 0) {
        offset0 = (int32)normal.x;
        offset1 = v.y > 0 ? 1 : -1;
        offset2 = v.z > 0 ? 1 : -1;
        side1 = get_block(i + offset0, j + offset1, k) > 0 ? 1 : 0;
        side2 = get_block(i + offset0, j, k + offset2) > 0 ? 1 : 0;
        corner = get_block(i + offset0, j + offset1, k + offset2) > 0 ? 1 : 0;
    } else if (normal.y != 0) {
        offset0 = (int32)normal.y;
        offset1 = v.x > 0 ? 1 : -1;
        offset2 = v.z > 0 ? 1 : -1;
        side1 = get_block(i + offset1, j + offset0, k) > 0 ? 1 : 0;
        side2 = get_block(i, j + offset0, k + offset2) > 0 ? 1 : 0;
        corner = get_block(i + offset1, j + offset0, k + offset2) > 0 ? 1 : 0;
    } else {
        offset0 = (int32)normal.z;
        offset1 = v.x > 0 ? 1 : -1;
        offset2 = v.y > 0 ? 1 : -1;
        side1 = get_block(i + offset1, j, k + offset0) > 0 ? 1 : 0;
        side2 = get_block(i, j + offset2, k + offset0) > 0 ? 1 : 0;
        corner = get_block(i + offset1, j + offset2, k + offset0) > 0 ? 1 : 0;
    }

    if (side1 && side2) {
        return 0;
    }
    return 3 - (side1 + side2 + corner);
}

void Chunk::fill_vertices(const int32 i, const int32 j, const int32 k, const int32 f, const Vector3f color, uint32 &attr_count, float32 *chunk_vertices) const {
    for (int32 v = 0; v < 6; v++) {
        chunk_vertices[attr_count++] = (cube_vertices_with_normal[f * 36 + v * 6] + i);
        chunk_vertices[attr_count++] = (cube_vertices_with_normal[f * 36 + v * 6 + 1] + j);
        chunk_vertices[attr_count++] = (cube_vertices_with_normal[f * 36 + v * 6 + 2] + k);
        for (int32 a = 3; a < 6; a++) {
            chunk_vertices[attr_count++] = (cube_vertices_with_normal[f * 36 + v * 6 + a]);
        }
        const float32 dx = cube_vertices_with_normal[f * 36 + v * 6] * 2.f;
        const float32 dz = cube_vertices_with_normal[f * 36 + v * 6 + 2] * 2.f;
        const float32 noise = get_noise_at(chunk_x, chunk_z, i, k, (int32)dx, (int32)dz);
        chunk_vertices[attr_count++] = (color.x) + noise;
        chunk_vertices[attr_count++] = (color.y) + noise;
        chunk_vertices[attr_count++] = (color.z) + noise;

        chunk_vertices[attr_count++] =
            (float32)get_vertex_ao(i, j, k, ((Vector3f *)cube_vertices_with_normal)[f * 12 + v * 2], ((Vector3f *)cube_vertices_with_normal)[f * 12 + v * 2 + 1]);
    }
}

void Chunk::update() {
    if (!blocks) {
        return;
    }

    float32 *chunk_vertices = chunk_map->temp_vertex_buffer;
    uint32 attr_count = 0;

    for (int32 i = 0; i < Config::World::CHUNK_SIZE; i++) {
        for (int32 j = 0; j < Config::World::CHUNK_SIZE; j++) {
            for (int32 k = 0; k < Config::World::CHUNK_SIZE; k++) {
                const uint8 block = blocks[BID(i, j, k)];
                const Vector3f color = block_color_map[block];
                if (block != 0) {
                    BlockPos neighbor;
                    neighbor = {chunk_x - 1, chunk_y, chunk_z, Config::World::CHUNK_SIZE - 1, j, k};
                    if ((i == 0 && chunk_map->get_block_at_block_pos(neighbor) == 0) || (i > 0 && blocks[BID(i - 1, j, k)] == 0)) {
                        fill_vertices(i, j, k, 2, color, attr_count, chunk_vertices);
                    }
                    neighbor = {chunk_x + 1, chunk_y, chunk_z, 0, j, k};
                    if ((i == Config::World::CHUNK_SIZE - 1 && chunk_map->get_block_at_block_pos(neighbor) == 0) ||
                        (i < Config::World::CHUNK_SIZE - 1 && blocks[BID(i + 1, j, k)] == 0)) {
                        fill_vertices(i, j, k, 3, color, attr_count, chunk_vertices);
                    }
                    neighbor = {chunk_x, chunk_y - 1, chunk_z, i, Config::World::CHUNK_SIZE - 1, k};
                    if ((j == 0 && chunk_map->get_block_at_block_pos(neighbor) == 0) || (j > 0 && blocks[BID(i, j - 1, k)] == 0)) {
                        fill_vertices(i, j, k, 4, color, attr_count, chunk_vertices);
                    }
                    neighbor = {chunk_x, chunk_y + 1, chunk_z, i, 0, k};
                    if ((j == Config::World::CHUNK_SIZE - 1 && chunk_map->get_block_at_block_pos(neighbor) == 0) ||
                        (j < Config::World::CHUNK_SIZE - 1 && blocks[BID(i, j + 1, k)] == 0)) {
                        fill_vertices(i, j, k, 5, color, attr_count, chunk_vertices);
                    }
                    neighbor = {chunk_x, chunk_y, chunk_z - 1, i, j, Config::World::CHUNK_SIZE - 1};
                    if ((k == 0 && chunk_map->get_block_at_block_pos(neighbor) == 0) || (k > 0 && blocks[BID(i, j, k - 1)] == 0)) {
                        fill_vertices(i, j, k, 0, color, attr_count, chunk_vertices);
                    }
                    neighbor = {chunk_x, chunk_y, chunk_z + 1, i, j, 0};
                    if ((k == Config::World::CHUNK_SIZE - 1 && chunk_map->get_block_at_block_pos(neighbor) == 0) ||
                        (k < Config::World::CHUNK_SIZE - 1 && blocks[BID(i, j, k + 1)] == 0)) {
                        fill_vertices(i, j, k, 1, color, attr_count, chunk_vertices);
                    }
                }
            }
        }
    }

    if (attr_count > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_chunk);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * attr_count, (void *)chunk_vertices, GL_DYNAMIC_DRAW);
    }
    vertex_count = attr_count / 10;
}

void Chunk::fill() {
    const int32 pos_x = chunk_x * Config::World::CHUNK_SIZE;
    const int32 pos_y = chunk_y * Config::World::CHUNK_SIZE;
    const int32 pos_z = chunk_z * Config::World::CHUNK_SIZE;

    for (int32 x = 0; x < Config::World::CHUNK_SIZE; x++) {
        for (int32 y = 0; y < Config::World::CHUNK_SIZE; y++) {
            for (int32 z = 0; z < Config::World::CHUNK_SIZE; z++) {
                constexpr float32 H = 32.f;
                SimplexNoise noise(1 / 128.f, 128.f);
                const float32 density = (noise.fractal(4, (float32)(pos_x + x), (float32)(pos_y + y), (float32)(pos_z + z)) * H) - (pos_y + y - H) * 0.5f;
                blocks[BID(x, y, z)] = (density > 0 || pos_y + y == 0) ? ((y / 4) + 1) : 0;
            }
        }
    }

    after_fill();
}

void Chunk::generate() {
    if (chunk_y > 2 || chunk_y < 0) {
        return;
    }
    if (!blocks) {
        blocks = pushArray(*chunk_map->world_arena, Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE, uint8);
    }

    memset(blocks, 0, Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE);

    chunk_map->push_to_be_filled(this);
}

AABB Chunk::get_aabb() {
    AABB res;
    const Vector3f size = {Config::World::CHUNK_SIZE, Config::World::CHUNK_SIZE, Config::World::CHUNK_SIZE};

    res.min = {(float32)(chunk_x * Config::World::CHUNK_SIZE), (float32)(chunk_y * Config::World::CHUNK_SIZE), (float32)(chunk_z * Config::World::CHUNK_SIZE)};
    res.max = res.min + size;
    return res;
}

void Chunk::initialize(const int32 chunk_x, const int32 chunk_y, const int32 chunk_z, ChunkMap *chunk_map) {
    this->chunk_x = chunk_x;
    this->chunk_y = chunk_y;
    this->chunk_z = chunk_z;
    this->chunk_map = chunk_map;
    this->next_in_hash = nullptr;

    if (this->chunk_y <= 2 && this->chunk_y >= 0) {
        generate();
    } else {
        initialize_open_gl_stuff(false);
    }
}

void Chunk::save_to_file() const {
    if (blocks != nullptr && filled && dirty) {
        char save_filename[Config::System::MAX_PATH_LEN];
        get_save_file_name(save_filename);
        SDL_RWops *fp = SDL_RWFromFile(save_filename, "wb");
        if (fp != nullptr) {
            const size_t num = SDL_RWwrite(fp, blocks, Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE, 1);
            SDL_RWclose(fp);
            if (num != 1) {
                LogError("Could not save chunk to file!");
            }
        } else {
            LogError("Could not open save file for saving!");
        }
    }
}

inline void update_neighbor(ChunkMap *chunk_map, int32 x, int32 y, int32 z) {
    if (Chunk *neighbor = chunk_map->get_chunk(x, y, z, false)) {
        neighbor->update();
    }
}

void Chunk::after_fill() {
    filled = true;
    initialize_open_gl_stuff(true);
    update_neighbor(chunk_map, chunk_x - 1, chunk_y, chunk_z);
    update_neighbor(chunk_map, chunk_x + 1, chunk_y, chunk_z);
    update_neighbor(chunk_map, chunk_x, chunk_y - 1, chunk_z);
    update_neighbor(chunk_map, chunk_x, chunk_y + 1, chunk_z);
    update_neighbor(chunk_map, chunk_x, chunk_y, chunk_z - 1);
    update_neighbor(chunk_map, chunk_x, chunk_y, chunk_z + 1);
}

bool Chunk::load_from_file() {
    char save_filename[Config::System::MAX_PATH_LEN];
    get_save_file_name(save_filename);
    SDL_RWops *fp = SDL_RWFromFile(save_filename, "rb");
    if (fp != nullptr) {
        const uint64 num = SDL_RWread(fp, blocks, Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE, 1);
        SDL_RWclose(fp);
        if (num != 1) {
            LogError("Could not load game memory from file!");
            return false;
        }
        after_fill();

        return true;
    }
    return false;
}

void Chunk::get_save_file_name(char *filename) const {
    ASSERT(snprintf(filename, Config::System::MAX_PATH_LEN, "%s%s/c_%d_%d_%d.erc", chunk_map->game_state->save_path, chunk_map->game_state->world_name, chunk_x,
                    chunk_y, chunk_z) > 0);
}

/////////////////////// ChunkMap /////////////////////////////////////

void ChunkMap::save() const {
    for (auto chunk : chunk_hash) {
        while (chunk) {
            chunk->save_to_file();
            chunk = chunk->next_in_hash;
        }
    }
}

void ChunkMap::initialize(GameState *state) {
    this->game_state = state;
    this->world_arena = &state->world_arena;
    temp_vertex_buffer = pushArray(state->scratch_arena, 5000000, float32);

    memset(to_be_filled, 0, sizeof(to_be_filled));

    // Pre-calculating noise values
    const SimplexNoise noise(1);
    constexpr int32 COUNT = Config::World::CHUNK_SIZE * 4;
    for (int32 i = 0; i < COUNT; i++) {
        for (int32 j = 0; j < COUNT; j++) {
            block_noise_values[i * COUNT + j] =
                noise.fractal(6, (float32)abs(i - COUNT / 2) / ((float32)COUNT / 2), (float32)abs(j - COUNT / 2) / ((float32)COUNT / 2));
        }
    }
}

// If arena is given, it will create the chunk if not found
Chunk *ChunkMap::get_chunk(const int32 chunk_x, const int32 chunk_y, const int32 chunk_z, const bool create) {
    const uint32 hash_value = chunk_x * 13 + chunk_y * 17 + chunk_z * 7;  // todo: better hash function
    const uint32 hash_slot = hash_value % 4096;

    Chunk *chunk = chunk_hash[hash_slot];
    if (!chunk && create) {
        chunk_hash[hash_slot] = pushStruct(*world_arena, Chunk);
        chunk = chunk_hash[hash_slot];
        chunk->initialize(chunk_x, chunk_y, chunk_z, this);
        return chunk;
    }

    while (chunk) {
        if (chunk->chunk_x == chunk_x && chunk->chunk_y == chunk_y && chunk->chunk_z == chunk_z) {
            // This chunk is the one we are looking for
            break;
        }

        // Chunk exists but not the one we are looking for, try next
        if (!chunk->next_in_hash && create) {
            chunk->next_in_hash = pushStruct(*world_arena, Chunk);
            chunk = chunk->next_in_hash;
            chunk->initialize(chunk_x, chunk_y, chunk_z, this);
            return chunk;
        }

        chunk = chunk->next_in_hash;
    }
    return chunk;
}

void ChunkMap::fill_next_chunk(const Vector3f &player_pos) {
    // todo: if chunk is too far now, don't fill it
    // todo: fill chunks that the player is currently looking at
    if (to_be_filled_len > 0) {
        float32 min_dist = 999999.f;
        uint32 min_index = 0;
        for (uint32 i = 0; i < to_be_filled_len; i++) {
            const Vector3f center = {(float32)to_be_filled[i]->chunk_x * Config::World::CHUNK_SIZE + Config::World::CHUNK_SIZE / 2,
                                     (float32)to_be_filled[i]->chunk_y * Config::World::CHUNK_SIZE + Config::World::CHUNK_SIZE / 2,
                                     (float32)to_be_filled[i]->chunk_z * Config::World::CHUNK_SIZE + Config::World::CHUNK_SIZE / 2};
            const float32 dist = sqr_dist(center, player_pos);
            if (dist < min_dist) {
                min_dist = dist;
                min_index = i;
            }
        }

        to_be_filled_len--;

        Chunk *temp = to_be_filled[to_be_filled_len];
        to_be_filled[to_be_filled_len] = to_be_filled[min_index];
        to_be_filled[min_index] = temp;

        if (!to_be_filled[to_be_filled_len]->load_from_file()) {
            to_be_filled[to_be_filled_len]->fill();
        }
    }
}

void ChunkMap::draw_chunks(const int32 model_loc, const Frustum &frustum, const Vector3f &player_pos) {
    // Fill a chunk if needed
    fill_next_chunk(player_pos);
    if (game_state->frame_count < 2) {
        fill_next_chunk(player_pos);
    }

    const int32 player_chunk_x = player_pos.x / Config::World::CHUNK_SIZE;
    const int32 player_chunk_y = player_pos.y / Config::World::CHUNK_SIZE;
    const int32 player_chunk_z = player_pos.z / Config::World::CHUNK_SIZE;

    constexpr int32 DRAW_RADIUS_SQR = Config::World::DRAW_RADIUS * Config::World::DRAW_RADIUS;
    for (int32 chunk_x = player_chunk_x - Config::World::DRAW_RADIUS; chunk_x < player_chunk_x + Config::World::DRAW_RADIUS; chunk_x++) {
        for (int32 chunk_y = player_chunk_y - Config::World::DRAW_RADIUS; chunk_y < player_chunk_y + Config::World::DRAW_RADIUS; chunk_y++) {
            for (int32 chunk_z = player_chunk_z - Config::World::DRAW_RADIUS; chunk_z < player_chunk_z + Config::World::DRAW_RADIUS; chunk_z++) {
                const int32 xd = chunk_x - player_chunk_x;
                const int32 yd = chunk_y - player_chunk_y;
                const int32 zd = chunk_z - player_chunk_z;
                if (xd * xd + yd * yd + zd * zd > DRAW_RADIUS_SQR) {
                    continue;
                }
                Chunk *chunk = get_chunk(chunk_x, chunk_y, chunk_z, world_arena);
                if (frustum.test_intersection(chunk->get_aabb()) != Frustum::TEST_OUTSIDE) {
                    chunk->draw(model_loc);
                }
            }
        }
    }
}

void ChunkMap::push_to_be_filled(Chunk *chunk) {
    to_be_filled[to_be_filled_len] = chunk;
    to_be_filled_len++;
}

inline bool is_block_pos_valid(const BlockPos &b_pos) {
    return b_pos.block_x < Config::World::CHUNK_SIZE && b_pos.block_y < Config::World::CHUNK_SIZE && b_pos.block_z < Config::World::CHUNK_SIZE &&
           b_pos.block_x >= 0 && b_pos.block_y >= 0 && b_pos.block_z >= 0;
}

uint8 ChunkMap::get_block_at_block_pos(const BlockPos &b_pos, const bool create_chunk) {
    if (!is_block_pos_valid(b_pos)) {
        return 0;
    }
    const Chunk *chunk = get_chunk(b_pos.chunk_x, b_pos.chunk_y, b_pos.chunk_z, create_chunk);
    if (!chunk || !chunk->blocks) {
        return 0;
    }
    return chunk->blocks[BID(b_pos.block_x, b_pos.block_y, b_pos.block_z)];
}

uint8 ChunkMap::get_block_at_pos(const Vector3f pos) {
    BlockPos b_pos;
    pos_to_block_pos(pos, b_pos);
    return get_block_at_block_pos(b_pos);
}

void ChunkMap::change_block_at_block_pos(const BlockPos &b_pos, const uint8 new_block) {
    if (!is_block_pos_valid(b_pos)) {
        return;
    }

    Chunk *chunk = get_chunk(b_pos.chunk_x, b_pos.chunk_y, b_pos.chunk_z);
    if (!chunk) {
        return;
    }

    chunk->dirty = true;
    if (!chunk->blocks) {
        // If chunk is empty, create the blocks array
        chunk->blocks = pushArray(*world_arena, Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE * Config::World::CHUNK_SIZE, uint8);
    }
    if (chunk->blocks[BID(b_pos.block_x, b_pos.block_y, b_pos.block_z)] != new_block) {
        chunk->blocks[BID(b_pos.block_x, b_pos.block_y, b_pos.block_z)] = new_block;
        chunk->update();

        // May need to update neighboring chunks
        if (b_pos.block_x == 0) {
            const BlockPos neighbor = {b_pos.chunk_x - 1, b_pos.chunk_y, b_pos.chunk_z, Config::World::CHUNK_SIZE - 1, b_pos.block_y, b_pos.block_z};
            if (get_block_at_block_pos(neighbor) != 0) {
                // No need to check chunk pointer because get_block_at_block_pos
                // returned non-zero
                get_chunk(b_pos.chunk_x - 1, b_pos.chunk_y, b_pos.chunk_z)->update();
            }
        }
        if (b_pos.block_x == Config::World::CHUNK_SIZE - 1) {
            const BlockPos neighbor = {b_pos.chunk_x + 1, b_pos.chunk_y, b_pos.chunk_z, 0, b_pos.block_y, b_pos.block_z};
            if (get_block_at_block_pos(neighbor) != 0) {
                get_chunk(b_pos.chunk_x + 1, b_pos.chunk_y, b_pos.chunk_z)->update();
            }
        }
        if (b_pos.block_y == 0) {
            const BlockPos neighbor = {b_pos.chunk_x, b_pos.chunk_y - 1, b_pos.chunk_z, b_pos.block_x, Config::World::CHUNK_SIZE - 1, b_pos.block_z};
            if (get_block_at_block_pos(neighbor) != 0) {
                get_chunk(b_pos.chunk_x, b_pos.chunk_y - 1, b_pos.chunk_z)->update();
            }
        }
        if (b_pos.block_y == Config::World::CHUNK_SIZE - 1) {
            const BlockPos neighbor = {b_pos.chunk_x, b_pos.chunk_y + 1, b_pos.chunk_z, b_pos.block_x, 0, b_pos.block_z};
            if (get_block_at_block_pos(neighbor) != 0) {
                get_chunk(b_pos.chunk_x, b_pos.chunk_y + 1, b_pos.chunk_z)->update();
            }
        }
        if (b_pos.block_z == 0) {
            const BlockPos neighbor = {b_pos.chunk_x, b_pos.chunk_y, b_pos.chunk_z - 1, b_pos.block_x, b_pos.block_y, Config::World::CHUNK_SIZE - 1};
            if (get_block_at_block_pos(neighbor) != 0) {
                get_chunk(b_pos.chunk_x, b_pos.chunk_y, b_pos.chunk_z - 1)->update();
            }
        }
        if (b_pos.block_z == Config::World::CHUNK_SIZE - 1) {
            const BlockPos neighbor = {b_pos.chunk_x, b_pos.chunk_y, b_pos.chunk_z + 1, b_pos.block_x, b_pos.block_y, 0};
            if (get_block_at_block_pos(neighbor) != 0) {
                get_chunk(b_pos.chunk_x, b_pos.chunk_y, b_pos.chunk_z + 1)->update();
            }
        }
    }
}

void ChunkMap::update_all_chunks(const Vector3f &player_pos) {
    const int32 player_chunk_x = player_pos.x / Config::World::CHUNK_SIZE;
    const int32 player_chunk_y = player_pos.y / Config::World::CHUNK_SIZE;
    const int32 player_chunk_z = player_pos.z / Config::World::CHUNK_SIZE;
    for (int32 chunk_x = player_chunk_x - Config::World::DRAW_RADIUS; chunk_x < player_chunk_x + Config::World::DRAW_RADIUS; chunk_x++) {
        for (int32 chunk_y = player_chunk_y - Config::World::DRAW_RADIUS; chunk_y < player_chunk_y + Config::World::DRAW_RADIUS; chunk_y++) {
            for (int32 chunk_z = player_chunk_z - Config::World::DRAW_RADIUS; chunk_z < player_chunk_z + Config::World::DRAW_RADIUS; chunk_z++) {
                get_chunk(chunk_x, chunk_y, chunk_z, world_arena)->update();
            }
        }
    }
}
