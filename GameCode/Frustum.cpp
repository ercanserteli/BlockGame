#include "Frustum.h"

#include "AABB.h"

using namespace glm;

// compute frustum planes from view and projection matrices
Frustum::Frustum(const mat4 &view_matrix, const mat4 &projection_matrix) {
    const mat4 &v = view_matrix;
    const mat4 &p = projection_matrix;

    mat4 clip_matrix;

    clip_matrix[0][0] = v[0][0] * p[0][0] + v[0][1] * p[1][0] + v[0][2] * p[2][0] + v[0][3] * p[3][0];
    clip_matrix[1][0] = v[0][0] * p[0][1] + v[0][1] * p[1][1] + v[0][2] * p[2][1] + v[0][3] * p[3][1];
    clip_matrix[2][0] = v[0][0] * p[0][2] + v[0][1] * p[1][2] + v[0][2] * p[2][2] + v[0][3] * p[3][2];
    clip_matrix[3][0] = v[0][0] * p[0][3] + v[0][1] * p[1][3] + v[0][2] * p[2][3] + v[0][3] * p[3][3];
    clip_matrix[0][1] = v[1][0] * p[0][0] + v[1][1] * p[1][0] + v[1][2] * p[2][0] + v[1][3] * p[3][0];
    clip_matrix[1][1] = v[1][0] * p[0][1] + v[1][1] * p[1][1] + v[1][2] * p[2][1] + v[1][3] * p[3][1];
    clip_matrix[2][1] = v[1][0] * p[0][2] + v[1][1] * p[1][2] + v[1][2] * p[2][2] + v[1][3] * p[3][2];
    clip_matrix[3][1] = v[1][0] * p[0][3] + v[1][1] * p[1][3] + v[1][2] * p[2][3] + v[1][3] * p[3][3];
    clip_matrix[0][2] = v[2][0] * p[0][0] + v[2][1] * p[1][0] + v[2][2] * p[2][0] + v[2][3] * p[3][0];
    clip_matrix[1][2] = v[2][0] * p[0][1] + v[2][1] * p[1][1] + v[2][2] * p[2][1] + v[2][3] * p[3][1];
    clip_matrix[2][2] = v[2][0] * p[0][2] + v[2][1] * p[1][2] + v[2][2] * p[2][2] + v[2][3] * p[3][2];
    clip_matrix[3][2] = v[2][0] * p[0][3] + v[2][1] * p[1][3] + v[2][2] * p[2][3] + v[2][3] * p[3][3];
    clip_matrix[0][3] = v[3][0] * p[0][0] + v[3][1] * p[1][0] + v[3][2] * p[2][0] + v[3][3] * p[3][0];
    clip_matrix[1][3] = v[3][0] * p[0][1] + v[3][1] * p[1][1] + v[3][2] * p[2][1] + v[3][3] * p[3][1];
    clip_matrix[2][3] = v[3][0] * p[0][2] + v[3][1] * p[1][2] + v[3][2] * p[2][2] + v[3][3] * p[3][2];
    clip_matrix[3][3] = v[3][0] * p[0][3] + v[3][1] * p[1][3] + v[3][2] * p[2][3] + v[3][3] * p[3][3];

    m_planes[PLANE_RIGHT].x = clip_matrix[3][0] - clip_matrix[0][0];
    m_planes[PLANE_RIGHT].y = clip_matrix[3][1] - clip_matrix[0][1];
    m_planes[PLANE_RIGHT].z = clip_matrix[3][2] - clip_matrix[0][2];
    m_planes[PLANE_RIGHT].w = clip_matrix[3][3] - clip_matrix[0][3];

    m_planes[PLANE_LEFT].x = clip_matrix[3][0] + clip_matrix[0][0];
    m_planes[PLANE_LEFT].y = clip_matrix[3][1] + clip_matrix[0][1];
    m_planes[PLANE_LEFT].z = clip_matrix[3][2] + clip_matrix[0][2];
    m_planes[PLANE_LEFT].w = clip_matrix[3][3] + clip_matrix[0][3];

    m_planes[PLANE_BOTTOM].x = clip_matrix[3][0] + clip_matrix[1][0];
    m_planes[PLANE_BOTTOM].y = clip_matrix[3][1] + clip_matrix[1][1];
    m_planes[PLANE_BOTTOM].z = clip_matrix[3][2] + clip_matrix[1][2];
    m_planes[PLANE_BOTTOM].w = clip_matrix[3][3] + clip_matrix[1][3];

    m_planes[PLANE_TOP].x = clip_matrix[3][0] - clip_matrix[1][0];
    m_planes[PLANE_TOP].y = clip_matrix[3][1] - clip_matrix[1][1];
    m_planes[PLANE_TOP].z = clip_matrix[3][2] - clip_matrix[1][2];
    m_planes[PLANE_TOP].w = clip_matrix[3][3] - clip_matrix[1][3];

    m_planes[PLANE_BACK].x = clip_matrix[3][0] - clip_matrix[2][0];
    m_planes[PLANE_BACK].y = clip_matrix[3][1] - clip_matrix[2][1];
    m_planes[PLANE_BACK].z = clip_matrix[3][2] - clip_matrix[2][2];
    m_planes[PLANE_BACK].w = clip_matrix[3][3] - clip_matrix[2][3];

    m_planes[PLANE_FRONT].x = clip_matrix[3][0] + clip_matrix[2][0];
    m_planes[PLANE_FRONT].y = clip_matrix[3][1] + clip_matrix[2][1];
    m_planes[PLANE_FRONT].z = clip_matrix[3][2] + clip_matrix[2][2];
    m_planes[PLANE_FRONT].w = clip_matrix[3][3] + clip_matrix[2][3];

    for (auto &m_plane : m_planes) {
        m_plane = glm::normalize(m_plane);
    }
}

// check whether an AABB intersects the frustum
Frustum::TestResult Frustum::test_intersection(const AABB &box) const {
    TestResult result = TEST_INSIDE;

    for (auto &m_plane : m_planes) {
        const float32 pos = m_plane.w;
        const vec3 normal = vec3(m_plane);

        if (glm::dot(normal, box.get_positive_vertex(normal).as_vec3()) + pos < 0.0f) {
            return TEST_OUTSIDE;
        }

        if (glm::dot(normal, box.get_negative_vertex(normal).as_vec3()) + pos < 0.0f) {
            result = TEST_INTERSECT;
        }
    }

    return result;
}
