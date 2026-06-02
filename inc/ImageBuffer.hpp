#pragma once

#include <cstdint>

#include "Color.hpp"

namespace rtx {

class ImageBuffer {
   private:
    static constexpr uint32_t CANVAS_CHANNELS = 3;

   private:
    uint32_t m_width;
    uint32_t m_height;
    uint8_t* m_data;

   public:
    ImageBuffer(uint32_t width, uint32_t height);
    ~ImageBuffer();

    void SetPixel(uint32_t x, uint32_t y, int_color color);
    int_color GetPixel(uint32_t x, uint32_t y);

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetChannels() const { return CANVAS_CHANNELS; }

    const uint8_t* GetData() const { return m_data; }

   private:
    size_t GetPixelIndex(uint32_t x, uint32_t y) const;
};

void WriteImageToDisk(const ImageBuffer& img, const char* filename);

}  // namespace rtx