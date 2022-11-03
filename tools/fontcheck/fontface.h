#ifndef FONTFACE_H
#define FONTFACE_H

#include <vector>

#include <QString>
#include <QByteArray>

using glyph_idx_t = uint32_t;

class FontFace
{
public:
    FontFace();
    ~FontFace();

    struct Glyph {
        char32_t code = 0;
        glyph_idx_t index = 0;
        bool hasBBox = false;
    };

    bool load(const QString& path);
    bool load(const QByteArray& fontData);

    glyph_idx_t glyphIndex(char32_t ucs4) const;
    bool inFont(char32_t ucs4) const;

    std::vector<Glyph> glyphs(bool withBBox) const;

private:
    struct FData;

    bool hasBBox(glyph_idx_t idx) const;

    FData* m_data = nullptr;
};

#endif // FONTFACE_H
