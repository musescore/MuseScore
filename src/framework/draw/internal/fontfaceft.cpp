/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include <cmath>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BBOX_H
#include FT_TRUETYPE_TABLES_H
#include <hb-ft.h>

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
#include <msdfgen.h>
#include <ext/import-font.h>
#endif

#include "global/types/bytearray.h"

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

struct HeightMetrics
{
    f26dot6_t ascent = 0;
    f26dot6_t descent = 0;
    f26dot6_t leading = 0;
};

struct muse::draw::FData
{
    ByteArray fontData;
    FT_Face face = nullptr;
    hb_font_t* hb_font = nullptr;
    std::unordered_map<glyph_idx_t, GlyphMetrics> glyphsMetrics;
    std::unordered_map<glyph_idx_t, SymbolMetrics> symbolMetrics;
    FT_Size_Metrics metrics;
    HeightMetrics heightMetrics;
};

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
static const int SDF_DIM = 28; //! default dimension of generated SDF. Real SDF can have another size
static const int SDF_SHAPE_SIZE = 100; //! effective shape size
static const double MIN_SDF_OUTLINE_PIXELS = 7.0;
static const double MAX_SDF_SCALE = 4.0;

static double sdfScaleForShape(const msdfgen::Vector2& shapeDims, double baseScale)
{
    double minDim = std::min(shapeDims.x, shapeDims.y);
    if (minDim <= 0.0) {
        return baseScale;
    }

    return std::min(std::max(baseScale, MIN_SDF_OUTLINE_PIXELS / minDim), MAX_SDF_SCALE);
}

static void generateSdf(GlyphImage& out, msdfgen::Shape& shape)
{
    struct Bounds
    {
        double l, b, r, t;
    };
    Bounds bounds = { 1e240, 1e240, -1e240, -1e240 };

    shape.normalize();
    shape.orientContours();

    shape.bound(bounds.l, bounds.b, bounds.r, bounds.t);
    msdfgen::Vector2 shapeDims(bounds.r - bounds.l, bounds.t - bounds.b);

    int pxRange = 4;
    int pixelFrameWidth = 3;
    double scale = static_cast<double>(SDF_DIM) / SDF_SHAPE_SIZE;
    scale = sdfScaleForShape(shapeDims, scale);

    int sdfWidth = static_cast<int>(std::ceil(scale * shapeDims.x));
    int sdfHeight = static_cast<int>(std::ceil(scale * shapeDims.y));

    sdfWidth += 2 * pixelFrameWidth;
    sdfHeight += 2 * pixelFrameWidth;

    msdfgen::Vector2 translate = { -bounds.l, -bounds.b };

    double pxRangeScaled = pixelFrameWidth / scale;

    double left = bounds.l - pxRangeScaled;
    double top = -bounds.t - pxRangeScaled;
    double width = shapeDims.x + pxRangeScaled * 2;
    double height = shapeDims.y + pxRangeScaled * 2;

    double range = pxRange / scale;
    translate += pxRangeScaled;

    auto sdf = msdfgen::Bitmap<float, 1>(sdfWidth, sdfHeight);
    msdfgen::SDFTransformation t(msdfgen::Projection(scale, translate), msdfgen::Range(range));
    msdfgen::generateSDF(sdf, shape, t);
    msdfgen::distanceSignCorrection(sdf, shape, t, msdfgen::FILL_NONZERO);

    out.sdf.bitmap.reserve(sdfWidth * sdfHeight);
    for (int y = 0; y < sdf.height(); ++y) {
        for (int x = 0; x < sdf.width(); ++x) {
            uint8_t px = static_cast<uint8_t>(msdfgen::pixelFloatToByte(*sdf(x, y)));
            out.sdf.bitmap.push_back(px);
        }
    }
    out.sdf.width = sdfWidth;
    out.sdf.height = sdfHeight;
    out.range = range;

    out.rect.setTop(top);
    out.rect.setLeft(left);
    out.rect.setWidth(width);
    out.rect.setHeight(height);

    out.sdf.hash = std::hash<std::string_view> {}({ reinterpret_cast<const char*>(out.sdf.bitmap.data()), out.sdf.bitmap.size() });
}

