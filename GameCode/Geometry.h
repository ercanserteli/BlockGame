#pragma once
#include <glm/glm.hpp>

#include "Definitions.h"

struct Vector2f {
    float32 x;
    float32 y;

    float32 get_magnitude() const {
        const float32 magnitude = sqrtf(x * x + y * y);
        return magnitude;
    }

    void normalize() {
        const float32 magnitude = sqrtf(x * x + y * y);
        if (magnitude > 0) {
            x = x / magnitude;
            y = y / magnitude;
        }
    }

    Vector2f get_normalized() const {
        Vector2f normalized = *this;
        normalized.normalize();
        return normalized;
    }

    // With vectors

    Vector2f &operator+=(const Vector2f b) {
        this->x += b.x;
        this->y += b.y;
        return *this;
    }
    Vector2f &operator-=(const Vector2f b) {
        this->x -= b.x;
        this->y -= b.y;
        return *this;
    }
    // Hadamard product
    Vector2f &operator*=(const Vector2f b) {
        this->x *= b.x;
        this->y *= b.y;
        return *this;
    }
    Vector2f &operator/=(const Vector2f b) {
        this->x /= b.x;
        this->y /= b.y;
        return *this;
    }

    // Scaling
    Vector2f &operator*=(const float32 b) {
        this->x = this->x * b;
        this->y = this->y * b;
        return *this;
    }
    Vector2f &operator/=(const float32 b) {
        this->x = this->x / b;
        this->y = this->y / b;
        return *this;
    }
};

// With vectors

inline Vector2f operator+(const Vector2f a, const Vector2f b) { return {a.x + b.x, a.y + b.y}; }
inline Vector2f operator-(const Vector2f a, const Vector2f b) { return {a.x - b.x, a.y - b.y}; }
// Hadamard product
inline Vector2f operator*(const Vector2f a, const Vector2f b) { return {a.x * b.x, a.y * b.y}; }
inline Vector2f operator/(const Vector2f a, const Vector2f b) { return {a.x / b.x, a.y / b.y}; }
// Dot product
inline float32 dot(const Vector2f a, const Vector2f b) { return a.x * b.x + a.y * b.y; }

// Scaling
inline Vector2f operator*(const Vector2f a, const float32 b) { return {a.x * b, a.y * b}; }
inline Vector2f operator/(const Vector2f a, const float32 b) { return {a.x / b, a.y / b}; }

struct Vector3f {
    float32 x;
    float32 y;
    float32 z;

    float32 get_magnitude() const {
        const float32 magnitude = sqrtf(x * x + y * y + z * z);
        return magnitude;
    }

    float32 get_sqr_magnitude() const { return x * x + y * y + z * z; }

    void normalize() {
        const float32 magnitude = sqrtf(x * x + y * y + z * z);
        if (magnitude > 0) {
            x = x / magnitude;
            y = y / magnitude;
            z = z / magnitude;
        }
    }

    Vector3f get_normalized() const {
        Vector3f normalized = *this;
        normalized.normalize();
        return normalized;
    }

    Vector3f() : x(0), y(0), z(0) {}
    Vector3f(const float32 x, const float32 y, const float32 z) : x(x), y(y), z(z) {}
    Vector3f(const glm::vec3 v) : x(v.x), y(v.y), z(v.z) {}

    glm::vec3& as_vec3() { return *(glm::vec3*)this; }
    const glm::vec3 &as_vec3() const { return *(const glm::vec3 *)this; }

    // With vector3fs

    Vector3f &operator+=(const Vector3f b) {
        this->x += b.x;
        this->y += b.y;
        this->z += b.z;
        return *this;
    }
    Vector3f &operator-=(const Vector3f b) {
        this->x -= b.x;
        this->y -= b.y;
        this->z -= b.z;
        return *this;
    }
    // Hadamard product
    Vector3f &operator*=(const Vector3f b) {
        this->x *= b.x;
        this->y *= b.y;
        this->z *= b.z;
        return *this;
    }
    Vector3f &operator/=(const Vector3f b) {
        this->x /= b.x;
        this->y /= b.y;
        this->z /= b.z;
        return *this;
    }

    // With vector2fs

