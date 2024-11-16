#include "AABB.h"

bool aabb_check(const AABB &b1, const AABB &b2) {
    return !(b1.max.x < b2.min.x || b1.min.x > b2.max.x || b1.max.y < b2.min.y || b1.min.y > b2.max.y || b1.max.z < b2.min.z || b1.min.z > b2.max.z);
}

AABB get_swept_broadphase_aabb(const AABB &b, const Vector3f &vel) {
    AABB res;
    res.min.x = vel.x > 0 ? b.min.x : b.min.x + vel.x;
    res.min.y = vel.y > 0 ? b.min.y : b.min.y + vel.y;
    res.min.z = vel.z > 0 ? b.min.z : b.min.z + vel.z;
    res.max.x = vel.x > 0 ? vel.x + b.max.x : b.max.x - vel.x;
    res.max.y = vel.y > 0 ? vel.y + b.max.y : b.max.y - vel.y;
    res.max.z = vel.z > 0 ? vel.z + b.max.z : b.max.z - vel.z;

    return res;
}

float32 swept_aabb_check(const AABB &b1, const AABB &b2, const Vector3f &speed, Vector3f &normal) {
    float32 x_inv_entry, y_inv_entry, z_inv_entry;
    float32 x_inv_exit, y_inv_exit, z_inv_exit;

    // find the distance between the objects on the near and far sides for both x
    // and y
    if (speed.x > 0.0f) {
        x_inv_entry = b2.min.x - b1.max.x;
        x_inv_exit = b2.max.x - b1.min.x;
    } else {
        x_inv_entry = b2.max.x - b1.min.x;
        x_inv_exit = b2.min.x - b1.max.x;
    }

    if (speed.y > 0.0f) {
        y_inv_entry = b2.min.y - b1.max.y;
        y_inv_exit = b2.max.y - b1.min.y;
    } else {
        y_inv_entry = b2.max.y - b1.min.y;
        y_inv_exit = b2.min.y - b1.max.y;
    }

    if (speed.z > 0.0f) {
        z_inv_entry = b2.min.z - b1.max.z;
        z_inv_exit = b2.max.z - b1.min.z;
    } else {
        z_inv_entry = b2.max.z - b1.min.z;
        z_inv_exit = b2.min.z - b1.max.z;
    }

    // find time of collision and time of leaving for each axis (if statement is
    // to prevent divide by zero)
    float32 x_entry, y_entry, z_entry;
    float32 x_exit, y_exit, z_exit;

    if (speed.x == 0.0f) {
        x_entry = -INFINITY;
        x_exit = INFINITY;
    } else {
        x_entry = x_inv_entry / speed.x;
        x_exit = x_inv_exit / speed.x;
    }

    if (speed.y == 0.0f) {
        y_entry = -INFINITY;
        y_exit = INFINITY;
    } else {
        y_entry = y_inv_entry / speed.y;
        y_exit = y_inv_exit / speed.y;
    }

    if (speed.z == 0.0f) {
        z_entry = -INFINITY;
        z_exit = INFINITY;
    } else {
        z_entry = z_inv_entry / speed.z;
        z_exit = z_inv_exit / speed.z;
    }

    // find the earliest/latest times of collision
    const float32 entry_time = MAX(x_entry, MAX(y_entry, z_entry));
    const float32 exit_time = MIN(x_exit, MIN(y_exit, z_exit));

    // if there was no collision
    if (entry_time > exit_time || x_entry < 0.0f && y_entry < 0.0f && z_entry < 0.0f || x_entry > 1.0f || y_entry > 1.0f || z_entry > 1.0f) {
        normal = {};
        return 1.0f;
    } else {  // if there was a collision
        // calculate normal of collided surface
        if (x_entry > y_entry && x_entry > z_entry) {
            if (x_inv_entry < 0.0f) {
                normal = {1.0f, 0.0f, 0.0f};
            } else {
                normal = {-1.0f, 0.0f, 0.0f};
            }
        } else if (y_entry > x_entry && y_entry > z_entry) {
            if (y_inv_entry < 0.0f) {
                normal = {0.0f, 1.0f, 0.0f};
            } else {
                normal = {0.0f, -1.0f, 0.0f};
            }
        } else {
            if (z_inv_entry < 0.0f) {
                normal = {0.0f, 0.0f, 1.0f};
            } else {
                normal = {0.0f, 0.0f, -1.0f};
            }
        }

        // return the time of collision
        return entry_time;
    }
}