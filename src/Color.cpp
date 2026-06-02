#include "Color.hpp"

int_color calculateColorFromRGB(float r, float g, float b) {
    int red = int(r * 255.0f);
    int green = int(g * 255.0f);
    int blue = int(b * 255.0f);

    return 0xFF000000 | red << 16 | green << 8 | blue;
}

int_color calculateColorFromRGBF(float r, float g, float b) {
    int red = int(r * 255.0f);
    int green = int(g * 255.0f);
    int blue = int(b * 255.0f);
    int alpha = 255;

    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

int getRed(int_color color) {
    return (color & 0x00FF0000) >> 16;
}

int getGreen(int_color color) {
    return (color & 0x0000FF00) >> 8;
}

int getBlue(int_color color) {
    return (color & 0x000000FF) >> 0;
}

float getRedF(int_color color) {
    return float((color & 0x00FF0000) >> 16) / 255.0f;
}

float getGreenF(int_color color) {
    return float((color & 0x0000FF00) >> 8) / 255.0f;
}

float getBlueF(int_color color) {
    return float((color & 0x000000FF) >> 0) / 255.0f;
}