
#include "render-sdf.h"

#include "arithmetics.hpp"

namespace msdfgen {

template <typename S>
inline FloatRGB mix(FloatRGB a, FloatRGB b, S weight) {
    FloatRGB output = {
        mix(a.r, b.r, weight),
        mix(a.g, b.g, weight),
        mix(a.b, b.b, weight)
    };
    return output;
}

template <typename T>
static T sample(const Bitmap<T> &bitmap, Point2 pos) {
    int w = bitmap.width(), h = bitmap.height();
    double x = pos.x*w-.5;
    double y = pos.y*h-.5;
    int l = (int) floor(x);
    int b = (int) floor(y);
    int r = l+1;
    int t = b+1;
    double lr = x-l;
    double bt = y-b;
    l = clamp(l, w-1), r = clamp(r, w-1);
    b = clamp(b, h-1), t = clamp(t, h-1);
    return mix(mix(bitmap(l, b), bitmap(r, b), lr), mix(bitmap(l, t), bitmap(r, t), lr), bt);
}

static float distVal(float dist, double pxRange) {
    if (!pxRange)
        return dist > .5;
    return (float) clamp((dist-.5)*pxRange+.5);
}

void renderSDF(Bitmap<float> &output, const Bitmap<float> &sdf, double pxRange) {
    int w = output.width(), h = output.height();
    pxRange *= (double) (w+h)/(sdf.width()+sdf.height());
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            float s = sample(sdf, Point2((x+.5)/w, (y+.5)/h));
            output(x, y) = distVal(s, pxRange);
        }
}

void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<float> &sdf, double pxRange) {
    int w = output.width(), h = output.height();
    pxRange *= (double) (w+h)/(sdf.width()+sdf.height());
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            float s = sample(sdf, Point2((x+.5)/w, (y+.5)/h));
            float v = distVal(s, pxRange);
            output(x, y).r = v;
            output(x, y).g = v;
            output(x, y).b = v;
        }
}

void renderSDF(Bitmap<float> &output, const Bitmap<FloatRGB> &sdf, double pxRange) {
    int w = output.width(), h = output.height();
    pxRange *= (double) (w+h)/(sdf.width()+sdf.height());
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            FloatRGB s = sample(sdf, Point2((x+.5)/w, (y+.5)/h));
            output(x, y) = distVal(median(s.r, s.g, s.b), pxRange);
        }
}

void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<FloatRGB> &sdf, double pxRange) {
    int w = output.width(), h = output.height();
    pxRange *= (double) (w+h)/(sdf.width()+sdf.height());
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            FloatRGB s = sample(sdf, Point2((x+.5)/w, (y+.5)/h));
            output(x, y).r = distVal(s.r, pxRange);
            output(x, y).g = distVal(s.g, pxRange);
            output(x, y).b = distVal(s.b, pxRange);
        }
}

void simulate8bit(Bitmap<float> &bitmap) {
    int w = bitmap.width(), h = bitmap.height();
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            unsigned char v = clamp(int(bitmap(x, y)*0x100), 0xff);
            bitmap(x, y) = v/255.f;
        }
}

void simulate8bit(Bitmap<FloatRGB> &bitmap) {
    int w = bitmap.width(), h = bitmap.height();
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            unsigned char r = clamp(int(bitmap(x, y).r*0x100), 0xff);
            unsigned char g = clamp(int(bitmap(x, y).g*0x100), 0xff);
            unsigned char b = clamp(int(bitmap(x, y).b*0x100), 0xff);
            bitmap(x, y).r = r/255.f;
            bitmap(x, y).g = g/255.f;
            bitmap(x, y).b = b/255.f;
        }
}

}
