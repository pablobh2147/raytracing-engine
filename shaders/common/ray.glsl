#ifndef COMMON_RAY_H
#define COMMON_RAY_H

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct RaycastResult {
    bool hit;
    vec3 position;
    vec3 normal;
    float distance;
    uint material_index;
};

#endif // COMMON_RAY_H