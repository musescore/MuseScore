/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "fontfaceft.h"

#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BBOX_H
#include FT_TRUETYPE_TABLES_H
#include <hb-ft.h>

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
#include <ext/import-font.h>
#endif

#include "global/types/bytearray.h"
#include "global/io/file.h"

#include "log.h"

using namespace muse::draw;

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

#define FLOOR(x)    ((x) & -64)
#define CEIL(x)     (((x) + 63) & -64)
#define TRUNC(x)    ((x) >> 6)
#define ROUND(x)    (((x) + 32) & -64)

struct muse::draw::GlyphMetrics
{
    FBBox bbox;
    FT_Fixed linearAdvance = 0;
};

struct muse::draw::SymbolMetrics
{
    glyph_idx_t idx = 0;
    FT_BBox bbox;
    FT_Fixed linearAdvance = 0;
};

struct muse::draw::FData
{
    ByteArray fontData;
    FT_Face face = nullptr;
    hb_font_t* hb_font = nullptr;
    std::unordered_map<glyph_idx_t, GlyphMetrics> glyphsMetrics;
    std::unordered_map<glyph_idx_t, SymbolMetrics> symbolMetrics;
    FT_Size_Metrics metrics;
};

FontFaceFT::FontFaceFT()
{
    m_data = new FData();
}

FontFaceFT::~FontFaceFT()
{
    if (m_data->hb_font) {
        hb_font_destroy(m_data->hb_font);
    }
    FT_Done_Face(m_data->face);
    delete m_data;
}

bool FontFaceFT::load(const FaceKey& key, const io::path_t& path, bool isSymbolMode)
{
    if (!_init_ft()) {
        return false;
    }

    m_key = key;
    m_isSymbolMode = isSymbolMode;

    {
        io::File file(path);
        if (!file.open(io::IODevice::ReadOnly)) {
            return false;
        }

        m_data->fontData = file.readAll();
    }

    int rval = FT_New_Memory_Face(ftlib, (FT_Byte*)m_data->fontData.constData(),
                                  (FT_Long)m_data->fontData.size(), 0, &m_data->face);
    if (rval) {
        LOGE() << "freetype: cannot create face: " << m_key.dataKey.family().id() << ", rval: " << rval;
        return false;
    }

    if (m_isSymbolMode) {
        FT_Set_Pixel_Sizes(m_data->face, 0, int(m_key.pixelSize + .5));
    } else {
        FT_Set_Char_Size(m_data->face, to_f26d6(m_key.pixelSize), to_f26d6(m_key.pixelSize), 0, 0);

        static FT_Matrix matrix;
        matrix.xx = 0x10000;
        matrix.yy = 0x10000;
        matrix.xy = 0;
        matrix.yx = 0;
        FT_Set_Transform(m_data->face, &matrix, nullptr);

        m_data->hb_font = hb_ft_font_create(m_data->face, NULL);
    }

    m_data->metrics = m_data->face->size->metrics;

    return true;
}

const FaceKey& FontFaceFT::key() const
{
    return m_key;
}

bool FontFaceFT::isSymbolMode() const
{
    return m_isSymbolMode;
}

std::vector<GlyphPos> FontFaceFT::glyphs(const char32_t* text, int text_length) const
{
    if (text_length < 1) {
        return std::vector<GlyphPos>();
    }

    std::vector<GlyphPos> result;
    if (m_isSymbolMode) {
        for (int i = 0; i < text_length; ++i) {
            glyph_idx_t idx = glyphIndex(text[i]);
            SymbolMetrics* sm = symbolMetrics(idx);
            IF_ASSERT_FAILED(sm) {
                return std::vector<GlyphPos>();
            }

            GlyphPos p;
            p.idx = sm->idx;
            p.x_advance = sm->linearAdvance;

            result.push_back(std::move(p));
        }
    } else {
        hb_buffer_t* hb_buffer = hb_buffer_create();
        hb_segment_properties_t props = HB_SEGMENT_PROPERTIES_DEFAULT;

        hb_buffer_add_utf32(hb_buffer, (uint32_t*)text, text_length, 0, -1);
        hb_buffer_set_direction(hb_buffer, props.direction);
        hb_buffer_set_script(hb_buffer, props.script);

        hb_buffer_set_segment_properties(hb_buffer, &props);
        hb_buffer_guess_segment_properties(hb_buffer);

        hb_shape(m_data->hb_font, hb_buffer, &HB_FEATURES[0], static_cast<unsigned int>(HB_FEATURES.size()));
        unsigned int len = hb_buffer_get_length(hb_buffer);
        result.reserve(len);

        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
        hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

        for (unsigned int i = 0; i < len; i++) {
            result.push_back({ info[i].codepoint, static_cast<f26dot6_t>(pos[i].x_advance) });
        }

        hb_buffer_destroy(hb_buffer);
    }

    return result;
}

