#include "Collision.h"

#include "AABB.h"

float32 detect_collision(ChunkMap &chunk_map, const AABB &box, Vector3f movement, Vector3f &cc_normal, BlockPos &cc_bpos) {
    const AABB broad_phase_box = get_swept_broadphase_aabb(box, movement);

    // Define the box to check for collisions in
    const auto start_x = (int32)floor(broad_phase_box.min.x);
    const auto start_y = (int32)floor(broad_phase_box.min.y);
    const auto start_z = (int32)floor(broad_phase_box.min.z);
    const auto end_x = (int32)ceil(broad_phase_box.max.x);
    const auto end_y = (int32)ceil(broad_phase_box.max.y);
    const auto end_z = (int32)ceil(broad_phase_box.max.z);

    float32 cc_t = 1.0f;
    for (int32 x = start_x; x <= end_x; x++) {
        for (int32 y = start_y; y <= end_y; y++) {
            for (int32 z = start_z; z <= end_z; z++) {
                BlockPos b_pos;
                pos_to_block_pos(Vector3f(x, y, z), b_pos);
                const uint8 block = chunk_map.get_block_at_block_pos(b_pos);
                if (block != 0) {
                    AABB block_box = get_block_box(b_pos);

                    // Basic AABB intersection check with the broad phase box
                    if (aabb_check(broad_phase_box, block_box)) {
                        Vector3f normal;
                        const float32 t = swept_aabb_check(box, block_box, movement, normal);
                        if (t < 1.0f) {
                            // collision
                            if (cc_t > t) {
                                cc_t = t;
                                cc_normal = normal;
                                cc_bpos = b_pos;
                            }
                        }
                    }
                }
            }
        }
    }
    return cc_t;
}

bool handle_collision(Player &player, ChunkMap &chunk_map, Vector3f &new_pos) {
    constexpr float32 EPSILON = 1.0f / 1024.0f;
    Vector3f player_movement = new_pos - player.pos;
    const AABB player_box = get_player_box(player.pos);

    // cc is shorthand for closest collision
    Vector3f cc_normal;
    BlockPos cc_bpos;
    const float32 cc_t = detect_collision(chunk_map, player_box, player_movement, cc_normal, cc_bpos);

    if (cc_t < 1.0f) {
        LogDebug("Collision at t=%f, normal=%f,%f,%f\n", cc_t, cc_normal.x, cc_normal.y, cc_normal.z);
        const float32 remaining_time = 1.0f - cc_t;
        player.pos += (player_movement * cc_t) + (cc_normal * EPSILON);

        BlockPos player_bpos;

        BlockPos cc_top_bpos = cc_bpos;
        cc_top_bpos.block_y += 1;
        canonicalize_block_pos(cc_top_bpos);

        BlockPos cc_top2_bpos = cc_top_bpos;
        cc_top2_bpos.block_y += 1;
        canonicalize_block_pos(cc_top2_bpos);

        pos_to_block_pos(player.pos, player_bpos);
        if (cc_normal.y == 0 && (cc_bpos.get_y() < player_bpos.get_y()) && chunk_map.get_block_at_block_pos(cc_top_bpos) == 0 &&
            chunk_map.get_block_at_block_pos(cc_top2_bpos) == 0) {
            // Running into a 1-high block moves you on top of it
            player_movement = player_movement * remaining_time;
            player.pos.y += 1;
        } else {
            player_movement = player_movement * remaining_time;
            player_movement = player_movement - (cc_normal * dot(player_movement, cc_normal));
        }
        new_pos = player.pos + player_movement;

        if (cc_normal.y == 1.0f || cc_normal.y == -1.0f) {
            player.on_ground = true;
            player.speed.y = 0;
        }
        return true;
    }
    return false;
}

uint8 find_block_in_front(ChunkMap &chunk_map, Vector3f org, const Vector3f dir, BlockPos &b_pos_result, BlockPos &front_b_pos_result) {
    constexpr float32 MAX_DIST = 5.0f * Config::Player::HEIGHT;

    float32 dist = 0;
    while (dist <= MAX_DIST) {
        constexpr float32 INTERVAL = 0.1f;
        Vector3f delta = dir * dist;
        Vector3f pos = org + delta;
        uint8 block = chunk_map.get_block_at_pos(pos);
        if (block > 0) {
            constexpr float32 MINI_INTERVAL = 0.01f;
            const float32 rough_dist = dist;
            dist = dist - INTERVAL + MINI_INTERVAL;
            while (dist <= rough_dist) {
                delta = dir * dist;
                pos = org + delta;
                block = chunk_map.get_block_at_pos(pos);
                if (block > 0) {
                    pos_to_block_pos(pos, b_pos_result);
                    Vector3f center_pos = block_pos_to_pos(b_pos_result);
                    float32 min_t = 1.0f;

                    for (int32 i = 0; i < 3; i++) {
                        for (int32 j = 0; j <= 1; j++) {
                            const float32 d = -0.5f + j;
                            const float32 t = (center_pos[i] + d - org[i]) / delta[i];
                            Vector3f intersection = org + delta * t;
                            if (intersection[(i + 1) % 3] < center_pos[(i + 1) % 3] + 0.5f && intersection[(i + 1) % 3] > center_pos[(i + 1) % 3] - 0.5f &&
                                intersection[(i + 2) % 3] < center_pos[(i + 2) % 3] + 0.5f && intersection[(i + 2) % 3] > center_pos[(i + 2) % 3] - 0.5f) {
                                // intersects
                                if (t < min_t) {
                                    min_t = t;
                                    Vector3f front_pos = center_pos;
                                    front_pos[i] += d * 2;
                                    pos_to_block_pos(front_pos, front_b_pos_result);
                                    if (chunk_map.get_block_at_block_pos(front_b_pos_result) > 0) {
                                        // There is a block in front => this block should not be selected
                                        return 0;
                                    }
                                }
                            }
                        }
                    }

                    return block;
                }
                dist += MINI_INTERVAL;
            }
        }

        dist += INTERVAL;
    }
    return 0;
}