/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "draw/internal/fontfaceft.h"
#include "draw/internal/fontfacext.h"

using namespace muse;
using namespace muse::draw;

class Draw_FontFaceXTTests : public ::testing::Test
{
public:
};

static constexpr int TEST_PIXEL_SIZE = 200;
static constexpr f26dot6_t ONE_PIXEL = 64;
static constexpr f26dot6_t TWO_PIXELS = 2 * ONE_PIXEL;
static constexpr f26dot6_t ADVANCE_EPSILON = 2;
static const std::vector<char32_t> TEST_CHARS = {
    U'a', U'e', U'g', U'm', U'o', U'p', U'q', U'r', U's', U'v',
    U'w', U'x', U'y', U'z', U' ', U'.', U',', U'-', U'\''
};

static const std::vector<std::u32string> TEST_STRINGS = {
    U"qwer",
    U"musescore",
    U"musegroup",
    U"getmedrink",
    U"Corno in G.",
    U"AW"
};

static io::path_t fontRoot()
{
    return MUSE_DRAW_TEST_FONTS_DIR;
}

static io::path_t edwinFtPath()
{
    return fontRoot() + "/edwin/Edwin-Roman.otf";
}

static io::path_t edwinXtPath()
{
    return fontRoot() + "/edwin/Edwin-Roman.ftx";
}

static FaceKey edwinFaceKey()
{
    return FaceKey(FontDataKey(u"Edwin"), Font::Type::Text, TEST_PIXEL_SIZE);
}

static bool valueIsNear(f26dot6_t v1, f26dot6_t v2, f26dot6_t epsilon)
{
    return std::abs(v1 - v2) <= epsilon;
}

static bool bboxIsNear(const FBBox& b1, const FBBox& b2, f26dot6_t epsilon)
{
    return valueIsNear(b1.x(), b2.x(), epsilon)
           && valueIsNear(b1.y(), b2.y(), epsilon)
           && valueIsNear(b1.width(), b2.width(), epsilon)
           && valueIsNear(b1.height(), b2.height(), epsilon);
}

static std::string bboxToString(const FBBox& bbox)
{
    return "x: " + std::to_string(bbox.x())
           + " y: " + std::to_string(bbox.y())
           + " w: " + std::to_string(bbox.width())
           + " h: " + std::to_string(bbox.height());
}

static f26dot6_t textAdvance(const IFontFace& face, const std::u32string& text)
{
    f26dot6_t result = 0;
    const std::vector<GlyphPos> glyphs = face.glyphs(text.data(), static_cast<int>(text.size()));
    for (const GlyphPos& glyph : glyphs) {
        result += glyph.x_advance;
    }

    return result;
}

static FBBox textBbox(const IFontFace& face, const std::u32string& text)
{
    FBBox result;
    f26dot6_t xOffset = 0;
    f26dot6_t xmax = 0;
    f26dot6_t ymax = 0;
    bool hasGlyph = false;

    const std::vector<GlyphPos> glyphs = face.glyphs(text.data(), static_cast<int>(text.size()));
    for (const GlyphPos& glyph : glyphs) {
        FBBox glyphBbox = face.glyphBbox(glyph.idx);
        f26dot6_t x = xOffset + glyphBbox.x();
        f26dot6_t y = glyphBbox.y();
        f26dot6_t glyphRight = x + glyphBbox.width();
        f26dot6_t glyphBottom = y + glyphBbox.height();

        if (hasGlyph) {
            result.setX(std::min(result.x(), x));
            result.setY(std::min(result.y(), y));
            xmax = std::max(xmax, glyphRight);
            ymax = std::max(ymax, glyphBottom);
        } else {
            result.setX(x);
            result.setY(y);
            xmax = glyphRight;
            ymax = glyphBottom;
            hasGlyph = true;
        }

        xOffset += glyph.x_advance;
    }

    if (hasGlyph) {
        result.setWidth(xmax - result.x());
        result.setHeight(ymax - result.y());
    }

    return result;
}

struct FontFaces {
    FontFaceFT ft;
    FontFaceXT xt;
};

static std::unique_ptr<FontFaces> loadEdwinFaces()
{
    std::unique_ptr<FontFaces> faces = std::make_unique<FontFaces>();
    const FaceKey key = edwinFaceKey();

    EXPECT_TRUE(faces->ft.load(key, edwinFtPath(), false));
    EXPECT_TRUE(faces->xt.load(key, edwinXtPath(), false));

    return faces;
}

