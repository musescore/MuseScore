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

#include "ifontface.h"

namespace muse::draw {
struct FData;
struct GlyphMetrics;
struct SymbolMetrics;
class FontFaceFT : public IFontFace
{
public:

    FontFaceFT();
    ~FontFaceFT();

    bool load(const FaceKey& key, const io::path_t& path, bool isSymbolMode) override;

    const FaceKey& key() const override;
    bool isSymbolMode() const override;

    f26dot6_t leading() const override;
    f26dot6_t ascent() const override;
    f26dot6_t descent() const override;
    f26dot6_t xHeight() const override;
    f26dot6_t capHeight() const override;

    std::vector<GlyphPos> glyphs(const char32_t* text, int text_length) const override;
    glyph_idx_t glyphIndex(char32_t ucs4) const override;
    glyph_idx_t glyphIndex(const std::string& glyphName) const override;
    char32_t findCharCode(glyph_idx_t idx) const override;

    FBBox glyphBbox(glyph_idx_t idx) const override;
    f26dot6_t glyphAdvance(glyph_idx_t idx) const override;

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
    const msdfgen::Shape& glyphShape(glyph_idx_t idx) const override;
#endif

private:

    GlyphMetrics* glyphMetrics(glyph_idx_t idx) const;
    SymbolMetrics* symbolMetrics(glyph_idx_t idx) const;

    FaceKey m_key;
    bool m_isSymbolMode = false;
    FData* m_data = nullptr;
#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
    mutable std::unordered_map<glyph_idx_t, msdfgen::Shape> m_cache;
#endif
};
}
