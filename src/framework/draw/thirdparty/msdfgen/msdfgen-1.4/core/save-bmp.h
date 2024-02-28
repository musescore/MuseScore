
#pragma once

#include "Bitmap.h"

namespace msdfgen {

/// Saves the bitmap as a BMP file.
bool saveBmp(const Bitmap<float> &bitmap, const char *filename);
bool saveBmp(const Bitmap<unsigned char> &bitmap, const char *filename);
bool saveBmp(const Bitmap<FloatRGB> &bitmap, const char *filename);

}