TEST_F(Draw_FontFaceXTTests, TextMetrics)
{
    std::unique_ptr<FontFaces> faces = loadEdwinFaces();

    EXPECT_EQ(faces->ft.key(), faces->xt.key());
    EXPECT_EQ(faces->ft.isSymbolMode(), faces->xt.isSymbolMode());

    EXPECT_TRUE(valueIsNear(faces->ft.leading(), faces->xt.leading(), ONE_PIXEL));
    EXPECT_TRUE(valueIsNear(faces->ft.ascent(), faces->xt.ascent(), ONE_PIXEL));
    EXPECT_TRUE(valueIsNear(faces->ft.descent(), faces->xt.descent(), ONE_PIXEL));
    EXPECT_EQ(faces->ft.xHeight(), faces->xt.xHeight());
    if (faces->xt.capHeight() >= 0) {
        EXPECT_TRUE(valueIsNear(faces->ft.capHeight(), faces->xt.capHeight(), ONE_PIXEL));
    }
}

TEST_F(Draw_FontFaceXTTests, GlyphMetrics)
{
    std::unique_ptr<FontFaces> faces = loadEdwinFaces();

    for (char32_t ch : TEST_CHARS) {
        glyph_idx_t ftGlyph = faces->ft.glyphIndex(ch);
        glyph_idx_t xtGlyph = faces->xt.glyphIndex(ch);

        ASSERT_NE(ftGlyph, 0) << static_cast<uint32_t>(ch);
        ASSERT_NE(xtGlyph, 0) << static_cast<uint32_t>(ch);

        EXPECT_EQ(ftGlyph, xtGlyph) << static_cast<uint32_t>(ch);
        EXPECT_EQ(faces->ft.findCharCode(ftGlyph), faces->xt.findCharCode(xtGlyph));
        EXPECT_TRUE(valueIsNear(faces->ft.glyphAdvance(ftGlyph), faces->xt.glyphAdvance(xtGlyph), ADVANCE_EPSILON))
            << static_cast<uint32_t>(ch);
        const FBBox ftBbox = faces->ft.glyphBbox(ftGlyph);
        const FBBox xtBbox = faces->xt.glyphBbox(xtGlyph);
        EXPECT_TRUE(bboxIsNear(ftBbox, xtBbox, TWO_PIXELS))
            << static_cast<uint32_t>(ch)
            << "\nft: " << bboxToString(ftBbox)
            << "\nxt: " << bboxToString(xtBbox);
    }
}

TEST_F(Draw_FontFaceXTTests, Glyphs)
{
    std::unique_ptr<FontFaces> faces = loadEdwinFaces();

    for (const std::u32string& text : TEST_STRINGS) {
        std::vector<GlyphPos> ftGlyphs = faces->ft.glyphs(text.data(), static_cast<int>(text.size()));
        std::vector<GlyphPos> xtGlyphs = faces->xt.glyphs(text.data(), static_cast<int>(text.size()));

        ASSERT_EQ(ftGlyphs.size(), xtGlyphs.size());

        for (size_t i = 0; i < ftGlyphs.size(); ++i) {
            EXPECT_EQ(ftGlyphs[i].idx, xtGlyphs[i].idx);
            EXPECT_TRUE(valueIsNear(ftGlyphs[i].x_advance, xtGlyphs[i].x_advance, ADVANCE_EPSILON));
        }
    }
}

TEST_F(Draw_FontFaceXTTests, StringAdvance)
{
    std::unique_ptr<FontFaces> faces = loadEdwinFaces();

    for (const std::u32string& text : TEST_STRINGS) {
        const f26dot6_t ftAdvance = textAdvance(faces->ft, text);
        const f26dot6_t xtAdvance = textAdvance(faces->xt, text);

        EXPECT_TRUE(valueIsNear(ftAdvance, xtAdvance, ADVANCE_EPSILON));
    }
}

TEST_F(Draw_FontFaceXTTests, StringBbox)
{
    std::unique_ptr<FontFaces> faces = loadEdwinFaces();

    for (const std::u32string& text : TEST_STRINGS) {
        const FBBox ftBbox = textBbox(faces->ft, text);
        const FBBox xtBbox = textBbox(faces->xt, text);

        EXPECT_TRUE(bboxIsNear(ftBbox, xtBbox, TWO_PIXELS))
            << "\nft: " << bboxToString(ftBbox)
            << "\nxt: " << bboxToString(xtBbox);
    }
}

TEST_F(Draw_FontFaceXTTests, GlyphImage)
{
    std::unique_ptr<FontFaces> faces = loadEdwinFaces();

    for (char32_t ch : TEST_CHARS) {
        if (ch == U' ') {
            continue;
        }

        glyph_idx_t glyph = faces->xt.glyphIndex(ch);
        ASSERT_NE(glyph, 0) << static_cast<uint32_t>(ch);

        const GlyphImage& image = faces->xt.glyphImage(glyph);
        EXPECT_FALSE(image.isNull()) << static_cast<uint32_t>(ch);
        EXPECT_FALSE(image.sdf.bitmap.empty()) << static_cast<uint32_t>(ch);
        EXPECT_GT(image.sdf.width, 0u) << static_cast<uint32_t>(ch);
        EXPECT_GT(image.sdf.height, 0u) << static_cast<uint32_t>(ch);
        EXPECT_GT(image.range, 0.f) << static_cast<uint32_t>(ch);
    }
}