glyph_idx_t FontFaceFT::glyphIndex(char32_t ucs4) const
{
    if (ucs4 == 0) {
        return 0;
    }

    FT_UInt index = FT_Get_Char_Index(m_data->face, ucs4);
    return static_cast<glyph_idx_t>(index);
}

glyph_idx_t FontFaceFT::glyphIndex(const std::string& glyphName) const
{
    FT_UInt index = FT_Get_Name_Index(m_data->face, glyphName.c_str());
    return static_cast<glyph_idx_t>(index);
}

char32_t FontFaceFT::findCharCode(glyph_idx_t idx) const
{
    auto findC = [this](glyph_idx_t idx)
    {
        FT_UInt gindex = 0;
        FT_ULong charcode = FT_Get_First_Char(m_data->face, &gindex);
        if (gindex == idx) {
            return charcode;
        }

        while (gindex != 0)
        {
            charcode = FT_Get_Next_Char(m_data->face, charcode, &gindex);
            if (gindex == idx) {
                return charcode;
            }
        }

        return FT_ULong(0);
    };

    char32_t c = findC(idx);

    // check
    assert(glyphIndex(c) == idx);

    return c;
}

FBBox FontFaceFT::glyphBbox(glyph_idx_t idx) const
{
    if (isSymbolMode()) {
        SymbolMetrics* sm = symbolMetrics(idx);
        IF_ASSERT_FAILED(sm) {
            return FBBox();
        }
        //! NOTE Moved form MUE FontEngineFT::bbox
        //! double m = 640.0 / dpi_f;
        //! -> to FBBox (f26dot6_t) double m = (640.0 / dpi_f) * (1 / 64);
        //! -> double m = 10.0 / dpi_f;
        //! -> dpi_f = 5.0 constant
        //! -> int m = 2;
        int m = 2;
        FBBox bbox;
        bbox.setCoords(sm->bbox.xMin / m, -sm->bbox.yMax / m,
                       sm->bbox.xMax / m, -sm->bbox.yMin / m);
        return bbox;
    } else {
        GlyphMetrics* gm = glyphMetrics(idx);
        if (!gm) {
            return FBBox();
        }
        return gm->bbox;
    }
}

f26dot6_t FontFaceFT::glyphAdvance(glyph_idx_t idx) const
{
    if (isSymbolMode()) {
        SymbolMetrics* sm = symbolMetrics(idx);
        IF_ASSERT_FAILED(sm) {
            return 0.0;
        }
        return sm->linearAdvance;
    } else {
        GlyphMetrics* gm = glyphMetrics(idx);
        if (!gm) {
            return 0.0;
        }
        return gm->linearAdvance;
    }
}

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
const msdfgen::Shape& FontFaceFT::glyphShape(glyph_idx_t idx) const
{
    static const msdfgen::Shape null;

    FT_UInt index = static_cast<FT_UInt>(idx);
    if (index == 0) {
        return null;
    }

    auto it = m_cache.find(idx);
    if (it != m_cache.end()) {
        return it->second;
    }

    if (FT_Load_Glyph(m_data->face, index, FT_LOAD_DEFAULT) != 0) {
        return null;
    }

    std::pair<glyph_idx_t, msdfgen::Shape> v;
    v.first = idx;
    v.second = msdfgen::loadGlyphSlot(m_data->face->glyph, nullptr);
    v.second.normalize();
    v.second.inverseYAxis = true;

    return m_cache.insert(std::move(v)).first->second;
}