    Vector3f &operator+=(const Vector2f b) {
        this->x += b.x;
        this->y += b.y;
        return *this;
    }
    Vector3f &operator-=(const Vector2f b) {
        this->x -= b.x;
        this->y -= b.y;
        return *this;
    }
    // Hadamard product
    Vector3f &operator*=(const Vector2f b) {
        this->x *= b.x;
        this->y *= b.y;
        return *this;
    }
    Vector3f &operator/=(const Vector2f b) {
        this->x /= b.x;
        this->y /= b.y;
        return *this;
    }

    // Scaling
    Vector3f &operator*=(const float32 b) {
        this->x = this->x * b;
        this->y = this->y * b;
        this->z = this->z * b;
        return *this;
    }
    Vector3f &operator/=(const float32 b) {
        this->x = this->x / b;
        this->y = this->y / b;
        this->z = this->z / b;
        return *this;
    }

    float32 &operator[](const int32 i) {
        switch (i) {
            default:
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
        }
    }
};

// With vector3fs

inline Vector3f operator+(const Vector3f a, const Vector3f b) {
    Vector3f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}
inline Vector3f operator-(const Vector3f a, const Vector3f b) {
    Vector3f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}
// Hadamard product
inline Vector3f operator*(const Vector3f a, const Vector3f b) {
    Vector3f result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}
inline Vector3f operator/(const Vector3f a, const Vector3f b) {
    Vector3f result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    return result;
}

// With vector2fs

inline Vector3f operator+(const Vector3f a, const Vector2f b) {
    Vector3f result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}
inline Vector3f operator-(const Vector3f a, const Vector2f b) {
    Vector3f result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}
// Hadamard product
inline Vector3f operator*(const Vector3f a, const Vector2f b) {
    Vector3f result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}
inline Vector3f operator/(const Vector3f a, const Vector2f b) {
    Vector3f result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    return result;
}
// Dot product
inline float32 dot(const Vector3f a, const Vector3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
// Cross product
inline Vector3f cross(const Vector3f a, const Vector3f b) {
    Vector3f res;
    res.x = a.y * b.z - b.y * a.z;
    res.y = b.x * a.z - a.x * b.z;
    res.z = a.x * b.y - b.x * a.y;
    return res;
}

// Scaling
inline Vector3f operator*(const Vector3f a, const float32 b) {
    Vector3f result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    return result;
}
inline Vector3f operator/(const Vector3f a, const float32 b) {
    Vector3f result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    return result;
}

inline float32 sqr_dist(const Vector3f a, const Vector3f b) { return (a - b).get_sqr_magnitude(); }

inline float32 dist(const Vector3f a, const Vector3f b) { return (a - b).get_magnitude(); }

inline bool test_ray_aabb_intersect(Vector3f ray_dir, const Vector3f ray_org, const Vector3f bottom_left, const Vector3f top_right, float32 &t) {
    // Protection against division by 0
    if (ray_dir.x == 0.f) {
        ray_dir.x = FLT_MIN;
    }
    if (ray_dir.y == 0.f) {
        ray_dir.y = FLT_MIN;
    }
    if (ray_dir.z == 0.f) {
        ray_dir.z = FLT_MIN;
    }

    Vector3f dirfrac;
    dirfrac.x = 1.0f / ray_dir.x;
    dirfrac.y = 1.0f / ray_dir.y;
    dirfrac.z = 1.0f / ray_dir.z;

    const float32 t1 = (bottom_left.x - ray_org.x) * dirfrac.x;
    const float32 t2 = (top_right.x - ray_org.x) * dirfrac.x;
    const float32 t3 = (bottom_left.y - ray_org.y) * dirfrac.y;
    const float32 t4 = (top_right.y - ray_org.y) * dirfrac.y;
    const float32 t5 = (bottom_left.z - ray_org.z) * dirfrac.z;
    const float32 t6 = (top_right.z - ray_org.z) * dirfrac.z;

    const float32 tmin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));
    const float32 tmax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));

    if (tmax < 0) {
        // AABB is behind ray
        t = tmax;
        return false;
    }

    if (tmin > tmax) {
        // No intersection
        t = tmax;
        return false;
    }

    // Intersection
    t = tmin;
    return true;
}