#endif

static f26dot6_t tableValueToF26Dot6(FT_Face face, long value, int pixelSize)
{
    if (!face || face->units_per_EM == 0) {
        return 0;
    }

    return static_cast<f26dot6_t>(value * pixelSize * 64.0 / face->units_per_EM);
}

static bool processHheaTable(FT_Face face, int pixelSize, HeightMetrics& metrics)
{
    TT_HoriHeader* hhea = (TT_HoriHeader*)FT_Get_Sfnt_Table(face, ft_sfnt_hhea);
    if (!hhea) {
        return false;
    }

    if (hhea->Ascender == 0 && hhea->Descender == 0) {
        return false;
    }

    metrics.ascent = tableValueToF26Dot6(face, hhea->Ascender, pixelSize);
    metrics.descent = -tableValueToF26Dot6(face, hhea->Descender, pixelSize);
    metrics.leading = tableValueToF26Dot6(face, hhea->Line_Gap, pixelSize);
    return true;
}

static bool processOS2Table(FT_Face face, int pixelSize, bool preferTypoLineMetrics, HeightMetrics& metrics)
{
    TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (!os2) {
        return false;
    }

    enum {
        USE_TYPO_METRICS = 0x80
    };
    if (preferTypoLineMetrics || (os2->fsSelection & USE_TYPO_METRICS)) {
        if (os2->sTypoAscender == 0 && os2->sTypoDescender == 0) {
            return false;
        }

        metrics.ascent = tableValueToF26Dot6(face, os2->sTypoAscender, pixelSize);
        metrics.descent = -tableValueToF26Dot6(face, os2->sTypoDescender, pixelSize);
        metrics.leading = tableValueToF26Dot6(face, os2->sTypoLineGap, pixelSize);
    } else {
        if (os2->usWinAscent == 0 && os2->usWinDescent == 0) {
            return false;
        }

        metrics.ascent = tableValueToF26Dot6(face, os2->usWinAscent, pixelSize);
        metrics.descent = tableValueToF26Dot6(face, os2->usWinDescent, pixelSize);
        metrics.leading = 0;
    }

    return true;
}

static HeightMetrics calculateHeightMetrics(FData* data, int pixelSize)
{
    const FT_Size_Metrics& metrics = data->metrics;
    const double scale = static_cast<double>(pixelSize) / metrics.y_ppem;

    HeightMetrics result;
    result.ascent = static_cast<f26dot6_t>(metrics.ascender * scale);
    result.descent = static_cast<f26dot6_t>(-metrics.descender * scale);
    result.leading = static_cast<f26dot6_t>((metrics.height - metrics.ascender + metrics.descender) * scale);

    processHheaTable(data->face, pixelSize, result);
    processOS2Table(data->face, pixelSize, true, result);
    return result;
}

static void initializeHeightMetrics(FData* data, int pixelSize)
{
    data->heightMetrics = calculateHeightMetrics(data, pixelSize);
}

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