#endif

f26dot6_t FontFaceFT::leading() const
{
    const auto& metrics = m_data->metrics;
    f26dot6_t v = metrics.height - metrics.ascender + metrics.descender;
    return v;
}

f26dot6_t FontFaceFT::ascent() const
{
    f26dot6_t v = m_data->metrics.ascender;
    return v;
}

f26dot6_t FontFaceFT::descent() const
{
    f26dot6_t v = -m_data->metrics.descender;
    return v;
}

f26dot6_t FontFaceFT::xHeight() const
{
    TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(m_data->face, ft_sfnt_os2);
    if (os2 && os2->sxHeight) {
        f26dot6_t result = std::round(os2->sxHeight * m_data->face->size->metrics.y_ppem * 64.0 / (double)m_data->face->units_per_EM);
        return result;
    }

    const glyph_idx_t glyph = glyphIndex('x');
    GlyphMetrics* gm = glyphMetrics(glyph);
    IF_ASSERT_FAILED(gm) {
        return 0;
    }
    return gm->bbox.height();
}

f26dot6_t FontFaceFT::capHeight() const
{
    TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(m_data->face, ft_sfnt_os2);
    if (os2 && os2->sCapHeight) {
        f26dot6_t result = std::round(os2->sCapHeight * m_data->face->size->metrics.y_ppem * 64.0 / (double)m_data->face->units_per_EM);
        return result;
    }

    const glyph_idx_t glyph = glyphIndex('x');
    GlyphMetrics* gm = glyphMetrics(glyph);
    IF_ASSERT_FAILED(gm) {
        return 0;
    }
    return gm->bbox.height();
}

GlyphMetrics* FontFaceFT::glyphMetrics(glyph_idx_t idx) const
{
    if (m_data->glyphsMetrics.find(idx) != m_data->glyphsMetrics.end()) {
        return &m_data->glyphsMetrics.at(idx);
    }

    FT_UInt index = static_cast<FT_UInt>(idx);
    if (index == 0) {
        return nullptr;
    }

    if (FT_Load_Glyph(m_data->face, index, FT_LOAD_DEFAULT) != 0) {
        return nullptr;
    }

    FT_GlyphSlot slot = m_data->face->glyph;

    GlyphMetrics& gm = m_data->glyphsMetrics[idx];

    gm.bbox.setLeft(slot->metrics.horiBearingX);
    gm.bbox.setTop(-slot->metrics.horiBearingY);
    gm.bbox.setWidth(slot->metrics.width);
    gm.bbox.setHeight(slot->metrics.height);

    gm.linearAdvance = slot->linearHoriAdvance >> 10;

    return &gm;
}

SymbolMetrics* FontFaceFT::symbolMetrics(glyph_idx_t idx) const
{
    if (m_data->symbolMetrics.find(idx) != m_data->symbolMetrics.end()) {
        return &m_data->symbolMetrics.at(idx);
    }

    FT_UInt index = static_cast<FT_UInt>(idx);
    if (index == 0) {
        return nullptr;
    }

    if (FT_Load_Glyph(m_data->face, index, FT_LOAD_DEFAULT) != 0) {
        return nullptr;
    }

    SymbolMetrics& sm = m_data->symbolMetrics[idx];

    sm.idx = static_cast<glyph_idx_t>(index);

    if (FT_Outline_Get_BBox(&m_data->face->glyph->outline, &sm.bbox) != 0) {
        return nullptr;
    }

    //! NOTE Moved form MUE FontEngineFT::advance
    //! double advance = linearHoriAdvance * dpi_f / 655360.0;
    //! -> f26dot6_t advance = linearHoriAdvance * dpi_f * 64.0 / 655360.0;
    //! -> dpi_f = 5.0 constant
    //! -> f26dot6_t advance = linearHoriAdvance * 320.0 / 655360.0;
    //! -> f26dot6_t advance = linearHoriAdvance / 2048;
    sm.linearAdvance = m_data->face->glyph->linearHoriAdvance / 2048;

    return &sm;
}
