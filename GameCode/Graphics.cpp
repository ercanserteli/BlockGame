#include "Graphics.h"

#include <glad/glad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glm/gtc/type_ptr.hpp>

#include "Chunk.h"
#include "Geometry.h"
#include "Shader.h"
#include "ShadowDebugVisuals.h"

namespace Graphics {
static MainShader main_shader;
static StarShader star_shader;
static Shader gui_shader;
static ShadowMapShader shadow_depth_shader;

static uint32 vao_cube;
static uint32 vao_sun;
static uint32 vao_stars;
static uint32 vao_crosshair;
static uint32 depth_map_fbo;
static uint32 depth_maps[Config::Graphics::SHADOW_MAP_CASCADE_COUNT];

static ShadowMode shadow_mode = ShadowMode::SHADOW_MAP;

void initialize_cube_graphics() {
    constexpr uint32 CUBE_VERTEX_COUNT = 216;
    uint32 vbo_cube;
    glGenVertexArrays(1, &vao_cube);
    glGenBuffers(1, &vbo_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
    glBufferData(GL_ARRAY_BUFFER, CUBE_VERTEX_COUNT * sizeof(float32), cube_vertices_with_normal, GL_STATIC_DRAW);
    glBindVertexArray(vao_cube);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void *)nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void *)(3 * sizeof(float32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // vao sun uses the same vbo as cube
    glGenVertexArrays(1, &vao_sun);
    glBindVertexArray(vao_sun);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void *)nullptr);
    glEnableVertexAttribArray(0);
}

void initialize_star_graphics(const Vector3f *stars) {
    // stars that are just points in space
    uint32 vbo_stars;
    glGenVertexArrays(1, &vao_stars);
    glBindVertexArray(vao_stars);
    glGenBuffers(1, &vbo_stars);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_stars);
    glBufferData(GL_ARRAY_BUFFER, Config::Game::STAR_COUNT * sizeof(float32), stars, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (void *)nullptr);
    glEnableVertexAttribArray(0);
}

