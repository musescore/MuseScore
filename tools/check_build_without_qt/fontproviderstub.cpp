#include "fontproviderstub.h"

using namespace muse;
using namespace muse::draw;

int FontProviderStub::addSymbolFont(const String&, const io::path_t&)
{
    return -1;
}

double FontProviderStub::lineSpacing(const Font&) const
{
    return 0.0;
}

double FontProviderStub::xHeight(const Font&) const
{
    return 0.0;
}

double FontProviderStub::height(const Font&) const
{
    return 0.0;
}

double FontProviderStub::capHeight(const Font&) const
{
    return 0.0;
}

double FontProviderStub::ascent(const Font&) const
{
    return 0.0;
}

double FontProviderStub::descent(const Font&) const
{
    return 0.0;
}

bool FontProviderStub::inFont(const Font&, Char) const
{
    return false;
}

bool FontProviderStub::inFontUcs4(const Font&, char32_t) const
{
    return false;
}

// Text
double FontProviderStub::horizontalAdvance(const Font&, const String&) const
{
    return 0.0;
}

double FontProviderStub::horizontalAdvance(const Font&, const Char&) const
{
    return 0.0;
}

RectF FontProviderStub::boundingRect(const Font&, const String&) const
{
    return RectF();
}

RectF FontProviderStub::boundingRect(const Font&, const Char&) const
{
    return RectF();
}

RectF FontProviderStub::boundingRect(const Font&, const RectF&, int, const String&) const
{
    return RectF();
}

RectF FontProviderStub::tightBoundingRect(const Font&, const String&) const
{
    return RectF();
}

// Score symbols
RectF FontProviderStub::symBBox(const Font&, char32_t, double) const
{
    return RectF();
}

double FontProviderStub::symAdvance(const Font&, char32_t, double) const
{
    return 0.0;
}
