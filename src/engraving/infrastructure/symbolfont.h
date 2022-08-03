/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_SYMBOLFONT_H
#define MU_ENGRAVING_SYMBOLFONT_H

#include <unordered_map>

#include "style/style.h"

#include "draw/types/geometry.h"

#include "modularity/ioc.h"
#include "draw/ifontprovider.h"
#include "io/path.h"

#include "smufl.h"

namespace mu {
class JsonObject;
}

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class SymbolFont
{
    INJECT_STATIC(score, mu::draw::IFontProvider, fontProvider)

public:
    SymbolFont(const String& name, const String& family, const io::path_t& filePath);
    SymbolFont(const SymbolFont& other);

    const String& name() const;
    const String& family() const;
    const io::path_t& fontPath() const;

    std::unordered_map<Sid, PropertyValue> engravingDefaults();
    double textEnclosureThickness();

    char32_t symCode(SymId id) const;
    SymId fromCode(char32_t code) const;
    String toString(SymId id) const;

    bool isValid(SymId id) const;
    bool useFallbackFont(SymId id) const;

    const mu::RectF bbox(SymId id, double mag) const;
    const mu::RectF bbox(SymId id, const mu::SizeF&) const;
    const mu::RectF bbox(const SymIdList& s, double mag) const;
    const mu::RectF bbox(const SymIdList& s, const mu::SizeF& mag) const;

    double width(SymId id, double mag) const;
    double height(SymId id, double mag) const;
    double advance(SymId id, double mag) const;
    double width(const SymIdList&, double mag) const;

    mu::PointF smuflAnchor(SymId symId, SmuflAnchorId anchorId, double mag) const;

    void draw(SymId id,         mu::draw::Painter*, double mag,            const mu::PointF& pos) const;
    void draw(SymId id,         mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos) const;
    void draw(SymId id,         mu::draw::Painter*, double mag,            const mu::PointF& pos, int n) const;
    void draw(const SymIdList&, mu::draw::Painter*, double mag,            const mu::PointF& pos) const;
    void draw(const SymIdList&, mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos) const;

private:

    friend class SymbolFonts;

    struct Sym {
        char32_t code;
        mu::RectF bbox;
        double advance = 0.0;

        std::map<SmuflAnchorId, mu::PointF> smuflAnchors;
        SymIdList subSymbolIds;

        bool isValid() const
        {
            return code != 0 && bbox.isValid();
        }

        bool isCompound() const
        {
            return !subSymbolIds.empty();
        }
    };

    void load();
    void loadGlyphsWithAnchors(const JsonObject& glyphsWithAnchors);
    void loadComposedGlyphs();
    void loadStylisticAlternates(const JsonObject& glyphsWithAlternatesObject);
    void loadEngravingDefaults(const JsonObject& engravingDefaultsObject);
    void computeMetrics(Sym& sym, const Smufl::Code& code);

    Sym& sym(SymId id);
    const Sym& sym(SymId id) const;

    bool m_loaded = false;
    std::vector<Sym> m_symbols;
    mutable draw::Font m_font;

    String m_name;
    String m_family;
    io::path_t m_fontPath;

    std::unordered_map<Sid, PropertyValue> m_engravingDefaults;
    double m_textEnclosureThickness = 0;
};
}

#endif // MU_ENGRAVING_SYMBOLFONT_H
