#pragma once
#include <glad/glad.h>

#include "GameBase.h"

struct Shader {
    void initialize(const char *vertex_source, const char *fragment_source);
    void use() const;
    int32 get_uniform_loc(const char *uniform_name) const { return glGetUniformLocation(shader_program, uniform_name); }

    uint32 shader_program;
};

struct MainShader : Shader {
    void initialize(const char *vertex_source, const char *fragment_source);
    void init_uniform_locations();

    int32 model_loc;
    int32 view_loc;
    int32 projection_loc;
    int32 ambient_base_loc;
    int32 object_color_loc;
    int32 sun_color_loc;
    int32 sun_pos_loc;
    int32 view_pos_loc;
    int32 sky_color_loc;
    int32 specular_strength_loc;
    int32 diffuse_strength_loc;
    int32 culling_distance_loc;
    int32 sun_space_matrix_loc;
    int32 cascade_ends_loc;
    int32 shadow_map_loc;
};

struct StarShader : Shader {
    void initialize(const char *vertex_source, const char *fragment_source);
    void init_uniform_locations();

    int32 model_loc;
    int32 view_loc;
    int32 projection_loc;
    int32 color_loc;
    int32 sky_color_loc;
    int32 star_visibility_loc;
};

struct ShadowMapShader : Shader {
    void initialize(const char *vertex_source, const char *fragment_source);
    void init_uniform_locations();

    int32 model_loc;
    int32 sun_space_matrix_loc;
};

struct DebugDepthMapShader : Shader {
    void initialize(const char *vertex_source, const char *fragment_source);
    void init_uniform_locations();

    int32 depth_map_loc;
    int32 cascade_level_loc;
};
