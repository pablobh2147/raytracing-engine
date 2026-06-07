#ifndef COMMON_RNG_H
#define COMMON_RNG_H

const uint UINT32_MAX = 4294967295u;

uint PcgHash(uint seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandomFloat(inout uint seed) {
    seed = PcgHash(seed);
    return float(seed) / float(UINT32_MAX);
}

vec3 RandomUnitSphere(inout uint seed) {
    vec3 v;
    do {
        v = vec3(
            RandomFloat(seed) * 2.0 - 1.0,
            RandomFloat(seed) * 2.0 - 1.0,
            RandomFloat(seed) * 2.0 - 1.0
        );
    } while (dot(v, v) > 1.0 || dot(v, v) < 1e-6);
    return normalize(v);
}

#endif // COMMON_RNG_H