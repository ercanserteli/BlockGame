#include "Shader.h"

#include "glad/glad.h"

void Shader::initialize(const char *vertex_source, const char *fragment_source) {
    if (!vertex_source || !fragment_source) {
        LogError("Could not read shader source file(s)!\n");
        return;
    }

    const uint32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar *const *)&vertex_source, nullptr);
    glCompileShader(vertex_shader);
    int32 success;
    char info_log[512];

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        LogError("ERROR::SHADER::VERTEX::COMPILATION_FAILED: \n%s\n", info_log);
        return;
    }

    const uint32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar *const *)&fragment_source, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        LogError("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: \n%s\n", info_log);
        return;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        LogError("ERROR::SHADER::PROGRAM::LINKING_FAILED: \n%s\n", info_log);
        return;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::initialize(const char *vertex_source, const char *geometry_source, const char *fragment_source) {
    if (!vertex_source || !geometry_source || !fragment_source) {
        LogError("Could not read shader source file(s)!\n");
        return;
    }

    const uint32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar *const *)&vertex_source, nullptr);
    glCompileShader(vertex_shader);
    int32 success;
    char info_log[512];

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        LogError("ERROR::SHADER::VERTEX::COMPILATION_FAILED: \n%s\n", info_log);
        return;
    }

    const uint32 geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry_shader, 1, (const GLchar *const *)&geometry_source, nullptr);
    glCompileShader(geometry_shader);
    glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(geometry_shader, 512, nullptr, info_log);
        LogError("ERROR::SHADER::GEOMETRY::COMPILATION_FAILED: \n%s\n", info_log);
        return;
    }

    const uint32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar *const *)&fragment_source, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        LogError("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: \n%s\n", info_log);
        return;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, geometry_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        LogError("ERROR::SHADER::PROGRAM::LINKING_FAILED: \n%s\n", info_log);
        return;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);
}

void Shader::use() const { glUseProgram(shader_program); }

void MainShader::initialize(const char *vertex_source, const char *fragment_source) {
    Shader::initialize(vertex_source, fragment_source);
    init_uniform_locations();
}

void MainShader::init_uniform_locations() {
    model_loc = get_uniform_loc("model");
    view_loc = get_uniform_loc("view");
    projection_loc = get_uniform_loc("projection");
    ambient_base_loc = get_uniform_loc("ambientBase");
    object_color_loc = get_uniform_loc("objectColor");
    sun_color_loc = get_uniform_loc("sunColor");
    sun_pos_loc = get_uniform_loc("sunPos");
    view_pos_loc = get_uniform_loc("viewPos");
    sky_color_loc = get_uniform_loc("skyColor");
    specular_strength_loc = get_uniform_loc("specularStrength");
    diffuse_strength_loc = get_uniform_loc("diffuseStrength");
    culling_distance_loc = get_uniform_loc("cullingDistance");
    sun_space_matrix_loc = get_uniform_loc("sunSpaceMatrix");
    cascade_ends_loc = get_uniform_loc("cascadeEnds");
    shadow_map_loc = get_uniform_loc("shadowMap");
    shadow_map_enabled_loc = get_uniform_loc("shadowMapEnabled");
}

void StarShader::initialize(const char *vertex_source, const char *fragment_source) {
    Shader::initialize(vertex_source, fragment_source);
    init_uniform_locations();
}

void StarShader::init_uniform_locations() {
    model_loc = get_uniform_loc("model");
    view_loc = get_uniform_loc("view");
    projection_loc = get_uniform_loc("projection");
    color_loc = get_uniform_loc("color");
    sky_color_loc = get_uniform_loc("skyColor");
    star_visibility_loc = get_uniform_loc("starVisibility");
}

void ShadowMapShader::initialize(const char *vertex_source, const char *fragment_source) {
    Shader::initialize(vertex_source, fragment_source);
    init_uniform_locations();
}

void ShadowMapShader::init_uniform_locations() {
    model_loc = get_uniform_loc("model");
    sun_space_matrix_loc = get_uniform_loc("sunSpaceMatrix");
}

void DebugDepthMapShader::initialize(const char *vertex_source, const char *fragment_source) {
    Shader::initialize(vertex_source, fragment_source);
    init_uniform_locations();
}

void DebugDepthMapShader::init_uniform_locations() {
    depth_map_loc = get_uniform_loc("depthMap");
    cascade_level_loc = get_uniform_loc("cascadeLevel");
}
