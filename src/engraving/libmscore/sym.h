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

#ifndef __SYM_H__
#define __SYM_H__

#include <QApplication>

#include "config.h"
#include "style.h"
#include "qtenum.h"
#include "symid.h"

#include "draw/painter.h"
#include "draw/font.h"

#include "modularity/ioc.h"
#include "draw/ifontprovider.h"

namespace Ms {
//---------------------------------------------------------
//   SmuflAnchorId
//---------------------------------------------------------

enum class SmuflAnchorId {
    stemDownNW,
    stemUpSE,
    stemDownSW,
    stemUpNW,
    cutOutNE,
    cutOutNW,
    cutOutSE,
    cutOutSW,
};

//---------------------------------------------------------
//   Sym
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class Sym
{
protected:
    int _code = -1;
    QRectF _bbox;
    qreal _advance = 0.0;

    std::map<SmuflAnchorId, QPointF> smuflAnchors;
    std::vector<SymId> _ids; // not empty if this is a compound symbol

public:
    Sym() { }

    bool isValid() const { return _code != -1 && _bbox.isValid(); }

    void setSymList(const std::vector<SymId>& sl) { _ids = sl; }
    const std::vector<SymId>& symList() const { return _ids; }

    int code() const { return _code; }
    void setCode(int val) { _code = val; }

    QRectF bbox() const { return _bbox; }
    void setBbox(QRectF val) { _bbox = val; }

    qreal advance() const { return _advance; }
    void setAdvance(qreal val) { _advance = val; }

    QPointF smuflAnchor(SmuflAnchorId anchorId) { return smuflAnchors[anchorId]; }
    void setSmuflAnchor(SmuflAnchorId anchorId, const QPointF& newValue) { smuflAnchors[anchorId] = newValue; }

    static SymId name2id(const QString& s) { return lnhash.value(s, SymId::noSym); }           // return noSym if not found
    static SymId oldName2id(const QString s) { return lonhash.value(s, SymId::noSym); }
    static const char* id2name(SymId id);

    static QString id2userName(SymId id) { return qApp->translate("symUserNames", symUserNames[int(id)]); }
    static SymId userName2id(const QString& s);

    static const std::array<const char*, int(SymId::lastSym) + 1> symNames;
    static const std::array<const char*, int(SymId::lastSym) + 1> symUserNames;
    static const QVector<SymId> commonScoreSymbols;

    static QHash<QString, SymId> lnhash;

    struct OldName {
        const char* name;
        SymId symId;
    };
    static QHash<QString, SymId> lonhash;
    static QVector<OldName> oldNames;
    friend class ScoreFont;
};

//---------------------------------------------------------
//   ScoreFont
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class ScoreFont
{
    INJECT(score, mu::draw::IFontProvider, fontProvider)

    bool m_loaded = false;
    QVector<Sym> m_symbols;
    QString m_name;
    QString m_family;
    QString m_fontPath;
    QString m_filename;
    std::list<std::pair<Sid, QVariant> > m_engravingDefaults;
    double m_textEnclosureThickness = 0;
    mutable mu::draw::Font m_font;

    static QVector<ScoreFont> s_scoreFonts;
    static std::array<uint, size_t(SymId::lastSym) + 1> s_mainSymCodeTable;
    void load();
    void computeMetrics(Sym* sym, int code);

public:
    ScoreFont() {}
    ScoreFont(const ScoreFont&);
    ScoreFont(const char* n, const char* f, const char* p, const char* fn)
        : m_name(n), m_family(f), m_fontPath(p), m_filename(fn)
    {
        m_symbols = QVector<Sym>(int(SymId::lastSym) + 1);
    }

    ~ScoreFont() = default;

    const QString& name() const { return m_name; }
    const QString& family() const { return m_family; }
    std::list<std::pair<Sid, QVariant> > engravingDefaults() { return m_engravingDefaults; }
    double textEnclosureThickness() { return m_textEnclosureThickness; }

    QString fontPath() const { return m_fontPath; }

    static ScoreFont* fontFactory(QString);
    static ScoreFont* fallbackFont();
    static const char* fallbackTextFont();
    static const QVector<ScoreFont>& scoreFonts() { return s_scoreFonts; }
    static QJsonObject initGlyphNamesJson();

    QString toString(SymId id) const;
    uint symCode(SymId id) const;

    void draw(SymId id,                  mu::draw::Painter*, const QSizeF& mag, const QPointF& pos, qreal scale) const;
    void draw(SymId id,                  mu::draw::Painter*, qreal mag,         const QPointF& pos, qreal scale) const;
    void draw(SymId id,                  mu::draw::Painter*, qreal mag,         const QPointF& pos) const;
    void draw(SymId id,                  mu::draw::Painter*, const QSizeF& mag, const QPointF& pos) const;
    void draw(SymId id,                  mu::draw::Painter*, qreal mag,         const QPointF& pos, int n) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, qreal mag,         const QPointF& pos) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, const QSizeF& mag, const QPointF& pos) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, qreal mag,         const QPointF& pos, qreal scale) const;
    void draw(const std::vector<SymId>&, mu::draw::Painter*, const QSizeF& mag, const QPointF& pos, qreal scale) const;

    qreal height(SymId id, qreal mag) const { return bbox(id, mag).height(); }
    qreal width(SymId id, qreal mag) const { return bbox(id, mag).width(); }
    qreal advance(SymId id, qreal mag) const;
    qreal width(const std::vector<SymId>&, qreal mag) const;

    const QRectF bbox(SymId id, const QSizeF&) const;
    const QRectF bbox(SymId id, qreal mag) const;
    const QRectF bbox(const std::vector<SymId>& s, const QSizeF& mag) const;
    const QRectF bbox(const std::vector<SymId>& s, qreal mag) const;

    QPointF smuflAnchor(SymId symId, SmuflAnchorId anchorId, qreal mag) const;

    bool isValid(SymId id) const { return sym(id).isValid(); }
    bool useFallbackFont(SymId id) const;

    Sym sym(SymId id) const;

    friend void initScoreFonts();
};

extern void initScoreFonts();
}     // namespace Ms

#endif
