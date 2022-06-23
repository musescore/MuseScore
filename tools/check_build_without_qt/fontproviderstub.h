#ifndef FONTPROVIDERSTUB_H
#define FONTPROVIDERSTUB_H

#include "engraving/infrastructure/draw/ifontprovider.h"

namespace mu::draw {
class FontProviderStub : public IFontProvider
{
public:

    int addApplicationFont(const String& family, const String& path) override;
    void insertSubstitution(const String& familyName, const String& substituteName) override;

    qreal lineSpacing(const Font& f) const override;
    qreal xHeight(const Font& f) const override;
    qreal height(const Font& f) const override;
    qreal ascent(const Font& f) const override;
    qreal descent(const Font& f) const override;

    bool inFont(const Font& f, Char ch) const override;
    bool inFontUcs4(const Font& f, uint ucs4) const override;

    // Text
    qreal horizontalAdvance(const Font& f, const String& string) const override;
    qreal horizontalAdvance(const Font& f, const Char& ch) const override;

    RectF boundingRect(const Font& f, const String& string) const override;
    RectF boundingRect(const Font& f, const Char& ch) const override;
    RectF boundingRect(const Font& f, const RectF& r, int flags, const String& string) const override;
    RectF tightBoundingRect(const Font& f, const String& string) const override;

    // Score symbols
    RectF symBBox(const Font& f, uint ucs4, qreal DPI_F) const override;
    qreal symAdvance(const Font& f, uint ucs4, qreal DPI_F) const override;
};
}

#endif // FONTPROVIDERSTUB_H