void initialize_hud_graphics() {
    // Crosshair that is displayed at the center of the screen
    const Vector3f color = {0.25f, 0.25f, 0.25f};
    constexpr float32 S = 0.003f;  // Short edge
    constexpr float32 L = 0.03f;   // Long edge
    const Vector3f crosshair[] = {{-S, -L, 0}, color, {+S, -L, 0}, color, {+S, +L, 0}, color, {-S, -L, 0}, color, {-S, +L, 0}, color, {+S, +L, 0}, color,
                                  {-L, -S, 0}, color, {-L, +S, 0}, color, {+L, +S, 0}, color, {-L, -S, 0}, color, {+L, -S, 0}, color, {+L, +S, 0}, color};
    uint32 vbo_crosshair;
    glGenVertexArrays(1, &vao_crosshair);
    glBindVertexArray(vao_crosshair);
    glGenBuffers(1, &vbo_crosshair);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_crosshair);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair), crosshair, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void *)nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void *)(3 * sizeof(float32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Setting the orthographic projection matrix for GUI shader
    SDL_DisplayMode display_mode;
    const int32 should_be_zero = SDL_GetCurrentDisplayMode(0, &display_mode);
    if (should_be_zero == 0) {
        const float32 w = display_mode.w;
        const float32 h = display_mode.h;
        glm::mat4 projection;
        if (w <= h)
            projection = glm::ortho(-1.0, 1.0, -1.0 * h / w, 1.0 * h / w, -1.0, 1.0);
        else
            projection = glm::ortho(-1.0 * w / h, 1.0 * w / h, -1.0, 1.0, -1.0, 1.0);
        gui_shader.use();
        glUniformMatrix4fv(gui_shader.get_uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }
}

void initialize_shadow_graphics() {
    constexpr float32 BORDER_COLOR[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glGenFramebuffers(1, &depth_map_fbo);

    glGenTextures(Config::Graphics::SHADOW_MAP_CASCADE_COUNT, depth_maps);
    for (const uint32 depth_map : depth_maps) {
        glBindTexture(GL_TEXTURE_2D, depth_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Config::Graphics::SHADOW_MAP_WIDTH, Config::Graphics::SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BORDER_COLOR);
    }

    // Checking status
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_maps[0], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LogError("Shadow FrameBuffer error, status: 0x%x\n", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initialize(const GameState *state) {
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LogError("Failed to initialize OpenGL context\n");
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  // for anti aliasing

    // Including shader sources
#include "shaders/frag_empty_glsl.h"
#include "shaders/frag_glsl.h"
#include "shaders/frag_gui_glsl.h"
#include "shaders/frag_star_glsl.h"
#include "shaders/vertex_glsl.h"
#include "shaders/vertex_gui_glsl.h"
#include "shaders/vertex_simple_depth_glsl.h"
#include "shaders/vertex_star_glsl.h"

    main_shader.initialize(vertex_source, frag_source);
    star_shader.initialize(vertex_star_source, frag_star_source);
    gui_shader.initialize(vertex_gui_source, frag_gui_source);
    shadow_depth_shader.initialize(vertex_simple_depth_source, frag_empty_source);
    DebugVisuals::initialize();
    initialize_cube_graphics();
    initialize_star_graphics(state->stars);
    initialize_hud_graphics();
    initialize_shadow_graphics();

    stbi_flip_vertically_on_write(true);
}

void calc_ortho_projs(const glm::mat4 &view_inverse, glm::mat4 *sun_views, const float32 aspect_ratio, const float32 fov, const float32 *cascade_ends,
                      glm::mat4 *sun_projs, GameState *state) {
    const float32 tan_half_vfov = tanf(glm::radians(fov / 2.0f));
    const float32 tan_half_hfov = tan_half_vfov * aspect_ratio;

    for (uint32 i = 0; i < Config::Graphics::SHADOW_MAP_CASCADE_COUNT; i++) {
        constexpr uint8 NUM_FRUSTUM_CORNERS = 8;
        const float32 xn = cascade_ends[i] * tan_half_hfov;
        const float32 xf = cascade_ends[i + 1] * tan_half_hfov;
        const float32 yn = cascade_ends[i] * tan_half_vfov;
        const float32 yf = cascade_ends[i + 1] * tan_half_vfov;

        glm::vec4 frustum_corners_w[NUM_FRUSTUM_CORNERS];
        const glm::vec4 frustum_corners_v[NUM_FRUSTUM_CORNERS] = {// near face
                                                                  glm::vec4(xn, yn, -cascade_ends[i], 1.0), glm::vec4(-xn, yn, -cascade_ends[i], 1.0),
                                                                  glm::vec4(xn, -yn, -cascade_ends[i], 1.0), glm::vec4(-xn, -yn, -cascade_ends[i], 1.0),

                                                                  // far face
                                                                  glm::vec4(xf, yf, -cascade_ends[i + 1], 1.0), glm::vec4(-xf, yf, -cascade_ends[i + 1], 1.0),
                                                                  glm::vec4(xf, -yf, -cascade_ends[i + 1], 1.0),
                                                                  glm::vec4(-xf, -yf, -cascade_ends[i + 1], 1.0)};

        float32 min_x = FLT_MAX;
        float32 max_x = -FLT_MAX;
        float32 min_y = FLT_MAX;
        float32 max_y = -FLT_MAX;
        float32 min_z = FLT_MAX;
        float32 max_z = -FLT_MAX;

        glm::vec3 frustum_center = {};
        for (uint32 j = 0; j < NUM_FRUSTUM_CORNERS; j++) {
            glm::vec4 corner_w = view_inverse * frustum_corners_v[j];  // Transform to world space
            frustum_corners_w[j] = corner_w;
            if (!DebugVisuals::frustums_initialized) DebugVisuals::debug_frustum_corners_w[i][j] = corner_w;
            frustum_center += glm::vec3(corner_w);
        }
        frustum_center /= 8;

        frustum_center = state->player.pos.as_vec3();
        sun_views[i] = glm::lookAt(frustum_center - state->sun.pos.as_vec3() * 100.f, frustum_center, glm::vec3(0, 1, 0));

        for (glm::vec4 &j : frustum_corners_w) {
            glm::vec4 corner_sv = sun_views[i] * j;  // Transform to light view space

            min_x = MIN(min_x, corner_sv.x);
            max_x = MAX(max_x, corner_sv.x);
            min_y = MIN(min_y, corner_sv.y);
            max_y = MAX(max_y, corner_sv.y);
            min_z = MIN(min_z, corner_sv.z);
            max_z = MAX(max_z, corner_sv.z);
        }

        // Texel snapping
        float32 texel_size = (max_x - min_x) / Config::Graphics::SHADOW_MAP_WIDTH;
        min_x = floor(min_x / texel_size) * texel_size;
        max_x = ceil(max_x / texel_size) * texel_size;
        texel_size = (max_y - min_y) / Config::Graphics::SHADOW_MAP_WIDTH;
        min_y = floor(min_y / texel_size) * texel_size;
        max_y = ceil(max_y / texel_size) * texel_size;
        texel_size = (max_z - min_z) / Config::Graphics::SHADOW_MAP_WIDTH;
        min_z = floor(min_z / texel_size) * texel_size;
        max_z = ceil(max_z / texel_size) * texel_size;

        sun_projs[i] = glm::ortho(min_x, max_x, min_y, max_y, -min_z + 100, -max_z);
        if (!DebugVisuals::frustums_initialized) DebugVisuals::calculate_light_frustum_corners(i, sun_projs[i], sun_views[i]);
    }
    DebugVisuals::frustums_initialized = true;
}

void take_screenshot(GameState *state, int32 screen_width, int32 screen_height) {
    const uint32 pixels_size = 3 * screen_height * screen_width;
    auto *pixels = (uint8 *)push_size(state->scratch_arena, pixels_size);
    glReadPixels(0, 0, screen_width, screen_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    char ss_filepath[Config::System::MAX_PATH_LEN];
    strcpy_s(ss_filepath, Config::System::MAX_PATH_LEN, state->save_path);
    strcat_s(ss_filepath, "screenshot.bmp");

    if (!stbi_write_bmp(ss_filepath, screen_width, screen_height, 3, pixels)) {
        LogError("Could not take screenshot");
    }

    state->scratch_arena.used -= pixels_size;
}

void draw_entities(GameState *state, const bool is_shadow) {
    if (state->entity_count > 0) {
        glBindVertexArray(vao_cube);
        for (uint32 i = 0; i < state->entity_count; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, state->entities[i].pos.as_vec3());
            model *= glm::mat4_cast(state->entities[i].rotation);
            if (!is_shadow) {
                glUniformMatrix4fv(main_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform3fv(main_shader.object_color_loc, 1, (float32 *)&state->entities[i].color);
            } else {
                glUniformMatrix4fv(shadow_depth_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
            }
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

void draw_chunks(GameState *state, const Frustum &player_frustum) {
    // Reset object color and ambient base strength
    glUniform3f(main_shader.object_color_loc, 0, 0, 0);
    glUniform1f(main_shader.ambient_base_loc, 0.25f);

    state->chunk_map.draw_chunks(main_shader.model_loc, player_frustum, state->player.pos);
}

void draw_gui() {
    gui_shader.use();
    glBindVertexArray(vao_crosshair);
    glDrawArrays(GL_TRIANGLES, 0, 12);
}

void draw_particles(const GameState *state) {
    if (state->particle_count > 0) {
        glBindVertexArray(vao_cube);
        for (uint32 i = 0; i < state->particle_count; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, state->particles[i].pos.as_vec3());
            model = glm::scale(model, {state->particles[i].scale, state->particles[i].scale, state->particles[i].scale});
            model *= glm::mat4_cast(state->particles[i].rotation);
            glUniformMatrix4fv(main_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(main_shader.object_color_loc, 1, (const float32 *)&state->particles[i].color);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

void draw_stars(const GameState *state, const glm::mat4 &view, const glm::mat4 &projection) {
    star_shader.use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, state->player.pos.as_vec3());
    glUniformMatrix4fv(star_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(star_shader.view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(star_shader.projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(star_shader.sky_color_loc, 1, (const float32 *)&state->sun.sky_color);
    glUniform1f(star_shader.star_visibility_loc, state->sun.star_visibility);

    glBindVertexArray(vao_stars);
    glUniform3f(star_shader.color_loc, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_POINTS, 0, Config::Game::STAR_COUNT);
}

void draw_sun(const GameState *state, const glm::vec3 &camera_up) {
    constexpr glm::vec3 ORIGIN = {};
    glm::mat4 star_view = glm::lookAt(ORIGIN, state->player.direction.as_vec3(), camera_up);
    glUniformMatrix4fv(star_shader.view_loc, 1, GL_FALSE, glm::value_ptr(star_view));
    glUniform1f(star_shader.star_visibility_loc, 1.0f);
    glBindVertexArray(vao_sun);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, state->sun.pos.as_vec3());
    model = model * glm::mat4_cast(state->sun.rot);
    model = glm::scale(model, {4, 4, 4});
    glUniformMatrix4fv(star_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(star_shader.color_loc, 1, (const float32 *)&state->sun.color);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void draw_block_selection_box(const GameState *state, const BlockPos &b_pos_pointing, const glm::mat4 &view) {
    const Vector3f pos_pointing = block_pos_to_pos(b_pos_pointing);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos_pointing.as_vec3());
    model = glm::scale(model, {1.01f, 1.01f, 1.01f});
    glUniformMatrix4fv(star_shader.view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(star_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void draw_selected_block(const GameState *state) {
    glBindVertexArray(vao_cube);
    Vector3f side_vector = cross({0, 1, 0}, state->player.direction);
    side_vector.normalize();
    Vector3f cube_pos;
    cube_pos += state->player.pos + state->player.direction * 0.3f + side_vector * (-0.15f);
    cube_pos.y -= 0.1f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cube_pos.as_vec3());
    model = glm::scale(model, {0.1, 0.1, 0.1});
    model = glm::rotate(model, 60 - glm::radians(state->player.yaw), glm::vec3(0, 1, 0));

    glUniformMatrix4fv(main_shader.model_loc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(main_shader.object_color_loc, 1, (float32 *)&block_color_map[state->player.selected_block + 1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void draw(GameState *state, const int32 screen_width, const int32 screen_height, SDL_Window *window, uint8 block_pointing, const BlockPos &b_pos_pointing,
          float32 time_delta) {
    static const glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    const Vector3f look_at_pos = state->player.pos + state->player.direction;
    glm::mat4 view = glm::lookAt(state->player.pos.as_vec3(), look_at_pos.as_vec3(), camera_up);
    glm::mat4 projection =
        glm::perspective(glm::radians(state->player.fov), (float32)screen_width / (float32)screen_height, 0.1f, Config::Graphics::CULLING_DISTANCE);
    const Frustum player_frustum(view, projection);

    constexpr float32 CASCADE_ENDS[] = {Config::Graphics::SHADOW_NEAR_PLANE, 100.0f, 400.0f, 1600.0f};
    glm::mat4 sun_space_matrices[Config::Graphics::SHADOW_MAP_CASCADE_COUNT];

    // Shadow depth maps rendering
    if (shadow_mode == ShadowMode::SHADOW_MAP) {
        glm::mat4 sun_projections[Config::Graphics::SHADOW_MAP_CASCADE_COUNT];
        glm::mat4 sun_views[Config::Graphics::SHADOW_MAP_CASCADE_COUNT];
        shadow_depth_shader.use();

        const glm::mat4 view_inverse = glm::inverse(view);
        calc_ortho_projs(view_inverse, sun_views, ((float32)screen_width) / ((float32)screen_height), state->player.fov, CASCADE_ENDS, sun_projections, state);

        glCullFace(GL_FRONT);
        glViewport(0, 0, Config::Graphics::SHADOW_MAP_WIDTH, Config::Graphics::SHADOW_MAP_HEIGHT);
        for (uint32 i = 0; i < Config::Graphics::SHADOW_MAP_CASCADE_COUNT; i++) {
            sun_space_matrices[i] = sun_projections[i] * sun_views[i];
            glUniformMatrix4fv(shadow_depth_shader.sun_space_matrix_loc, 1, GL_FALSE, glm::value_ptr(sun_space_matrices[i]));

            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_maps[i], 0);
            glClear(GL_DEPTH_BUFFER_BIT);

            Frustum sun_frustum(sun_views[i], sun_projections[i]);
            state->chunk_map.draw_chunks(shadow_depth_shader.model_loc, sun_frustum, state->player.pos);
            draw_entities(state, true);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Real rendering
    glCullFace(GL_BACK);
    glViewport(0, 0, screen_width, screen_height);
    glClearColor(state->sun.sky_color.x, state->sun.sky_color.y, state->sun.sky_color.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float32 absolute_time = 0;
    absolute_time += time_delta;
    
    draw_stars(state, view, projection);
    draw_sun(state, camera_up);

    if (block_pointing > 0) {
        draw_block_selection_box(state, b_pos_pointing, view);
    }

    main_shader.use();
    glUniformMatrix4fv(main_shader.view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(main_shader.projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(main_shader.sun_color_loc, 1, (float32 *)&state->sun.color);
    glUniform3fv(main_shader.sun_pos_loc, 1, (float32 *)&state->sun.pos);
    glUniform3fv(main_shader.view_pos_loc, 1, (float32 *)&state->player.pos);
    glUniform3fv(main_shader.sky_color_loc, 1, (float32 *)&state->sun.sky_color);
    glUniform1f(main_shader.ambient_base_loc, 0.4f);
    glUniform1f(main_shader.specular_strength_loc, state->sun.specular_strength);
    glUniform1f(main_shader.diffuse_strength_loc, state->sun.diffuse_strength);
    glUniform1f(main_shader.culling_distance_loc, Config::World::DRAW_RADIUS * 32);

    if (shadow_mode == ShadowMode::SHADOW_MAP) {
        glUniformMatrix4fv(main_shader.sun_space_matrix_loc, Config::Graphics::SHADOW_MAP_CASCADE_COUNT, GL_FALSE, glm::value_ptr(sun_space_matrices[0]));
        glUniform1fv(main_shader.cascade_ends_loc, Config::Graphics::SHADOW_MAP_CASCADE_COUNT, CASCADE_ENDS + 1);
        constexpr int32 DEPTH_TEXTURE_IDS[] = {0, 1, 2};
        glUniform1iv(main_shader.shadow_map_loc, Config::Graphics::SHADOW_MAP_CASCADE_COUNT, DEPTH_TEXTURE_IDS);

        for (uint32 i = 0; i < Config::Graphics::SHADOW_MAP_CASCADE_COUNT; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, depth_maps[i]);
        }
    }

    if (!state->player.throw_mode) {
        draw_selected_block(state);
    }
    draw_particles(state);
    draw_entities(state, false);
    draw_chunks(state, player_frustum);
    draw_gui();

    // Shadow map debug visuals
    if (state->debug_visuals_enabled && shadow_mode == ShadowMode::SHADOW_MAP) {
        DebugVisuals::draw_debug_shadow_maps(screen_width, screen_height, depth_maps);
        DebugVisuals::draw_frustum_wire_frames(view, projection);
        DebugVisuals::draw_light_frustum_wire_frames(view, projection);
    }

    SDL_GL_SwapWindow(window);
}
}  // namespace Graphics
