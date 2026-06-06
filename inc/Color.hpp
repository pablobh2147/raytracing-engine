#pragma once

#include "math/Vector.hpp"

namespace hzr {

typedef Vector4f Color;
typedef unsigned int int_color;

int_color calculateColorFromRGB(float r, float g, float b);
int_color calculateColorFromRGBF(float r, float g, float b);

int getRed(int_color color);
int getGreen(int_color color);
int getBlue(int_color color);

float getRedF(int_color color);
float getGreenF(int_color color);
float getBlueF(int_color color);

}  // namespace hzr