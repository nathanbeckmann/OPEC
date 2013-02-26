#pragma once

#include <math.h>
#include <memory>

static inline bool feq(double x, double y) {
    return fabs(x-y) < 1e-3;
}

template<typename T>
static bool approx(T a, T b, uint32_t factor) {
    return std::abs(a-b) * factor < std::min(a, b);
}
