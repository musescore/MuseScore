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

#include "draw/geometry.h"

#include "modularity/ioc.h"
#include "draw/ifontprovider.h"

#include "symid.h"

namespace mu::draw {
class Painter;
}

namespace Ms {
enum class SmuflAnchorId;
class Sym;

//---------------------------------------------------------
//   ScoreFont
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------
class ScoreFont
{
    INJECT(score, mu::draw::IFontProvider, fontProvider)

public:
    ScoreFont() = default;
    ScoreFont(const char* name, const char* family, const char* path, const char* filename);
    ScoreFont(const ScoreFont& other);

    const QString& name() const;
    const QString& family() const;
    QString fontPath() const;

    std::list<std::pair<Sid, QVariant> > engravingDefaults();
    double textEnclosureThickness();

    static void initScoreFonts();
    static const std::vector<ScoreFont>& scoreFonts();
    static ScoreFont* fontByName(const QString& name);
    static ScoreFont* fallbackFont();
    static const char* fallbackTextFont();

    Sym sym(SymId id) const;
    uint symCode(SymId id) const;
    QString toString(SymId id) const;

    bool isValid(SymId id) const;
    bool useFallbackFont(SymId id) const;

    const mu::RectF bbox(SymId id, qreal mag) const;
    const mu::RectF bbox(SymId id, const mu::SizeF&) const;
    const mu::RectF bbox(const std::vector<SymId>& s, qreal mag) const;
    const mu::RectF bbox(const std::vector<SymId>& s, const mu::SizeF& mag) const;

    qreal width(SymId id, qreal mag) const;
    qreal height(SymId id, qreal mag) const;
    qreal advance(SymId id, qreal mag) const;
    qreal width(const std::vector<SymId>&, qreal mag) const;

    mu::PointF smuflAnchor(SymId symId, SmuflAnchorId anchorId, qreal mag) const;

    void draw(SymId id,                  mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos, qreal scale) const;
    void draw(SymId id,                  mu::draw::Painter*, qreal mag,         const mu::PointF& pos, qreal scale) const;
    void draw(SymId id,                  mu::draw::Painter*, qreal mag,         const mu::PointF& pos) const;
    void draw(SymId id,                  mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos) const;
    void draw(SymId id,                  mu::draw::Painter*, qreal mag,         const mu::PointF& pos, int n) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, qreal mag,         const mu::PointF& pos) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, qreal mag,         const mu::PointF& pos, qreal scale) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, const mu::SizeF& mag, const mu::PointF& pos, qreal scale) const;

private:
    static QJsonObject initGlyphNamesJson();

    void load();
    void loadGlyphsWithAnchors(const QJsonObject& glyphsWithAnchors);
    void loadComposedGlyphs();
    void loadStylisticAlternates(const QJsonObject& glyphsWithAlternatesObject);
    void loadEngravingDefaults(const QJsonObject& engravingDefaultsObject);
    void computeMetrics(Sym* sym, int code);

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
    static std::array<uint, size_t(SymId::lastSym) + 1> s_mainSymCodeTable;
};
}

#endif // MS_SCOREFONT_H
