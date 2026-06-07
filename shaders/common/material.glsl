#ifndef COMMON_MATERIAL_H
#define COMMON_MATERIAL_H

struct Material {
    vec3 albedo;
    float roughness;
    vec3 emission;
    float metallic;
};

#endif // COMMON_MATERIAL_H
