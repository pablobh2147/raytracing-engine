# Raytracing Engine

A raytracing renderer with physically-based materials, built as a personal learning project.

## Examples

<p align="center">
  <img src="docs/img/example-1.gif" alt="Example render 1" width="45%">
  <img src="docs/img/example-2.gif" alt="Example render 2" width="45%">
</p>

## Overview

This project implements a path tracer capable of rendering scenes with realistic lighting and materials. It features:

- **Physically-based rendering** with metallic/roughness workflow
- **Global illumination** through path tracing
- **Material system** supporting albedo, roughness, metallic, and emissive properties
- **Multi-sample anti-aliasing** for noise reduction
- **Animation support** for camera movement sequences

## Technical Details

- **Language**: C++20
- **Math library**: GLM
- **Image output**: STB Image Write
- **Build system**: CMake
- **Optimizations**: Native CPU optimizations, LTO, fast-math

The renderer currently runs entirely on the CPU with aggressive compiler optimizations.

## Future Goals

- **GPU acceleration** using Vulkan compute shaders
- **Enhanced material models** for more realistic surfaces
- **Scene file format** for easier scene configuration
- **Real-time preview** capabilities

## Building

```bash
# Clone with submodules
git clone --recursive https://github.com/pablobh2147/raytracing-engine

# Build
mkdir build && cd build
cmake ..
make

# Run
./raytracer
```