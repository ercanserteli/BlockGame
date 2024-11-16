#include "ShadowDebugVisuals.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

namespace DebugVisuals {

glm::vec4 debug_frustum_corners_w[Config::Graphics::SHADOW_MAP_CASCADE_COUNT][8];
bool frustums_initialized = false;

static uint32 vao_quad, vbo_quad;
static uint32 vao_player_frustum, vbo_player_frustum, ebo_player_frustum;
static uint32 vao_light_frustum, vbo_light_frustum, ebo_light_frustum;
static DebugDepthMapShader debug_depth_shader;
static Shader line_shader;
static glm::vec4 light_frustum_corners[Config::Graphics::SHADOW_MAP_CASCADE_COUNT][8];

void initialize_frustum(uint32 &vao, uint32 &vbo, uint32 &ebo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 8, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    constexpr uint32 FRUSTUM_INDICES[] = {// Near face
                                          0, 1, 1, 3, 3, 2, 2, 0,
                                          // Far face
                                          4, 5, 5, 7, 7, 6, 6, 4,
                                          // Connecting edges
                                          0, 4, 1, 5, 2, 6, 3, 7};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FRUSTUM_INDICES), FRUSTUM_INDICES, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void initialize_depth_quad() {
    constexpr float32 QUAD_VERTICES[] = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao_quad);
    glGenBuffers(1, &vbo_quad);
    glBindVertexArray(vao_quad);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), &QUAD_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float32), (void *)nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float32), (void *)(3 * sizeof(float32)));
}

void initialize() {
#include "shaders/frag_csm_debug_glsl.h"
#include "shaders/frag_line_glsl.h"
#include "shaders/vertex_line_glsl.h"
#include "shaders/vertex_csm_debug_glsl.h"
    debug_depth_shader.initialize(vertex_csm_debug_source, frag_csm_debug_source);
    line_shader.initialize(vertex_line_source, frag_line_source);
    initialize_depth_quad();
    initialize_frustum(vao_player_frustum, vbo_player_frustum, ebo_player_frustum);
    initialize_frustum(vao_light_frustum, vbo_light_frustum, ebo_light_frustum);
}

void draw_debug_shadow_maps(int32 window_width, int32 window_height, const uint32 *depth_maps) {
    // Save current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    debug_depth_shader.use();
    glBindVertexArray(vao_quad);

    const int debug_width = window_width / 4;
    const int debug_height = window_height / 4;

    // Render each cascade level
    for (int i = 0; i < 3; i++) {
        constexpr int PADDING = 10;
        glViewport(PADDING + i * (debug_width + PADDING), window_height - debug_height - PADDING, debug_width, debug_height);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_maps[i]);
        glUniform1i(debug_depth_shader.depth_map_loc, 0);
        glUniform1i(debug_depth_shader.cascade_level_loc, i);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Restore original viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void draw_frustum_wire_frames(const glm::mat4 &view, const glm::mat4 &projection) {
    line_shader.use();
    glUniformMatrix4fv(line_shader.get_uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(line_shader.get_uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao_player_frustum);
    glLineWidth(2.0f);

    for (int i = 0; i < Config::Graphics::SHADOW_MAP_CASCADE_COUNT; i++) {
        // Update VBO data with frustum corners
        glm::vec3 frustum_vertices[8];
        for (int j = 0; j < 8; j++) {
            frustum_vertices[j] = glm::vec3(debug_frustum_corners_w[i][j]);  // Convert vec4 to vec3
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo_player_frustum);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(frustum_vertices), frustum_vertices);

        glm::vec3 color;
        if (i == 0)
            color = glm::vec3(1.0, 0.0, 0.0);
        else if (i == 1)
            color = glm::vec3(0.0, 1.0, 0.0);
        else if (i == 2)
            color = glm::vec3(0.0, 0.0, 1.0);

        glUniform3fv(line_shader.get_uniform_loc("color"), 1, glm::value_ptr(color));

        // Draw the frustum edges
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glLineWidth(1.0f);
}

void calculate_light_frustum_corners(const uint32 cascade, const glm::mat4 &sun_projection, const glm::mat4 &sun_view) {
    // Define in clip space
    const glm::vec4 corners[8] = {{-1, -1, -1, 1}, {1, -1, -1, 1}, {-1, 1, -1, 1}, {1, 1, -1, 1}, {-1, -1, 1, 1}, {1, -1, 1, 1}, {-1, 1, 1, 1}, {1, 1, 1, 1}};

    const glm::mat4 inv_sun_projection = glm::inverse(sun_projection);
    const glm::mat4 inv_sun_view = glm::inverse(sun_view);

    for (int i = 0; i < 8; i++) {
        // Clip -> View
        glm::vec4 view_space_corner = inv_sun_projection * corners[i];
        view_space_corner /= view_space_corner.w;

        // View -> World
        light_frustum_corners[cascade][i] = inv_sun_view * view_space_corner;
    }
}

void draw_light_frustum_wire_frames(const glm::mat4 &view, const glm::mat4 &projection) {
    line_shader.use();
    glUniformMatrix4fv(line_shader.get_uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(line_shader.get_uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao_light_frustum);
    glLineWidth(4.0f);

    for (int i = 0; i < Config::Graphics::SHADOW_MAP_CASCADE_COUNT; i++) {
        glm::vec3 frustum_vertices[8];
        for (int j = 0; j < 8; j++) {
            frustum_vertices[j] = glm::vec3(light_frustum_corners[i][j]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo_light_frustum);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(frustum_vertices), frustum_vertices);

        glm::vec3 color;
        if (i == 0)
            color = glm::vec3(1.0, 1.0, 0.0);
        else if (i == 1)
            color = glm::vec3(1.0, 0.8, 0.0);
        else if (i == 2)
            color = glm::vec3(1.0, 0.6, 0.0);

        glUniform3fv(line_shader.get_uniform_loc("color"), 1, glm::value_ptr(color));
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glLineWidth(1.0f);
}
}  // namespace DebugVisuals
