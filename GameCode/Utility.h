#pragma once

#include "Definitions.h"

#include <stdio.h>
#include <stdlib.h>

// Platform independent create dir
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

inline int32 create_dir(char *dir) {
#ifdef _WIN32
    return _mkdir(dir);
#else
    return mkdir(dir, 0777);
#endif
}

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

inline void join_path(char *dst, const char *base_path, const char *sub_path) {
    ASSERT(snprintf(dst, Config::System::MAX_PATH_LEN, "%s%s", base_path, sub_path) > 0);
}