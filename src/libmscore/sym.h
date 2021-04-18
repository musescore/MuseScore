//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYM_H__
#define __SYM_H__

#include <QApplication>

#include "config.h"
#include "style.h"
#include "qtenum.h"
#include "symid.h"

#include "draw/painter.h"

#include "ft2build.h"
#include FT_FREETYPE_H

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
    FT_UInt _index = 0;
    QRectF _bbox;
    qreal _advance = 0.0;

    std::map<SmuflAnchorId, QPointF> smuflAnchors;
    std::vector<SymId> _ids; // not empty if this is a compound symbol

public:
    Sym() { }

    bool isValid() const { return _code != -1 && _bbox.isValid(); }

    void setSymList(const std::vector<SymId>& sl) { _ids = sl; }
    const std::vector<SymId>& symList() const { return _ids; }

    FT_UInt index() const { return _index; }
    void setIndex(FT_UInt i) { _index = i; }

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
//   GlyphKey
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

struct GlyphKey {
    FT_Face face;
    SymId id;
    qreal magX;
    qreal magY;
    qreal worldScale;
    QColor color;

public:
    GlyphKey(FT_Face _f, SymId _id, float mx, float my, float s, QColor c)
        : face(_f), id(_id), magX(mx), magY(my), worldScale(s), color(c) {}
    bool operator==(const GlyphKey&) const;
};

//---------------------------------------------------------
//   GlyphPixmap
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

struct GlyphPixmap {
    QPixmap pm;
    QPointF offset;
};

inline uint qHash(const GlyphKey& k)
{
    return (int(k.id) << 16) + (int(k.magX * 100) << 8) + k.magY * 100;
}

//---------------------------------------------------------
//   ScoreFont
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class ScoreFont
{
    FT_Face face = 0;
    QVector<Sym> _symbols;
    QString _name;
    QString _family;
    QString _fontPath;
    QString _filename;
    QByteArray fontImage;
    QCache<GlyphKey, GlyphPixmap>* cache { 0 };
    std::list<std::pair<Sid, QVariant> > _engravingDefaults;
    double _textEnclosureThickness = 0;
    mutable QFont* font { 0 };

    static QVector<ScoreFont> _scoreFonts;
    static std::array<uint, size_t(SymId::lastSym) + 1> _mainSymCodeTable;
    void load();
    void computeMetrics(Sym* sym, int code);

public:
    ScoreFont() {}
    ScoreFont(const ScoreFont&);
    ScoreFont(const char* n, const char* f, const char* p, const char* fn)
        : _name(n), _family(f), _fontPath(p), _filename(fn)
    {
        _symbols = QVector<Sym>(int(SymId::lastSym) + 1);
    }

    ~ScoreFont();

    const QString& name() const { return _name; }
    const QString& family() const { return _family; }
    std::list<std::pair<Sid, QVariant> > engravingDefaults() { return _engravingDefaults; }
    double textEnclosureThickness() { return _textEnclosureThickness; }

    QString fontPath() const { return _fontPath; }

    static ScoreFont* fontFactory(QString);
    static ScoreFont* fallbackFont();
    static const char* fallbackTextFont();
    static const QVector<ScoreFont>& scoreFonts() { return _scoreFonts; }
    static QJsonObject initGlyphNamesJson();

    QString toString(SymId) const;
    QPixmap sym2pixmap(SymId, qreal) { return QPixmap(); }        // TODOxxxx

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
