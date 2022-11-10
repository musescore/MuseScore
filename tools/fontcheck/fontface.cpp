#include "fontface.h"

#include <QFile>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BBOX_H
#include FT_TRUETYPE_TABLES_H
#include <hb-ft.h>

#include "log.h"

static const hb_tag_t KernTag = HB_TAG('k', 'e', 'r', 'n'); // kerning operations
static const hb_tag_t LigaTag = HB_TAG('l', 'i', 'g', 'a'); // standard ligature substitution
static const hb_tag_t CligTag = HB_TAG('c', 'l', 'i', 'g'); // contextual ligature substitution
static const hb_tag_t DligTag = HB_TAG('d', 'l', 'i', 'g'); // contextual ligature substitution
static const hb_tag_t HligTag = HB_TAG('h', 'l', 'i', 'g'); // contextual ligature substitution

static const std::vector<hb_feature_t> HB_FEATURES = {
    { KernTag, 1, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
    { LigaTag, 1, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
    { CligTag, 1, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
    { DligTag, 1, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
    { HligTag, 1, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END }
};

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
    hb_font_t* hb_font = nullptr;
};

FontFace::FontFace()
{
    m_data = new FData();
}

FontFace::~FontFace()
{
    FT_Done_Face(m_data->face);
    if (m_data->hb_font) {
        hb_font_destroy(m_data->hb_font);
    }
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

    m_data->hb_font = hb_ft_font_create(m_data->face, NULL);

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

std::vector<FontFace::Glyph> FontFace::allGlyphs(bool withBBox) const
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

std::vector<glyph_idx_t> FontFace::glyphs(const char32_t* text, int text_length, bool withLigatures) const
{
    std::vector<glyph_idx_t> result;

    if (withLigatures) {
        hb_buffer_t* hb_buffer = hb_buffer_create();
        hb_segment_properties_t props = HB_SEGMENT_PROPERTIES_DEFAULT;

        hb_buffer_add_utf32(hb_buffer, (uint32_t*)text, text_length, 0, -1);
        hb_buffer_set_direction(hb_buffer, props.direction);
        hb_buffer_set_script(hb_buffer, props.script);

        hb_buffer_set_segment_properties(hb_buffer, &props);
        hb_buffer_guess_segment_properties(hb_buffer);

        hb_shape(m_data->hb_font, hb_buffer, &HB_FEATURES[0], HB_FEATURES.size());
        unsigned int len = hb_buffer_get_length(hb_buffer);
        result.reserve(len);

        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
        //hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

        for (unsigned int i = 0; i < len; i++) {
            result.push_back(info[i].codepoint);
        }

        hb_buffer_destroy(hb_buffer);
    } else {
        for (int i = 0; i < text_length; ++i) {
            FT_UInt index = FT_Get_Char_Index(m_data->face, text[i]);
            result.push_back(static_cast<glyph_idx_t>(index));
        }
    }

    return result;
}
