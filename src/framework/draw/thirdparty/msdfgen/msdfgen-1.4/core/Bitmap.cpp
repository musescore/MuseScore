
#include "Bitmap.h"

#include <cstring>

namespace msdfgen {

template <typename T>
Bitmap<T>::Bitmap() : content(NULL), w(0), h(0) { }

template <typename T>
Bitmap<T>::Bitmap(int width, int height) : w(width), h(height) {
    content = new T[w*h];
}

template <typename T>
Bitmap<T>::Bitmap(const Bitmap<T> &orig) : w(orig.w), h(orig.h) {
    content = new T[w*h];
    memcpy(content, orig.content, w*h*sizeof(T));
}

#ifdef MSDFGEN_USE_CPP11
template <typename T>
Bitmap<T>::Bitmap(Bitmap<T> &&orig) : content(orig.content), w(orig.w), h(orig.h) {
    orig.content = NULL;
}
#endif

template <typename T>
Bitmap<T>::~Bitmap() {
    delete [] content;
}

template <typename T>
Bitmap<T> & Bitmap<T>::operator=(const Bitmap<T> &orig) {
    delete [] content;
    w = orig.w, h = orig.h;
    content = new T[w*h];
    memcpy(content, orig.content, w*h*sizeof(T));
    return *this;
}

#ifdef MSDFGEN_USE_CPP11
template <typename T>
Bitmap<T> & Bitmap<T>::operator=(Bitmap<T> &&orig) {
    delete [] content;
    content = orig.content;
    w = orig.w, h = orig.h;
    orig.content = NULL;
    return *this;
}
#endif

template <typename T>
int Bitmap<T>::width() const {
    return w;
}

template <typename T>
int Bitmap<T>::height() const {
    return h;
}

template <typename T>
T & Bitmap<T>::operator()(int x, int y) {
    return content[y*w+x];
}

template <typename T>
const T & Bitmap<T>::operator()(int x, int y) const {
    return content[y*w+x];
}

template<typename T>
T * Bitmap<T>::takeMemoryAway()
{
	T *temp = content;
	w = h = 0;
	content = nullptr;
	return temp;
}

template<typename T>
const T * Bitmap<T>::contentMemory() const
{
	return content;
}

template class Bitmap<float>;
template class Bitmap<FloatRGB>;
template class Bitmap<unsigned char>;

}