bool FontFaceFT::load(const FaceKey& key, const FontData& fontData, bool isSymbolMode)
{
    if (!_init_ft()) {
        return false;
    }

    m_key = key;
    m_isSymbolMode = isSymbolMode;

    if (!fontData.valid()) {
        LOGE() << "empty font data: " << m_key.dataKey.family().id();
        return false;
    }

    m_data->fontData = fontData.data;

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
    initializeHeightMetrics(m_data, m_key.pixelSize);

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
            f26dot6_t xAdvance = static_cast<f26dot6_t>(pos[i].x_advance);
            hb_position_t hbAdvance = hb_font_get_glyph_h_advance(m_data->hb_font, info[i].codepoint);
            if (GlyphMetrics* metrics = glyphMetrics(info[i].codepoint)) {
                xAdvance += metrics->linearAdvance - hbAdvance;
            }

            result.push_back({ info[i].codepoint, xAdvance });
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
        FT_UInt index = static_cast<FT_UInt>(idx);
        if (index == 0) {
            return FBBox();
        }

        if (FT_Load_Glyph(m_data->face, index, FT_LOAD_NO_BITMAP) != 0) {
            return FBBox();
        }

        FT_GlyphSlot slot = m_data->face->glyph;
        if (slot->format != FT_GLYPH_FORMAT_OUTLINE) {
            return FBBox();
        }

        FT_BBox outlineBox;
        FT_Outline_Get_BBox(&slot->outline, &outlineBox);

        FBBox bbox;
        bbox.setCoords(outlineBox.xMin, -outlineBox.yMax, outlineBox.xMax, -outlineBox.yMin);
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

const GlyphImage& FontFaceFT::glyphImage(glyph_idx_t idx) const
{
    static GlyphImage null;

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
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

    msdfgen::Shape shape;
    msdfgen::readFreetypeOutline(shape, &m_data->face->glyph->outline);
    if (shape.contours.empty()) {
        return m_cache.insert({ idx, GlyphImage() }).first->second;
    }

    std::pair<glyph_idx_t, GlyphImage> v;
    v.first = idx;
    shape.inverseYAxis = true;
    generateSdf(v.second, shape);

    return m_cache.insert(std::move(v)).first->second;
#else
    UNUSED(idx);
    return null;
#endif
}

f26dot6_t FontFaceFT::leading() const
{
    return m_data->heightMetrics.leading;
}

f26dot6_t FontFaceFT::ascent() const
{
    return m_data->heightMetrics.ascent;
}

f26dot6_t FontFaceFT::descent() const
{
    return m_data->heightMetrics.descent;
}

f26dot6_t FontFaceFT::xHeight() const
{
    TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(m_data->face, ft_sfnt_os2);
    if (os2 && os2->sxHeight) {
        f26dot6_t result = std::round(os2->sxHeight * m_data->face->size->metrics.y_ppem * 64.0 / (double)m_data->face->units_per_EM);
        return result;
    }

    const glyph_idx_t glyph = glyphIndex('x');
    if (glyph == 0) {
        return 0;
    }

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

    const glyph_idx_t glyph = glyphIndex('X');
    if (glyph == 0) {
        return 0;
    }

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

    if (FT_Load_Glyph(m_data->face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING) != 0) {
        return nullptr;
    }

    FT_GlyphSlot slot = m_data->face->glyph;

    GlyphMetrics& gm = m_data->glyphsMetrics[idx];

    auto floorF26Dot6 = [](FT_Pos value) {
        return value & -64;
    };

    auto ceilF26Dot6 = [](FT_Pos value) {
        return (value + 63) & -64;
    };

    FT_Pos left = floorF26Dot6(slot->metrics.horiBearingX);
    FT_Pos right = ceilF26Dot6(slot->metrics.horiBearingX + slot->metrics.width);
    FT_Pos top = ceilF26Dot6(slot->metrics.horiBearingY);
    FT_Pos bottom = floorF26Dot6(slot->metrics.horiBearingY - slot->metrics.height);

    gm.bbox.setLeft(left);
    gm.bbox.setTop(-top);
    gm.bbox.setWidth(right - left);
    gm.bbox.setHeight(top - bottom);

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

    if (FT_Load_Glyph(m_data->face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING) != 0) {
        return nullptr;
    }

    SymbolMetrics& sm = m_data->symbolMetrics[idx];

    sm.idx = static_cast<glyph_idx_t>(index);

    FT_GlyphSlot slot = m_data->face->glyph;
    sm.bbox.xMin = slot->metrics.horiBearingX;
    sm.bbox.xMax = slot->metrics.horiBearingX + slot->metrics.width;
    sm.bbox.yMin = slot->metrics.horiBearingY - slot->metrics.height;
    sm.bbox.yMax = slot->metrics.horiBearingY;

    //! NOTE Moved form MSS FontEngineFT::advance
    //! double advance = linearHoriAdvance * dpi_f / 655360.0;
    //! -> f26dot6_t advance = linearHoriAdvance * dpi_f * 64.0 / 655360.0;
    //! -> dpi_f = 5.0 constant
    //! -> f26dot6_t advance = linearHoriAdvance * 320.0 / 655360.0;
    //! -> f26dot6_t advance = linearHoriAdvance / 2048;
    sm.linearAdvance = m_data->face->glyph->linearHoriAdvance / 2048;

    return &sm;
}
