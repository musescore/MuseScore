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
#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "global/io/iodevice.h"
#include "global/io/buffer.h"

#include "ifontface.h"

namespace muse {
class ZipReader;
}

namespace muse::draw {
class AtlasTexture
{
public:
    AtlasTexture() = default;
    AtlasTexture(AtlasTexture&& o) = default;

    bool loadAtlas(const muse::ByteArray& atlas);
    void loadAtlasMetaInfo(const muse::ByteArray& atlas);
    Sdf getGlyphImage(glyph_idx_t idx);

private:
    struct GlyphInfo {
        glyph_idx_t gidx;
        muse::Rect rect;
        bool rotated = false;
    };

    std::map<glyph_idx_t, GlyphInfo> m_glyphInfos;
    std::vector<uint8_t> m_atlas;
    uint32_t m_atlasWidth = 0;
    uint32_t m_atlasHeight = 0;
};

class FontFaceXT : public IFontFace
{
public:
    FontFaceXT();
    ~FontFaceXT();

    struct GlyphData {
        GlyphData() = default;
        GlyphData(GlyphData&& o) = default;
        FBBox textBbox;
        f26dot6_t textAdvance = 0;
        FBBox symBbox;
        f26dot6_t symAdvance = 0;
        GlyphImage glyphImage;

        void write(muse::io::IODevice* d) const;
        void read(muse::io::IODevice* d);
    };

    bool load(const FaceKey& key, const FontData& fontData, bool isSymbolMode) override;

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
    const GlyphImage& glyphImage(glyph_idx_t idx) const override;

    using Ligature = std::pair<glyph_idx_t, std::vector<glyph_idx_t> >;
    using Ligatures = std::vector<Ligature>;

    using Kernings = std::map<std::pair<glyph_idx_t, glyph_idx_t>, f26dot6_t>;

    static void applyLigatures(std::vector<glyph_idx_t>& glyphs, const Ligatures& ls);

private:

    const GlyphData& glyphData(glyph_idx_t idx) const;
    f26dot6_t glyphAdvance(glyph_idx_t idx, glyph_idx_t nextIdx) const;

    FaceKey m_key;
    bool m_isSymbolMode = false;

    std::unique_ptr<muse::io::Buffer> m_fileBuffer;
    std::unique_ptr<muse::ZipReader> m_zip;
    f26dot6_t m_leading = -1;
    f26dot6_t m_ascent = -1;
    f26dot6_t m_descent = -1;
    f26dot6_t m_xHeight = -1;
    f26dot6_t m_capHeight = -1;

    Ligatures m_ligatures;
    Kernings m_kernings;

    mutable std::unordered_map<std::string, glyph_idx_t> m_nonUnicodeGlyphs;
    mutable std::unordered_map<char32_t, glyph_idx_t> m_unicodeGlyphs;
    mutable std::unordered_map<glyph_idx_t, GlyphData> m_cache;
    mutable AtlasTexture m_atlastexture;
};
}
