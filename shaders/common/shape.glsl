#ifndef COMMON_SHAPE_H
#define COMMON_SHAPE_H

struct Sphere {
    vec3 position;
    float radius;
    uint material_index;
};

struct Plane {
    vec3 normal;
    float distance;
    uint material_index;
};

struct Triangle {
    vec3 v1;
    vec3 v2;
    vec3 v3;
    uint material_index;
};

#endif // COMMON_SHAPE_H