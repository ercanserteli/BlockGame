#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

struct AABB;

struct Frustum {
    enum TestResult { TEST_OUTSIDE, TEST_INTERSECT, TEST_INSIDE };
    enum Plane { PLANE_BACK, PLANE_FRONT, PLANE_RIGHT, PLANE_LEFT, PLANE_TOP, PLANE_BOTTOM };

    Frustum(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix);
    TestResult test_intersection(const AABB &box) const;
    
    glm::vec4 m_planes[6] = {};
};