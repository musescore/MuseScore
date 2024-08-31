#ifndef FONTPROVIDERSTUB_H
#define FONTPROVIDERSTUB_H

#include "draw/ifontprovider.h"

namespace muse::draw {
class FontProviderStub : public IFontProvider
{
public:

    int addSymbolFont(const String& family, const io::path_t& path) override;

    double lineSpacing(const Font& f) const override;
    double xHeight(const Font& f) const override;
    double height(const Font& f) const override;
    double capHeight(const Font& f) const override;
    double ascent(const Font& f) const override;
    double descent(const Font& f) const override;

    bool inFont(const Font& f, Char ch) const override;
    bool inFontUcs4(const Font& f, char32_t ucs4) const override;

    // Text
    double horizontalAdvance(const Font& f, const String& string) const override;
    double horizontalAdvance(const Font& f, const Char& ch) const override;

    RectF boundingRect(const Font& f, const String& string) const override;
    RectF boundingRect(const Font& f, const Char& ch) const override;
    RectF boundingRect(const Font& f, const RectF& r, int flags, const String& string) const override;
    RectF tightBoundingRect(const Font& f, const String& string) const override;

    // Score symbols
    RectF symBBox(const Font& f, char32_t ucs4, double DPI_F) const override;
    double symAdvance(const Font& f, char32_t ucs4, double DPI_F) const override;
};
}

#endif // FONTPROVIDERSTUB_H
