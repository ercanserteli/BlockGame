#pragma once

#include "Definitions.h"

#include <stdlib.h>

inline float32 rand_float() { return ((float32)(rand() % 1024) / 1024.f); }

inline float32 lin2exp(float32 x, const bool inv = false) {
    if (!inv) {
        return pow(E32, (1 - 1 / (x * x + 0.000001f)));
    }
    x = 1 - x;
    return 1 - pow(E32, (1 - 1 / (x * x + 0.000001f)));
}

inline int32 mod(const int32 a, const int32 b) {
    int32 ret = a % b;
    if (ret < 0) ret += b;
    return ret;
}
