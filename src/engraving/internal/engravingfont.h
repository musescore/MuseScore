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
#ifndef MU_ENGRAVING_ENGRAVINGFONT_H
#define MU_ENGRAVING_ENGRAVINGFONT_H

#include <unordered_map>

#include "iengravingfont.h"
#include "modularity/ioc.h"
#include "draw/ifontprovider.h"
#include "draw/types/geometry.h"
#include "iengravingfontsprovider.h"

#include "io/path.h"

#include "smufl.h"

#include "style/styledef.h"
#include "types/symid.h"

namespace mu {
class JsonObject;
}

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class Shape;

class EngravingFont : public IEngravingFont
{
    INJECT_STATIC(mu::draw::IFontProvider, fontProvider)
    INJECT_STATIC(IEngravingFontsProvider, engravingFonts)
public:
    EngravingFont(const std::string& name, const std::string& family, const io::path_t& filePath);
    EngravingFont(const EngravingFont& other);

    const std::string& name() const override;
    const std::string& family() const override;

    std::unordered_map<Sid, PropertyValue> engravingDefaults() const override;
    double textEnclosureThickness();

    char32_t symCode(SymId id) const override;
    SymId fromCode(char32_t code) const override;
    String toString(SymId id) const override;

    bool isValid(SymId id) const override;

    RectF bbox(SymId id, double mag) const override;
    RectF bbox(SymId id, const mu::SizeF&) const override;
    RectF bbox(const SymIdList& s, double mag) const override;
    RectF bbox(const SymIdList& s, const mu::SizeF& mag) const override;
    Shape shape(const SymIdList& s, double mag) const override;
    Shape shape(const SymIdList& s, const mu::SizeF& mag) const override;

    double width(SymId id, double mag) const override;
    double width(const SymIdList&, double mag) const override;
    double height(SymId id, double mag) const override;
    double advance(SymId id, double mag) const override;

    PointF smuflAnchor(SymId symId, SmuflAnchorId anchorId, double mag) const override;

    // Draw
    void draw(SymId id, draw::Painter* p, double mag, const PointF& pos) const override;
    void draw(SymId id, draw::Painter* p, const SizeF& mag, const PointF& pos) const override;

    void draw(const SymIdList& ids, draw::Painter* p, double mag, const PointF& pos) const override;
    void draw(const SymIdList& ids, draw::Painter* p, const SizeF& mag, const PointF& pos) const override;

    void ensureLoad();

private:

    friend class SymbolFonts;

    struct Sym {
        char32_t code;
        RectF bbox;
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

    void loadGlyphsWithAnchors(const JsonObject& glyphsWithAnchors);
    void loadComposedGlyphs();
    void loadStylisticAlternates(const JsonObject& glyphsWithAlternatesObject);
    void loadEngravingDefaults(const JsonObject& engravingDefaultsObject);
    void computeMetrics(Sym& sym, const Smufl::Code& code);

    Sym& sym(SymId id);
    const Sym& sym(SymId id) const;

    bool useFallbackFont(SymId id) const;

    bool m_loaded = false;
    std::vector<Sym> m_symbols;
    mutable draw::Font m_font;

    std::string m_name;
    std::string m_family;
    io::path_t m_fontPath;

    std::unordered_map<Sid, PropertyValue> m_engravingDefaults;
    double m_textEnclosureThickness = 0;
};
}

#endif // MU_ENGRAVING_ENGRAVINGFONT_H
