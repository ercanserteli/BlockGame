#pragma once
#include "Definitions.h"
#include "GameBase.h"

struct ChunkMap;
struct AABB;
struct Vector3f;
struct BlockPos;

float32 detect_collision(ChunkMap &chunk_map, const AABB &box, Vector3f movement, Vector3f &cc_normal, BlockPos &cc_bpos);
bool handle_collision(Player &player, ChunkMap &chunk_map, Vector3f &new_pos);
uint8 find_block_in_front(ChunkMap &chunk_map, Vector3f org, Vector3f dir, BlockPos &b_pos_result, BlockPos &front_b_pos_result);