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
#ifndef MS_SCOREFONT_H
#define MS_SCOREFONT_H

#include "style/style.h"

#include "infrastructure/draw/geometry.h"

#include "modularity/ioc.h"
#include "infrastructure/draw/ifontprovider.h"

namespace mu::draw {
class Painter;
}

namespace Ms {
class ScoreFont
{
    INJECT_STATIC(score, mu::draw::IFontProvider, fontProvider)

public:
    ScoreFont(const char* name, const char* family, const char* path, const char* filename);
    ScoreFont(const ScoreFont& other);

    const QString& name() const;
    const QString& family() const;
    const QString& fontPath() const;

    std::list<std::pair<Sid, QVariant> > engravingDefaults();
    double textEnclosureThickness();

    static void initScoreFonts();
    static const std::vector<ScoreFont>& scoreFonts();
    static ScoreFont* fontByName(const QString& name);
    static ScoreFont* fallbackFont();
    static const char* fallbackTextFont();

    uint symCode(SymId id) const;
    SymId fromCode(uint code) const;
    QString toString(SymId id) const;

    bool isValid(SymId id) const;
    bool useFallbackFont(SymId id) const;

    const mu::RectF bbox(SymId id, qreal mag) const;
    const mu::RectF bbox(SymId id, const mu::SizeF&) const;
    const mu::RectF bbox(const SymIdList& s, qreal mag) const;
    const mu::RectF bbox(const SymIdList& s, const mu::SizeF& mag) const;

    qreal width(SymId id, qreal mag) const;
    qreal height(SymId id, qreal mag) const;
    qreal advance(SymId id, qreal mag) const;
    qreal width(const SymIdList&, qreal mag) const;

    mu::PointF smuflAnchor(SymId symId, SmuflAnchorId anchorId, qreal mag) const;

    void draw(SymId id,         mu::draw::Painter*, qreal mag,            const mu::PointF& pos) const;
    void draw(SymId id,         mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos) const;
    void draw(SymId id,         mu::draw::Painter*, qreal mag,            const mu::PointF& pos, int n) const;
    void draw(const SymIdList&, mu::draw::Painter*, qreal mag,            const mu::PointF& pos) const;
    void draw(const SymIdList&, mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos) const;

private:
    struct Sym {
        uint code = 0;
        mu::RectF bbox;
        qreal advance = 0.0;

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

    static QJsonObject initGlyphNamesJson();

    void load();
    void loadGlyphsWithAnchors(const QJsonObject& glyphsWithAnchors);
    void loadComposedGlyphs();
    void loadStylisticAlternates(const QJsonObject& glyphsWithAlternatesObject);
    void loadEngravingDefaults(const QJsonObject& engravingDefaultsObject);
    void computeMetrics(Sym& sym, uint code);

    Sym& sym(SymId id);
    const Sym& sym(SymId id) const;

    bool m_loaded = false;
    std::vector<Sym> m_symbols;
    mutable mu::draw::Font m_font;

    QString m_name;
    QString m_family;
    QString m_fontPath;
    QString m_filename;

    std::list<std::pair<Sid, QVariant> > m_engravingDefaults;
    double m_textEnclosureThickness = 0;

    static std::vector<ScoreFont> s_scoreFonts;
    static std::array<uint, size_t(SymId::lastSym) + 1> s_symIdCodes;
};
}

#endif // MS_SCOREFONT_H
