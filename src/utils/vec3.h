#ifndef VEC3_H
#define VEC3_H

typedef struct {
    float x, y, z;
} Vec3;

Vec3 vec3_normalize_fast(Vec3 v);

#endif // VEC3_H