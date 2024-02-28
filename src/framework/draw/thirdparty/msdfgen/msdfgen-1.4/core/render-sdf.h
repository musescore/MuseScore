
#pragma once

#include "Vector2.h"
#include "Bitmap.h"

namespace msdfgen {

/// Reconstructs the shape's appearance into output from the distance field sdf.
void renderSDF(Bitmap<float> &output, const Bitmap<float> &sdf, double pxRange = 0);
void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<float> &sdf, double pxRange = 0);
void renderSDF(Bitmap<float> &output, const Bitmap<FloatRGB> &sdf, double pxRange = 0);
void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<FloatRGB> &sdf, double pxRange = 0);

/// Snaps the values of the floating-point bitmaps into one of the 256 values representable in a standard 8-bit bitmap.
void simulate8bit(Bitmap<float> &bitmap);
void simulate8bit(Bitmap<FloatRGB> &bitmap);

}
