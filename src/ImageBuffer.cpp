#include "ImageBuffer.hpp"

#include <stb_image_write.h>

namespace rtx {

ImageBuffer::ImageBuffer(uint32_t width, uint32_t height)
    : m_width(width),
      m_height(height),
      m_data(new uint8_t[width * height * CANVAS_CHANNELS]) {}

ImageBuffer::~ImageBuffer() { delete[] m_data; }

void ImageBuffer::SetPixel(uint32_t x, uint32_t y, int_color color) {
    size_t index = GetPixelIndex(x, y);

    m_data[index + 0] = getRed(color);
    m_data[index + 1] = getGreen(color);
    m_data[index + 2] = getBlue(color);
}

int_color ImageBuffer::GetPixel(uint32_t x, uint32_t y) {
    const size_t index = GetPixelIndex(x, y);
    return calculateColorFromRGB(m_data[index + 0], m_data[index + 1], m_data[index + 2]);
}

size_t ImageBuffer::GetPixelIndex(uint32_t x, uint32_t y) const {
    // We use height - 1 - y because the origin of the image is at the bottom left corner
    return (x + (m_height - 1 - y) * m_width) * CANVAS_CHANNELS;
}

void WriteImageToDisk(const ImageBuffer& img, const char* filename) {
    stbi_write_png(filename, img.GetWidth(), img.GetHeight(), img.GetChannels(), img.GetData(), 0);
}

}  // namespace rtx