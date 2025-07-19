#ifndef FONTPROVIDERSTUB_H
#define FONTPROVIDERSTUB_H

#include "engraving/draw/ifontprovider.h"

namespace mu::draw {
class FontProviderStub : public IFontProvider
{
public:

    int addSymbolFont(const QString& family, const io::path_t& path) override;
    int addTextFont(const io::path_t& path) override;
    void insertSubstitution(const QString& familyName, const QString& substituteName) override;

    qreal lineSpacing(const Font& f) const override;
    qreal xHeight(const Font& f) const override;
    qreal height(const Font& f) const override;
    qreal capHeight(const Font& f) const override;
    qreal ascent(const Font& f) const override;
    qreal descent(const Font& f) const override;

    bool inFont(const Font& f, QChar ch) const override;
    bool inFontUcs4(const Font& f, char32_t ucs4) const override;

    // Text
    qreal horizontalAdvance(const Font& f, const QString& string) const override;
    qreal horizontalAdvance(const Font& f, const QChar& ch) const override;

    RectF boundingRect(const Font& f, const QString& string) const override;
    RectF boundingRect(const Font& f, const QChar& ch) const override;
    RectF boundingRect(const Font& f, const RectF& r, int flags, const QString& string) const override;
    RectF tightBoundingRect(const Font& f, const QString& string) const override;

    // Score symbols
    RectF symBBox(const Font& f, char32_t ucs4, qreal DPI_F) const override;
    qreal symAdvance(const Font& f, char32_t ucs4, qreal DPI_F) const override;
};
}

#endif // FONTPROVIDERSTUB_H
