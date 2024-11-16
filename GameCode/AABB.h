#pragma once

#include "Chunk.h"
#include "Config.h"
#include "Definitions.h"
#include "Geometry.h"

struct AABB {
    Vector3f min;
    Vector3f max;

    Vector3f get_positive_vertex(const Vector3f &normal) const {
        Vector3f positive_vertex = min;

        if (normal.x >= 0.0f) positive_vertex.x = max.x;
        if (normal.y >= 0.0f) positive_vertex.y = max.y;
        if (normal.z >= 0.0f) positive_vertex.z = max.z;

        return positive_vertex;
    }

    Vector3f get_negative_vertex(const Vector3f &normal) const {
        Vector3f negative_vertex = max;

        if (normal.x >= 0.0f) negative_vertex.x = min.x;
        if (normal.y >= 0.0f) negative_vertex.y = min.y;
        if (normal.z >= 0.0f) negative_vertex.z = min.z;

        return negative_vertex;
    }
};

inline AABB get_player_box(const Vector3f &player_pos) {
    AABB player_box = {player_pos, player_pos};
    player_box.min.x -= Config::Player::WIDTH / 2;
    player_box.min.z -= Config::Player::WIDTH / 2;
    player_box.min.y -= Config::Player::HEIGHT;
    player_box.max.x += Config::Player::WIDTH / 2;
    player_box.max.z += Config::Player::WIDTH / 2;
    return player_box;
}

inline AABB get_cube_box(const Vector3f &pos, const float32 side_length) {
    AABB box = {pos, pos};
    box.min.x -= side_length / 2;
    box.min.y -= side_length / 2;
    box.min.z -= side_length / 2;
    box.max.x += side_length / 2;
    box.max.y += side_length / 2;
    box.max.z += side_length / 2;
    return box;
}

inline AABB get_block_box(const BlockPos &b_pos) {
    const Vector3f bottom_left = {b_pos.chunk_x * Config::World::CHUNK_SIZE + b_pos.block_x - 0.5f,
                                  b_pos.chunk_y * Config::World::CHUNK_SIZE + b_pos.block_y - 0.5f,
                                  b_pos.chunk_z * Config::World::CHUNK_SIZE + b_pos.block_z - 0.5f};
    const Vector3f top_right = {b_pos.chunk_x * Config::World::CHUNK_SIZE + b_pos.block_x + 0.5f,
                                b_pos.chunk_y * Config::World::CHUNK_SIZE + b_pos.block_y + 0.5f,
                                b_pos.chunk_z * Config::World::CHUNK_SIZE + b_pos.block_z + 0.5f};
    const AABB block_box = {bottom_left, top_right};
    return block_box;
}

bool aabb_check(const AABB &b1, const AABB &b2);
AABB get_swept_broadphase_aabb(const AABB &b, const Vector3f &vel);
float32 swept_aabb_check(const AABB &b1, const AABB &b2, const Vector3f &speed, Vector3f &normal);