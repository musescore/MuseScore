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
#pragma once

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
#include <msdfgen.h>
#endif

#include "global/io/path.h"
#include "types/fontstypes.h"

namespace muse::draw {
using f26dot6_t = long;         // A signed 26.6 fixed-point type used for vectorial pixel coordinates.

inline long to_f26d6(float v) { return static_cast<long>(v * 64); }
inline double from_f26d6(f26dot6_t v) { return static_cast<double>(v) / 64.0; }

using FBBox = RectX<f26dot6_t>;

struct GlyphPos {
    glyph_idx_t idx = 0;
    f26dot6_t x_advance = 0.;
};

class IFontFace
{
public:

    virtual ~IFontFace() = default;

    virtual bool load(const FaceKey& key, const io::path_t& path, bool isSymbolMode) = 0;

    virtual const FaceKey& key() const = 0;
    virtual bool isSymbolMode() const = 0;

    virtual f26dot6_t leading() const = 0;
    virtual f26dot6_t ascent() const = 0;
    virtual f26dot6_t descent() const = 0;
    virtual f26dot6_t xHeight() const = 0;
    virtual f26dot6_t capHeight() const = 0;

    virtual std::vector<GlyphPos> glyphs(const char32_t* text, int text_length) const = 0;
    virtual glyph_idx_t glyphIndex(char32_t ucs4) const = 0;
    virtual glyph_idx_t glyphIndex(const std::string& glyphName) const = 0;
    virtual char32_t findCharCode(glyph_idx_t idx) const = 0; // for tests

    virtual FBBox glyphBbox(glyph_idx_t idx) const = 0;
    virtual f26dot6_t glyphAdvance(glyph_idx_t idx) const = 0;

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
    virtual const msdfgen::Shape& glyphShape(glyph_idx_t idx) const = 0;
#endif
};
}

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
inline bool operator==(const msdfgen::Shape& s1, const msdfgen::Shape& s2)
{
    if (s1.inverseYAxis != s2.inverseYAxis) {
        return false;
    }

    if (s1.fillRule != s2.fillRule) {
        return false;
    }

    if (s1.contours.size() != s2.contours.size()) {
        return false;
    } else {
        auto pointsIsEqual = [](const msdfgen::Point2* p1, const msdfgen::Point2* p2, int count) {
            for (int i = 0; i < count; ++i) {
                if (p1[i] != p2[i]) {
                    return false;
                }
            }
            return true;
        };

        for (size_t ci = 0; ci < s1.contours.size(); ++ci) {
            const msdfgen::Contour& c1 = s1.contours.at(ci);
            const msdfgen::Contour& c2 = s2.contours.at(ci);
            if (c1.edges.size() != c2.edges.size()) {
                return false;
            } else {
                for (size_t ei = 0; ei < c1.edges.size(); ++ei) {
                    const msdfgen::EdgeSegment& e1 = c1.edges.at(ei);
                    const msdfgen::EdgeSegment& e2 = c2.edges.at(ei);
                    if (e1.actualType != e2.actualType) {
                        return false;
                    } else {
                        switch (e1.actualType) {
                        case msdfgen::EdgeSegment::ActualType::Undefined: {
                            continue;
                        } break;
                        case msdfgen::EdgeSegment::ActualType::Linear: {
                            if (!pointsIsEqual(e1.segments.linear.p, e2.segments.linear.p, 2)) {
                                return false;
                            }
                        } break;
                        case msdfgen::EdgeSegment::ActualType::Quadratic: {
                            if (!pointsIsEqual(e1.segments.quadratic.p, e2.segments.quadratic.p, 3)) {
                                return false;
                            }
                        } break;
                        case msdfgen::EdgeSegment::ActualType::Cubic: {
                            if (!pointsIsEqual(e1.segments.cubic.p, e2.segments.cubic.p, 4)) {
                                return false;
                            }
                        } break;
                        }
                    }
                }
            }
        }
    }

    return true;
}

#endif
