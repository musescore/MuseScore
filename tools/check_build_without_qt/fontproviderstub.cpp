#include "fontproviderstub.h"

using namespace mu;
using namespace mu::draw;

int FontProviderStub::addApplicationFont(const String&, const String&)
{
    return -1;
}

void FontProviderStub::insertSubstitution(const String&, const String&)
{
}

qreal FontProviderStub::lineSpacing(const Font&) const
{
    return 0.0;
}

qreal FontProviderStub::xHeight(const Font&) const
{
    return 0.0;
}

qreal FontProviderStub::height(const Font&) const
{
    return 0.0;
}

qreal FontProviderStub::ascent(const Font&) const
{
    return 0.0;
}

qreal FontProviderStub::descent(const Font&) const
{
    return 0.0;
}

bool FontProviderStub::inFont(const Font&, Char) const
{
    return false;
}

bool FontProviderStub::inFontUcs4(const Font&, uint) const
{
    return false;
}

// Text
qreal FontProviderStub::horizontalAdvance(const Font&, const String&) const
{
    return 0.0;
}

qreal FontProviderStub::horizontalAdvance(const Font&, const Char&) const
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
RectF FontProviderStub::symBBox(const Font&, uint, qreal) const
{
    return RectF();
}

qreal FontProviderStub::symAdvance(const Font&, uint, qreal) const
{
    return 0.0;
}
