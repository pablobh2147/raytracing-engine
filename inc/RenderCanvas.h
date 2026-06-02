#pragma once

#include "color.h"

namespace rtx {

	class RenderCanvas {

	private:
		int width;
		int height;
		char* pixels;

	public:
		RenderCanvas(int width, int height);
		~RenderCanvas();

		void setPixel(int x, int y, int_color color);
		int_color getPixel(int x, int y);

		int getWidth() const { return this->width; }
		int getHeight() const { return this->height; }

		const char* getPixels() const { return this->pixels; }

	};

	void writeImageToDisk(RenderCanvas* canvas, const char* filename);

}