#include "vec3.h"
#include <math.h>

Vec3 vec3_normalize_fast(Vec3 v) {
    float inv_len = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x *= inv_len;
    v.y *= inv_len;
    v.z *= inv_len;
    return v;
}