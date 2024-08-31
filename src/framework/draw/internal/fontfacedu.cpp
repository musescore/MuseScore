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
#include "fontfacedu.h"

using namespace muse::draw;

struct DummyGlyph {
    FBBox textBbox;
    f26dot6_t textAdvance = 0;
    FBBox symBbox;
    f26dot6_t symAdvance = 0;
#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
    msdfgen::Shape shape;
#endif
};

static const DummyGlyph& dummyGlyph()
{
    static DummyGlyph g;
    if (g.textAdvance == 0) {
        g.textBbox = FBBox(0, -8064, 4160, 9728);
        g.textAdvance = 4160;
        g.symBbox = FBBox(0, -4011, 2079, 4817);
        g.symAdvance = 2080;

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
        using namespace msdfgen;

        g.shape.inverseYAxis = true;
        g.shape.fillRule = static_cast<msdfgen::FillRule>(msdfgen::FillRule::NonZero);

        Contour c1;
        {
            std::vector<Point2> points = {
                Point2(0, -25), Point2(64, -25),
                Point2(64, -25), Point2(64, 125),
                Point2(64, 125), Point2(0, 125),
                Point2(0, 125), Point2(0, -25)
            };

            for (size_t i = 0; i < 8;) {
                EdgeSegment e;
                e.actualType = EdgeSegment::ActualType::Linear;
                e.segments.linear.p[0] = points.at(i++);
                e.segments.linear.p[1] = points.at(i++);
                c1.edges.push_back(e);
            }
        }
        g.shape.contours.push_back(c1);

        Contour c2;
        {
            std::vector<Point2> points = {
                Point2(9, 115), Point2(54, 115),
                Point2(54, 115), Point2(54, -15),
                Point2(54, -15), Point2(9, -15),
                Point2(9, -15), Point2(9, 115)
            };

            for (size_t i = 0; i < 8;) {
                EdgeSegment e;
                e.actualType = EdgeSegment::ActualType::Linear;
                e.segments.linear.p[0] = points.at(i++);
                e.segments.linear.p[1] = points.at(i++);
                c2.edges.push_back(e);
            }
        }
        g.shape.contours.push_back(c2);
#endif
    }

    return g;
}

FontFaceDU::FontFaceDU(IFontFace* origin)
    : m_origin(origin)
{
}

FontFaceDU::~FontFaceDU()
{
    delete m_origin;
}

bool FontFaceDU::load(const FaceKey& key, const io::path_t& path, bool isSymbolMode)
{
    return m_origin->load(key, path, isSymbolMode);
}

const FaceKey& FontFaceDU::key() const
{
    return m_origin->key();
}

bool FontFaceDU::isSymbolMode() const
{
    return m_origin->isSymbolMode();
}

f26dot6_t FontFaceDU::leading() const
{
    return m_origin->leading();
}

f26dot6_t FontFaceDU::ascent() const
{
    return m_origin->ascent();
}

f26dot6_t FontFaceDU::descent() const
{
    return m_origin->descent();
}

f26dot6_t FontFaceDU::xHeight() const
{
    return m_origin->xHeight();
}

f26dot6_t FontFaceDU::capHeight() const
{
    return m_origin->capHeight();
}

std::vector<GlyphPos> FontFaceDU::glyphs(const char32_t* text, int text_length) const
{
    std::vector<GlyphPos> glyphs = m_origin->glyphs(text, text_length);
    std::vector<GlyphPos> ret;
    ret.reserve(glyphs.size());
    for (GlyphPos& gp : glyphs) {
        if (gp.idx == 0) {
            gp.x_advance = glyphAdvance(0); // dummy
        }
        ret.push_back(gp);
    }
    return ret;
}

glyph_idx_t FontFaceDU::glyphIndex(char32_t ucs4) const
{
    return m_origin->glyphIndex(ucs4);
}

glyph_idx_t FontFaceDU::glyphIndex(const std::string& glyphName) const
{
    return m_origin->glyphIndex(glyphName);
}

char32_t FontFaceDU::findCharCode(glyph_idx_t idx) const
{
    return m_origin->findCharCode(idx);
}

FBBox FontFaceDU::glyphBbox(glyph_idx_t idx) const
{
    if (idx == 0) {
        return m_origin->isSymbolMode() ? dummyGlyph().symBbox : dummyGlyph().textBbox;
    }
    return m_origin->glyphBbox(idx);
}

f26dot6_t FontFaceDU::glyphAdvance(glyph_idx_t idx) const
{
    if (idx == 0) {
        return m_origin->isSymbolMode() ? dummyGlyph().symAdvance : dummyGlyph().textAdvance;
    }
    return m_origin->glyphAdvance(idx);
}

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
const msdfgen::Shape& FontFaceDU::glyphShape(glyph_idx_t idx) const
{
    if (idx == 0) {
        return dummyGlyph().shape;
    }
    return m_origin->glyphShape(idx);
}

#endif
