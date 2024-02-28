
#pragma once

namespace msdfgen {

/// A floating-point RGB pixel.
struct FloatRGB {
    float r, g, b;
};

/// A 2D image bitmap.
template <typename T>
class Bitmap {

public:
    Bitmap();
    Bitmap(int width, int height);
    Bitmap(const Bitmap<T> &orig);
#ifdef MSDFGEN_USE_CPP11
    Bitmap(Bitmap<T> &&orig);
#endif
    ~Bitmap();
    Bitmap<T> & operator=(const Bitmap<T> &orig);
#ifdef MSDFGEN_USE_CPP11
    Bitmap<T> & operator=(Bitmap<T> &&orig);
#endif
    /// Bitmap width in pixels.
    int width() const;
    /// Bitmap height in pixels.
    int height() const;
    T & operator()(int x, int y);
    const T & operator()(int x, int y) const;

	T *takeMemoryAway();
	const T *contentMemory() const;

private:
    T *content;
    int w, h;

};

}
