#pragma once
#include <glm/matrix.hpp>

#include "Config.h"
#include "Definitions.h"

namespace DebugVisuals {
extern glm::vec4 debug_frustum_corners_w[Config::Graphics::SHADOW_MAP_CASCADE_COUNT][8];
extern bool frustums_initialized;

void initialize();
void draw_debug_shadow_maps(int32 window_width, int32 window_height, const uint32 *depth_maps);
void draw_frustum_wire_frames(const glm::mat4 &view, const glm::mat4 &projection);
void calculate_light_frustum_corners(uint32 cascade, const glm::mat4 &sun_projection, const glm::mat4 &sun_view);
void draw_light_frustum_wire_frames(const glm::mat4 &view, const glm::mat4 &projection);
}
