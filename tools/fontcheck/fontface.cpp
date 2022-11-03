#include "fontface.h"

#include <QFile>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BBOX_H
#include FT_TRUETYPE_TABLES_H

#include "log.h"

static FT_Library ftlib = nullptr;
static bool _init_ft()
{
    int error = 0;
    if (!ftlib) {
        error = FT_Init_FreeType(&ftlib);
        if (!ftlib || error) {
            LOGE() << "init freetype library failed";
        }
    }
    return error == 0;
}

struct FontFace::FData
{
    QByteArray fontData;
    FT_Face face = nullptr;
};

FontFace::FontFace()
{
    m_data = new FData();
}

FontFace::~FontFace()
{
    FT_Done_Face(m_data->face);
    delete m_data;
}

bool FontFace::load(const QString& path)
{
    if (!QFile::exists(path)) {
        LOGE() << "not found file: " << path.toStdString();
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open file: " << path.toStdString();
        return false;
    }

    QByteArray fontData = file.readAll();
    return load(fontData);
}

bool FontFace::load(const QByteArray& fontData)
{
    if (!_init_ft()) {
        return false;
    }

    m_data->fontData = fontData;

    int rval = FT_New_Memory_Face(ftlib, (FT_Byte*)m_data->fontData.constData(),
                                  (FT_Long)m_data->fontData.size(), 0, &m_data->face);
    if (rval) {
        LOGE() << "freetype: cannot create face";
        return false;
    }

    FT_Set_Pixel_Sizes(m_data->face, 0, 200);

    return true;
}

glyph_idx_t FontFace::glyphIndex(char32_t ucs4) const
{
    FT_UInt index = FT_Get_Char_Index(m_data->face, ucs4);
    return static_cast<glyph_idx_t>(index);
}

bool FontFace::inFont(char32_t ucs4) const
{
    glyph_idx_t idx = glyphIndex(ucs4);
    if (idx == 0) {
        return false;
    }

    return hasBBox(idx);
}

bool FontFace::hasBBox(glyph_idx_t idx) const
{
    if (FT_Load_Glyph(m_data->face, idx, FT_LOAD_DEFAULT) != 0) {
        LOGE() << "failed load glyth: " << idx;
        return false;
    }

    FT_GlyphSlot slot = m_data->face->glyph;
    bool hasBB = (slot->metrics.width > 0) && (slot->metrics.height > 0);
    return hasBB;
}

std::vector<FontFace::Glyph> FontFace::glyphs(bool withBBox) const
{
    std::vector<Glyph> gs;

    FT_UInt gindex = 0;
    FT_ULong charcode = FT_Get_First_Char(m_data->face, &gindex);
    while (gindex != 0)
    {
        Glyph g;
        g.code = static_cast<char32_t>(charcode);
        g.index = static_cast<glyph_idx_t>(gindex);

        g.hasBBox = hasBBox(static_cast<glyph_idx_t>(gindex));

        if (withBBox) {
            if (!g.hasBBox) {
                charcode = FT_Get_Next_Char(m_data->face, charcode, &gindex);
                continue;
            }
        }

        gs.push_back(std::move(g));

        charcode = FT_Get_Next_Char(m_data->face, charcode, &gindex);
    }

    return gs;
}
