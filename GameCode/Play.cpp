#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AABB.h"
#include "Chunk.h"
#include "Collision.h"
#include "GameBase.h"
#include "Geometry.h"
#include "ShadowDebugVisuals.h"
#include "Sound.h"

namespace Play {
void spawn_particles(GameState *state, const Vector3f &pos, const Vector3f &color, const float32 scale, const int32 count) {
    for (uint32 i = state->particle_count; i < MIN(state->particle_count + count, Config::Game::PARTICLE_LIMIT); i++) {
        state->particles[i].initialize(pos, color, scale);
    }
    state->particle_count = MIN(state->particle_count + count, Config::Game::PARTICLE_LIMIT);
}

BlockEntity *spawn_block_entity(GameState *state, const Vector3f &pos, const Vector3f &color) {
    if (state->entity_count < Config::Game::ENTITY_LIMIT) {
        state->entities[state->entity_count] = BlockEntity();
        state->entities[state->entity_count].pos = pos;
        state->entities[state->entity_count].color = color;
        state->entities[state->entity_count].bound = true;
        state->entities[state->entity_count].rot_speed = {rand_float() * 4.f - 2.0f, rand_float() * 4.f - 2.0f};
        state->entity_count++;
        return &state->entities[state->entity_count - 1];
    }
    return nullptr;
}

void update_sun(GameState *state, float32 time_delta, const ControllerInput *controller, const ControllerInput *last_controller) {
    Sun &sun = state->sun;

    // Toggle sun speed boost with F4
    if (controller->button_f4 && !last_controller->button_f4) {
        sun.speed_boost = sun.speed_boost == 1.f ? 20.f : 1.f;
    }

    sun.age = fmod(sun.age + time_delta * Config::World::SUN_SPEED * sun.speed_boost, 2.f * PI32);

    const float32 sun_sin = sinf(sun.age);
    const float32 sun_cos = cosf(sun.age);

    sun.pos.y = Config::World::SUN_DISTANCE * sun_sin;
    sun.pos.z = Config::World::SUN_DISTANCE * sun_cos;

    const Vector3f sun_color_morning = {1.0f, 1.0f, 0.9f};
    const Vector3f sun_color_evening = {0.35f, 0.35f, 1.0f};
    sun.color = (sun_color_morning - sun_color_evening) * (sun_sin * 0.5f + 0.5f) + sun_color_evening;

    const Vector3f sky_color = {0.53f, 0.81f, 0.98f};
    sun.sky_color = sky_color * (sun_sin * 0.5f + 0.5f);

    const glm::quat rot = {1.0f, 0.0f, 0.0f, 0.0f};
    sun.rot = glm::rotate(rot, -sun.age, {1, 0, 0});

    sun.diffuse_strength = MIN(0.85f, MAX(0.0f, sun_sin));
    sun.specular_strength = sun_sin > 0 ? 1.f : 0.f;

    sun.star_visibility = MIN(1.0f, MAX(0.0f, -sun_sin + 0.25f));
}

void update_particles(GameState *state, float32 time_delta) {
    for (uint32 i = 0; i < state->particle_count; i++) {
        state->particles[i].age += time_delta;
        if (state->particles[i].age >= 1.5f) {
            state->particles[i] = state->particles[state->particle_count - 1];
            state->particle_count--;
        }
        state->particles[i].pos += state->particles[i].speed * time_delta;
        state->particles[i].speed.y -= 25.f * time_delta;

        state->particles[i].rotation = glm::rotate(state->particles[i].rotation, state->particles[i].rot_speed.x * time_delta, glm::vec3(1, 0, 0));
        state->particles[i].rotation = glm::rotate(state->particles[i].rotation, state->particles[i].rot_speed.y * time_delta, glm::vec3(0, 1, 0));

        Vector3f bottom_point = state->particles[i].pos;
        Vector3f top_point = state->particles[i].pos;
        bottom_point.y -= state->particles[i].scale * 0.5f;
        top_point.y += state->particles[i].scale * 0.5f;
        if (state->chunk_map.get_block_at_pos(bottom_point) > 0 && state->chunk_map.get_block_at_pos(top_point) == 0) {
            BlockPos block_bottom;
            pos_to_block_pos(bottom_point, block_bottom);
            state->particles[i].pos.y = block_pos_to_pos(block_bottom).y + 0.5f + state->particles[i].scale * 0.5f;
            state->particles[i].speed.y = 0;
            const Vector3f direction = state->particles[i].speed.get_normalized();
            state->particles[i].speed -= (state->particles[i].speed * time_delta + direction * (time_delta * 5));
            state->particles[i].rot_speed -= (state->particles[i].rot_speed * (time_delta * 5));
        }
    }
}

void update_entities(GameState *state, float32 time_delta) {
    for (uint32 i = 0; i < state->entity_count; i++) {
        state->entities[i].age += time_delta;
        Vector3f old_pos = state->entities[i].pos;

        if (state->entities[i].bound) {
            float32 progress = lin2exp(MIN(state->entities[i].age / 2.f, 1.f), true);

            Vector3f start_pos = state->player.pos + state->player.direction * state->entities[i].bound_offset.get_magnitude();
            Vector3f side_vector = cross({0, 1, 0}, state->player.direction).get_normalized();
            Vector3f end_pos = state->player.pos + state->player.direction * 2.0f + side_vector * (-1.0f);
            end_pos.y += 0.8f;
            state->entities[i].pos = start_pos + (end_pos - start_pos) * progress;
            glm::quat start_rot = {1, 0, 0, 0};
            glm::quat end_rot = {1, 0, 0, 0};
            end_rot = glm::rotate(end_rot, -2.0f, side_vector.as_vec3());
            end_rot = glm::rotate(end_rot, -glm::radians(state->player.yaw), glm::vec3(0, 1, 0));
            state->entities[i].rotation = glm::normalize(start_rot + (end_rot - start_rot) * progress);
        } else {
            if (state->entities[i].age >= 12.f) {
                state->entities[i] = state->entities[state->entity_count - 1];
                if (state->player.hand_entity == &state->entities[state->entity_count - 1]) {
                    state->player.hand_entity = &state->entities[i];
                }
                state->entity_count--;
            }

            Vector3f movement = state->entities[i].speed * time_delta;
            state->entities[i].pos += movement;
            state->entities[i].speed.y = MAX(state->entities[i].speed.y - 25.f * time_delta, -150.0f);

            state->entities[i].rotation = glm::rotate(state->entities[i].rotation, state->entities[i].rot_speed.x * time_delta, glm::vec3(1, 0, 0));
            state->entities[i].rotation = glm::rotate(state->entities[i].rotation, state->entities[i].rot_speed.y * time_delta, glm::vec3(0, 1, 0));

            BlockPos org_block_pos;
            pos_to_block_pos(state->entities[i].pos, org_block_pos);

            BlockPos hit_block_pos;
            AABB entity_box = get_cube_box(state->entities[i].pos - movement, 1.0f);
            Vector3f cc_normal;
            float32 cc_t = detect_collision(state->chunk_map, entity_box, movement, cc_normal, hit_block_pos);
            if (cc_t < 1.0f) {
                Vector3f hit_pos;
                hit_pos = block_pos_to_pos(hit_block_pos);

                Vector3f reverse_dir = (old_pos - state->entities[i].pos).get_normalized();
                Vector3f break_pos = state->entities[i].pos + reverse_dir;
                spawn_particles(state, break_pos, state->entities[i].color, 0.33f, 8);

                if (state->entities[i].speed.get_magnitude() > 80.0f) {
                    for (int32 dx = -1; dx <= 1; dx++) {
                        for (int32 dy = -1; dy <= 1; dy++) {
                            for (int32 dz = -1; dz <= 1; dz++) {
                                Vector3f to_break_pos;
                                to_break_pos = hit_pos;
                                to_break_pos.x += (float32)dx;
                                to_break_pos.y += (float32)dy;
                                to_break_pos.z += (float32)dz;
                                BlockPos to_break_b_pos;
                                pos_to_block_pos(to_break_pos, to_break_b_pos);
                                uint8 to_break_block = state->chunk_map.get_block_at_block_pos(to_break_b_pos);
                                if (to_break_block > 0) {
                                    state->chunk_map.change_block_at_block_pos(to_break_b_pos, 0);
                                    spawn_particles(state, to_break_pos, block_color_map[to_break_block], 0.33f, 6);
                                }
                            }
                        }
                    }
                } else if (state->entities[i].speed.get_magnitude() > 50.0f) {
                    uint8 to_break_block = state->chunk_map.get_block_at_block_pos(hit_block_pos);
                    if (to_break_block > 0) {
                        state->chunk_map.change_block_at_block_pos(hit_block_pos, 0);
                        spawn_particles(state, hit_pos, block_color_map[to_break_block], 0.33f, 8);
                    }
                }

                // shitty volume scaling based on distance
                float32 dist = (state->entities[i].pos - state->player.pos).get_magnitude();
                float32 ratio = powf(1.0f - MIN((dist) / 200.0f, 1.0f), 2);
                int32 vol = Sound::get_volume();
                Sound::set_volume(2, (int32)((float32)vol * ratio));
                Sound::play(Sound::break_sound, 2);

                state->entities[i] = state->entities[state->entity_count - 1];
                if (state->player.hand_entity == &state->entities[state->entity_count - 1]) {
                    state->player.hand_entity = &state->entities[i];
                }
                state->entity_count--;
            }
        }
    }
}

void update_fov(GameState *state, float32 time_delta, const ControllerInput *controller) {
    if (controller->button_l) {
        state->player.fov += time_delta * 15.f;
        if (state->player.fov > 90.0f) {
            state->player.fov = 90.0f;
        }
        LogDebug("FOV: %f\n", state->player.fov);
    }
    if (controller->button_r) {
        state->player.fov -= time_delta * 15.f;
        if (state->player.fov < 10.0f) {
            state->player.fov = 10.0f;
        }
        LogDebug("FOV: %f\n", state->player.fov);
    }
}

void update_player(GameState *state, float32 time_delta, ControllerInput *controller, const ControllerInput *last_controller, BlockPos &b_pos_pointing,
                   uint8 &block_pointing) {
    Vector3f acceleration = {};
    Vector3f walk_direction = {};
    Vector3f forward_direction = state->player.direction;
    forward_direction.y = 0;

    BlockPos b_pos_pointing_front;
    block_pointing = find_block_in_front(state->chunk_map, state->player.pos, state->player.direction, b_pos_pointing, b_pos_pointing_front);

    static float32 block_break_cooldown = 0.f;
    static float32 block_put_cooldown = 0.f;

    // Go forward
    if (controller->button_b) {
        walk_direction += forward_direction;
    }
    // Go back
    if (controller->button_a) {
        walk_direction -= forward_direction;
    }
    // Go left
    if (controller->button_c) {
        Vector3f right = cross(forward_direction, {0, 1, 0});
        right.normalize();
        walk_direction -= right;
    }
    // Go right
    if (controller->button_d) {
        Vector3f right = cross(forward_direction, {0, 1, 0});
        right.normalize();
        walk_direction += right;
    }
    // Go up
    if (controller->button_l2) {
        const Vector3f down = {0, -1, 0};
        acceleration -= down * Config::Player::ACCELERATION;
        state->player.on_ground = false;
    }
    // Jump
    if (controller->button_r2) {
        if (state->player.on_ground) {
            const Vector3f up = {0, 1, 0};
            acceleration += up * Config::Player::ACCELERATION * 10;
            state->player.on_ground = false;
        }
    }
    // Change player mode (build/throw)
    if (!last_controller->button_l3 && controller->button_l3) {
        state->player.throw_mode = !state->player.throw_mode;
    }

    // Toggle debug visuals with F3
    if (!last_controller->button_f3 && controller->button_f3) {
        state->debug_visuals_enabled = !state->debug_visuals_enabled;
        if (state->debug_visuals_enabled) {
            DebugVisuals::frustums_initialized = false;
        }
    }

    if (walk_direction.get_sqr_magnitude() > 0) {
        if (state->player.on_ground && !Sound::is_playing(1)) {
            Sound::play(Sound::step_sound, 1);
        }
    }

    // Mouse left click
    if (controller->button_mouse_l) {
        if (block_pointing > 0 && block_break_cooldown == 0.f) {
            state->chunk_map.change_block_at_block_pos(b_pos_pointing, 0);
            block_break_cooldown = 0.3f;
            Sound::play(Sound::break_sound);

            // Spawning particles
            const Vector3f break_pos = block_pos_to_pos(b_pos_pointing);
            Vector3f color = block_color_map[block_pointing];
            const float32 noise = get_noise_at(b_pos_pointing.chunk_x, b_pos_pointing.chunk_z, b_pos_pointing.block_x, b_pos_pointing.block_z);
            color.x += noise;
            color.y += noise;
            color.z += noise;
            spawn_particles(state, break_pos, color, 0.33f, 8);
        }
    } else {
        block_break_cooldown = 0;
    }

    // Mouse right click
    if (controller->button_mouse_r) {
        if (block_pointing > 0 && block_put_cooldown == 0.f) {
            if (!state->player.throw_mode) {
                state->chunk_map.change_block_at_block_pos(b_pos_pointing_front, state->player.selected_block + 1);
                block_put_cooldown = 0.3f;

                Sound::play(Sound::place_sound);
            } else {
                if (state->player.hand_entity == nullptr) {
                    block_put_cooldown = 0.3f;
                    state->chunk_map.change_block_at_block_pos(b_pos_pointing, 0);
                    const Vector3f break_pos = block_pos_to_pos(b_pos_pointing);
                    const Vector3f color = block_color_map[block_pointing];
                    state->player.hand_entity = spawn_block_entity(state, break_pos, color);
                    state->player.hand_entity->bound_offset = break_pos - state->player.pos;
                }
            }
        }
    } else {
        block_put_cooldown = 0;
        if (last_controller->button_mouse_r) {
            if (state->player.hand_entity != nullptr) {
                constexpr float32 MAX_SPEED = 100.0f;
                constexpr float32 MAX_PULL = 2.0f;

                state->player.hand_entity->speed = state->player.direction * (MAX_SPEED * lin2exp(MIN(state->player.hand_entity->age, MAX_PULL)));
                state->player.hand_entity->bound = false;
                state->player.hand_entity->age = 0.f;
                state->player.hand_entity = nullptr;
            }
        }
    }

    // Mouse middle click
    if (controller->button_mouse_m) {
        if (block_pointing > 0) {
            state->player.selected_block = block_pointing - 1;
        }
    }

    // Block manipulation cool downs
    block_break_cooldown = MAX(block_break_cooldown - time_delta, 0);
    block_put_cooldown = MAX(block_put_cooldown - time_delta, 0);

    // Add walking acceleration
    walk_direction.normalize();
    acceleration += walk_direction * Config::Player::ACCELERATION;

    // Adjusting camera angles
    state->player.pitch += (controller->dir_down - controller->dir_up) * time_delta * 75.f;
    state->player.yaw += (controller->dir_right - controller->dir_left) * time_delta * 75.f;

    state->player.pitch -= ((float32)(controller->mouse_move_y)) * 0.1f;
    state->player.yaw += ((float32)(controller->mouse_move_x)) * 0.1f;
    controller->mouse_move_x = 0;
    controller->mouse_move_y = 0;

    // Clamping pitch
    if (state->player.pitch > 89.0f) {
        state->player.pitch = 89.0f;
    } else if (state->player.pitch < -89.0f) {
        state->player.pitch = -89.0f;
    }

    state->player.direction.x = cos(glm::radians(state->player.pitch)) * cos(glm::radians(state->player.yaw));
    state->player.direction.y = sin(glm::radians(state->player.pitch));
    state->player.direction.z = cos(glm::radians(state->player.pitch)) * sin(glm::radians(state->player.yaw));
    state->player.direction.normalize();

    state->player.speed += (acceleration * time_delta);

    constexpr float32 FC = Config::Physics::FRICTION_CONSTANT;
    constexpr float32 FM = Config::Physics::FRICTION_MULTIPLIER;
    Vector3f friction;
    friction.x = state->player.speed.x * FM + (state->player.speed.x > 0 ? FC : -FC);
    friction.y = (state->player.speed.y * (FM * 0.25f) + (state->player.speed.y > 0 ? FC * 0.1f : -FC * 0.1f));
    friction.z = state->player.speed.z * FM + (state->player.speed.z > 0 ? FC : -FC);

    if (fabsf(friction.x) > fabsf(state->player.speed.x) / time_delta) {
        friction.x = state->player.speed.x / time_delta;
    }
    if (fabsf(friction.y) > fabsf(state->player.speed.y) / time_delta) {
        friction.y = state->player.speed.y / time_delta;
    }
    if (fabsf(friction.z) > fabsf(state->player.speed.z) / time_delta) {
        friction.z = state->player.speed.z / time_delta;
    }

    state->player.speed -= friction * time_delta;

    if (!state->player.on_ground && state->frame_count > 2) {
        // gravity
        state->player.speed.y -= 50.f * time_delta;
    }

    Vector3f new_pos = state->player.pos + state->player.speed * time_delta;

    // Player collision
    while (handle_collision(state->player, state->chunk_map, new_pos)) {
    }
    state->player.pos = new_pos;

    if (state->player.on_ground) {
        constexpr float32 WIDTH = Config::Player::WIDTH;
        constexpr float32 HEIGHT = Config::Player::HEIGHT;
        const Vector3f offsets[] = {{0, -HEIGHT - 0.5F, 0},
                                    {+WIDTH * 0.5F, -HEIGHT - 0.5F, +WIDTH * 0.5F},
                                    {-WIDTH * 0.5F, -HEIGHT - 0.5F, +WIDTH * 0.5F},
                                    {+WIDTH * 0.5F, -HEIGHT - 0.5F, -WIDTH * 0.5F},
                                    {-WIDTH * 0.5F, -HEIGHT - 0.5F, -WIDTH * 0.5F}};
        bool solid_ground = false;
        for (const Vector3f &offset : offsets) {
            if (state->chunk_map.get_block_at_pos(state->player.pos + offset) > 0) {
                solid_ground = true;
                break;
            }
        }
        state->player.on_ground = solid_ground;
    }

    // Player selected block
    state->player.selected_block = mod((state->player.selected_block + controller->mouse_wheel), 8);
    controller->mouse_wheel = 0;
}

void update(GameState *state, float32 time_delta, ControllerInput *controller, const ControllerInput *last_controller, BlockPos &b_pos_pointing,
            uint8 &block_pointing) {
    if (controller->button_f5 && !last_controller->button_f5) {
        state->chunk_map.update_all_chunks(state->player.pos);
    }

    update_player(state, time_delta, controller, last_controller, b_pos_pointing, block_pointing);
    update_sun(state, time_delta, controller, last_controller);
    update_particles(state, time_delta);
    update_entities(state, time_delta);
    update_fov(state, time_delta, controller);
}
}  // namespace Play
