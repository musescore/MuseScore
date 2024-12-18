#include "fontproviderstub.h"

using namespace mu;
using namespace mu::draw;

int FontProviderStub::addSymbolFont(const QString&, const io::path_t &)
{
    return -1;
}

int FontProviderStub::addTextFont(const io::path_t& path)
{
    return -1;
}

void FontProviderStub::insertSubstitution(const QString&, const QString&)
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

qreal FontProviderStub::capHeight(const Font&) const
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

bool FontProviderStub::inFont(const Font&, QChar) const
{
    return false;
}

bool FontProviderStub::inFontUcs4(const Font&, uint) const
{
    return false;
}

// Text
qreal FontProviderStub::horizontalAdvance(const Font&, const QString&) const
{
    return 0.0;
}

qreal FontProviderStub::horizontalAdvance(const Font&, const QChar&) const
{
    return 0.0;
}

RectF FontProviderStub::boundingRect(const Font&, const QString&) const
{
    return RectF();
}

RectF FontProviderStub::boundingRect(const Font&, const QChar&) const
{
    return RectF();
}

RectF FontProviderStub::boundingRect(const Font&, const RectF&, int, const QString&) const
{
    return RectF();
}

RectF FontProviderStub::tightBoundingRect(const Font&, const QString&) const
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
