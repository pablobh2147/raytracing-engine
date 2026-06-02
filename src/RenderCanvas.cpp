#include "RenderCanvas.h"

#include <stb_image_write.h>

namespace rtx {

	constexpr int CANVAS_CHANNELS = 3;

	RenderCanvas::RenderCanvas(int width, int height) {
		this->width = width;
		this->height = height;
		this->pixels = new char[width * height * CANVAS_CHANNELS];
	}

	RenderCanvas::~RenderCanvas() {
		delete[] this->pixels;
	}

	void RenderCanvas::setPixel(int x, int y, int_color color) {
		// We use height - 1 - y because the origin of the image is at the bottom left corner
		const int index = (x + (this->height - 1 - y) * this->width) * CANVAS_CHANNELS;

		this->pixels[index + 0] = getRed(color);
		this->pixels[index + 1] = getGreen(color);
		this->pixels[index + 2] = getBlue(color);
	}

	int_color RenderCanvas::getPixel(int x, int y) {
		const int index = (x + (this->height - 1 - y) * this->width) * CANVAS_CHANNELS;

		return calculateColorFromRGB(this->pixels[index + 0], this->pixels[index + 1], this->pixels[index + 2]);
	}

	void writeImageToDisk(RenderCanvas* canvas, const char* filename) {
		stbi_write_png(filename, canvas->getWidth(), canvas->getHeight(), CANVAS_CHANNELS, canvas->getPixels(), 0);
	}

}