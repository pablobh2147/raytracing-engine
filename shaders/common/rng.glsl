/*  ==============================================================================
 *  Hoshizora (星空) — Physically Based Renderer
 *  ------------------------------------------------------------------------------
 *  Copyright (c) 2026 Pablo Bermejo Hernández. All rights reserved.
 *
 *  This software is licensed under the MIT License.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 *  See the LICENSE file in the project root for full license information.
 *  ==============================================================================
 */

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