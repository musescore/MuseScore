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

#include <cmath>
#include <QFontDatabase>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCache>

#include "style.h"
#include "sym.h"
#include "utils.h"
#include "score.h"
#include "xml.h"
#include "mscore.h"

#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_BBOX_H

static FT_Library ftlib;

namespace Ms {
//---------------------------------------------------------
//   scoreFonts
//    this is the list of available score fonts
//---------------------------------------------------------

static const int FALLBACK_FONT = 1;       // Bravura

QVector<ScoreFont> ScoreFont::_scoreFonts {
    ScoreFont("Leland",     "Leland",      ":/fonts/leland/",    "Leland.otf"),
    ScoreFont("Bravura",    "Bravura",     ":/fonts/bravura/",   "Bravura.otf"),
    ScoreFont("Emmentaler", "MScore",      ":/fonts/mscore/",    "mscore.ttf"),
    ScoreFont("Gonville",   "Gootville",   ":/fonts/gootville/", "Gootville.otf"),
    ScoreFont("MuseJazz",   "MuseJazz",    ":/fonts/musejazz/",  "MuseJazz.otf"),
    ScoreFont("Petaluma",   "Petaluma",    ":/fonts/petaluma/",  "Petaluma.otf"),
};

std::array<uint, size_t(SymId::lastSym) + 1> ScoreFont::_mainSymCodeTable { { 0 } };

//---------------------------------------------------------
//   commonScoreSymbols
//    subset for use in text palette, possible translations, etc
//---------------------------------------------------------

const QVector<SymId> Sym::commonScoreSymbols = {
    SymId::accidentalFlat,
    SymId::accidentalNatural,
    SymId::accidentalSharp,
    SymId::accidentalDoubleFlat,
    SymId::accidentalDoubleSharp,
    SymId::metNoteWhole,
    SymId::metNoteHalfUp,
    SymId::metNoteQuarterUp,
    SymId::metNote8thUp,
    SymId::metNote16thUp,
    SymId::metNote32ndUp,
    SymId::metNote64thUp,
    SymId::metNote128thUp,
    SymId::metAugmentationDot,
    SymId::restWholeLegerLine,
    SymId::restHalfLegerLine,
    SymId::restQuarter,
    SymId::rest8th,
    SymId::rest16th,
    SymId::rest32nd,
    SymId::rest64th,
    SymId::rest128th,
    SymId::segno,
    SymId::coda,
    SymId::segnoSerpent1,
    SymId::codaSquare,
    SymId::repeat1Bar,
    SymId::repeat2Bars,
    SymId::repeat4Bars,
    SymId::gClef,
    SymId::fClef,
    SymId::cClef,
    SymId::lyricsElisionNarrow,
    SymId::lyricsElision,
    SymId::lyricsElisionWide,
    SymId::dynamicPiano,
    SymId::dynamicMezzo,
    SymId::dynamicForte,
    SymId::dynamicNiente,
    SymId::dynamicRinforzando,
    SymId::dynamicSforzando,
    SymId::dynamicZ,
    SymId::space
};

//---------------------------------------------------------
//   userName2id
//---------------------------------------------------------

SymId Sym::userName2id(const QString& s)
{
    int idx = 0;
    for (const char* a : symUserNames) {
        if (a && strcmp(a, qPrintable(s)) == 0) {
            return SymId(idx);
        }
    }
    return SymId::noSym;
}

//---------------------------------------------------------
//   GlyphKey operator==
//---------------------------------------------------------

bool GlyphKey::operator==(const GlyphKey& k) const
{
    return (face == k.face) && (id == k.id)
           && (magX == k.magX) && (magY == k.magY) && (worldScale == k.worldScale) && (color == k.color);
}

Sym ScoreFont::sym(SymId id) const
{
    int index = static_cast<int>(id);

    if (index >= 0 && index < _symbols.size()) {
        return _symbols[index];
    }

    return Sym();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, qreal mag, const QPointF& pos) const
{
    qreal worldScale = painter->worldTransform().m11();
    draw(id, painter, mag, pos, worldScale);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, const QSizeF& mag, const QPointF& pos) const
{
    qreal worldScale = painter->worldTransform().m11();
    draw(id, painter, mag, pos, worldScale);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, qreal mag, const QPointF& pos, qreal worldScale) const
{
    draw(id, painter, QSizeF(mag, mag), pos, worldScale);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, const QSizeF& mag, const QPointF& pos, qreal worldScale) const
{
    if (!sym(id).symList().empty()) {    // is this a compound symbol?
        draw(sym(id).symList(), painter, mag, pos);
        return;
    }
    if (!isValid(id)) {
        if (MScore::useFallbackFont && this != ScoreFont::fallbackFont()) {
            fallbackFont()->draw(id, painter, mag, pos, worldScale);
        } else {
            qDebug("ScoreFont::draw: invalid sym %d", int(id));
        }
        return;
    }
    int rv = FT_Load_Glyph(face, sym(id).index(), FT_LOAD_DEFAULT);
    if (rv) {
        qDebug("load glyph id %d, failed: 0x%x", int(id), rv);
        return;
    }

    if (MScore::pdfPrinting) {
        if (font == 0) {
            QString s(_fontPath + _filename);
            if (-1 == QFontDatabase::addApplicationFont(s)) {
                qDebug("Mscore: fatal error: cannot load internal font <%s>", qPrintable(s));
                return;
            }
            font = new QFont;
            font->setWeight(QFont::Normal);
            font->setItalic(false);
            font->setFamily(_family);
            font->setStyleStrategy(QFont::NoFontMerging);
            font->setHintingPreference(QFont::PreferVerticalHinting);
        }
        qreal size = 20.0 * MScore::pixelRatio;
        font->setPointSize(size);
        QSizeF imag = QSizeF(1.0 / mag.width(), 1.0 / mag.height());
        painter->scale(mag.width(), mag.height());
        painter->setFont(*font);
        painter->drawText(QPointF(pos.x() * imag.width(), pos.y() * imag.height()), toString(id));
        painter->scale(imag.width(), imag.height());
        return;
    }

    QColor color(painter->pen().color());

    int pr           = painter->device()->devicePixelRatio();
    qreal pixelRatio = qreal(pr > 0 ? pr : 1);
    worldScale      *= pixelRatio;
//      if (worldScale < 1.0)
//            worldScale = 1.0;
    int scale16X      = lrint(worldScale * 6553.6 * mag.width() * DPI_F);
    int scale16Y      = lrint(worldScale * 6553.6 * mag.height() * DPI_F);

    GlyphKey gk(face, id, mag.width(), mag.height(), worldScale, color);
    GlyphPixmap* pm = cache->object(gk);

    if (!pm) {
        FT_Matrix matrix {
            scale16X, 0,
            0,       scale16Y
        };

        FT_Glyph glyph;
        FT_Get_Glyph(face->glyph, &glyph);
        FT_Glyph_Transform(glyph, &matrix, 0);
        rv = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        if (rv) {
            qDebug("glyph to bitmap failed: 0x%x", rv);
            return;
        }

        FT_BitmapGlyph gb = (FT_BitmapGlyph)glyph;
        FT_Bitmap* bm     = &gb->bitmap;

        if (bm->width == 0 || bm->rows == 0) {
            qDebug("zero glyph, id %d", int(id));
            return;
        }
        QImage img(QSize(bm->width, bm->rows), QImage::Format_ARGB32);
        img.fill(Qt::transparent);

        for (unsigned y = 0; y < bm->rows; ++y) {
            unsigned* dst      = (unsigned*)img.scanLine(y);
            unsigned char* src = (unsigned char*)(bm->buffer) + bm->pitch * y;
            for (unsigned x = 0; x < bm->width; ++x) {
                unsigned val = *src++;
                color.setAlpha(std::min(int(val), painter->pen().color().alpha()));
                *dst++ = color.rgba();
            }
        }
        pm = new GlyphPixmap;
        pm->pm = QPixmap::fromImage(img, Qt::NoFormatConversion);
        pm->pm.setDevicePixelRatio(worldScale);
        pm->offset = QPointF(qreal(gb->left), -qreal(gb->top)) / worldScale;
        if (!cache->insert(gk, pm)) {
            qDebug("cannot cache glyph");
        }
        FT_Done_Glyph(glyph);
    }
    painter->drawPixmap(pos + pm->offset, pm->pm);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, qreal mag, const QPointF& pos, int n) const
{
    std::vector<SymId> d;
    for (int i = 0; i < n; ++i) {
        d.push_back(id);
    }
    draw(d, painter, mag, pos);
}

void ScoreFont::draw(const std::vector<SymId>& ids, mu::draw::Painter* p, qreal mag, const QPointF& _pos, qreal scale) const
{
    QPointF pos(_pos);
    for (SymId id : ids) {
        draw(id, p, mag, pos, scale);
        pos.rx() += advance(id, mag);
    }
}

void ScoreFont::draw(const std::vector<SymId>& ids, mu::draw::Painter* p, const QSizeF& mag, const QPointF& _pos) const
{
    qreal scale = p->worldTransform().m11();
    draw(ids, p, mag, _pos, scale);
}

void ScoreFont::draw(const std::vector<SymId>& ids, mu::draw::Painter* p, const QSizeF& mag, const QPointF& _pos, qreal scale) const
{
    QPointF pos(_pos);
    for (SymId id : ids) {
        draw(id, p, mag, pos, scale);
        pos.rx() += advance(id, mag.width());
    }
}

void ScoreFont::draw(const std::vector<SymId>& ids, mu::draw::Painter* p, qreal mag, const QPointF& _pos) const
{
    qreal scale = p->worldTransform().m11();
    draw(ids, p, mag, _pos, scale);
}

//---------------------------------------------------------
//   id2name
//---------------------------------------------------------

const char* Sym::id2name(SymId id)
{
    return symNames[int(id)];
}

//---------------------------------------------------------
//   initScoreFonts
//    load default score font
//---------------------------------------------------------

void initScoreFonts()
{
    QJsonObject glyphNamesJson(ScoreFont::initGlyphNamesJson());
    if (glyphNamesJson.empty()) {
        qFatal("initGlyphNamesJson failed");
    }
    int error = FT_Init_FreeType(&ftlib);
    if (!ftlib || error) {
        qFatal("init freetype library failed");
    }
    for (size_t i = 0; i < Sym::symNames.size(); ++i) {
        const char* name = Sym::symNames[i];
        Sym::lnhash.insert(name, SymId(i));
        bool ok;
        uint code = glyphNamesJson.value(name).toObject().value("codepoint").toString().midRef(2).toUInt(&ok, 16);
        if (ok) {
            ScoreFont::_mainSymCodeTable[i] = code;
        } else if (MScore::debugMode) {
            qDebug("codepoint not recognized for glyph %s", qPrintable(name));
        }
    }
    for (const Sym::OldName& i : qAsConst(Sym::oldNames)) {
        Sym::lonhash.insert(i.name, SymId(i.symId));
    }
    QFont::insertSubstitution("Leland Text",    "Bravura Text");
    QFont::insertSubstitution("Bravura Text",   "Leland Text");
    QFont::insertSubstitution("MScore Text",    "Leland Text");
    QFont::insertSubstitution("Gootville Text", "Leland Text");
    QFont::insertSubstitution("MuseJazz Text",  "Leland Text");
    QFont::insertSubstitution("Petaluma Text",  "MuseJazz Text");
    QFont::insertSubstitution("ScoreFont",      "Leland Text");   // alias for current Musical Text Font
    ScoreFont::fallbackFont();     // load fallback font
}

//---------------------------------------------------------
//   codeToString
//---------------------------------------------------------

static QString codeToString(uint code)
{
    return QString::fromUcs4(&code, 1);
}

//---------------------------------------------------------
//   toString
//---------------------------------------------------------

QString ScoreFont::toString(SymId id) const
{
    const Sym& s = sym(id);
    int code;
    if (s.isValid()) {
        code = s.code();
    } else {
        // fallback: search in the common SMuFL table
        code = _mainSymCodeTable[size_t(id)];
    }
    return codeToString(code);
}

//---------------------------------------------------------
//   computeMetrics
//---------------------------------------------------------

void ScoreFont::computeMetrics(Sym* sym, int code)
{
    FT_UInt index = FT_Get_Char_Index(face, code);
    if (index != 0) {
        if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT) == 0) {
            FT_BBox bb;
            if (FT_Outline_Get_BBox(&face->glyph->outline, &bb) == 0) {
                constexpr double m = 640.0 / DPI_F;
                QRectF bbox;
                bbox.setCoords(bb.xMin / m, -bb.yMax / m, bb.xMax / m, -bb.yMin / m);
                sym->setIndex(index);
                sym->setCode(code);
                sym->setBbox(bbox);
                sym->setAdvance(face->glyph->linearHoriAdvance * DPI_F / 655360.0);
            }
        } else {
            qDebug("load glyph failed");
        }
    }
//      else
//            qDebug("no index");
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void ScoreFont::load()
{
    QString facePath = _fontPath + _filename;
    QFile f(facePath);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug("ScoreFont::load(): open failed <%s>", qPrintable(facePath));
        return;
    }
    fontImage = f.readAll();
    int rval = FT_New_Memory_Face(ftlib, (FT_Byte*)fontImage.data(), fontImage.size(), 0, &face);
    if (rval) {
        qDebug("freetype: cannot create face <%s>: %d", qPrintable(facePath), rval);
        return;
    }
    cache = new QCache<GlyphKey, GlyphPixmap>(100);

    qreal pixelSize = 200.0;
    FT_Set_Pixel_Sizes(face, 0, int(pixelSize + .5));

    for (size_t id = 0; id < _mainSymCodeTable.size(); ++id) {
        uint code = _mainSymCodeTable[id];
        if (code == 0) {
            continue;
        }
        SymId symId = SymId(id);
        Sym* sym    = &_symbols[int(symId)];
        computeMetrics(sym, code);
    }

    QJsonParseError error;
    QFile fi(_fontPath + "metadata.json");
    if (!fi.open(QIODevice::ReadOnly)) {
        qDebug("ScoreFont: open glyph metadata file <%s> failed", qPrintable(fi.fileName()));
    }
    QJsonObject metadataJson = QJsonDocument::fromJson(fi.readAll(), &error).object();
    if (error.error != QJsonParseError::NoError) {
        qDebug("Json parse error in <%s>(offset: %d): %s", qPrintable(fi.fileName()),
               error.offset, qPrintable(error.errorString()));
    }

    QJsonObject glyphsWithAnchors = metadataJson.value("glyphsWithAnchors").toObject();
    for (const auto& symName : glyphsWithAnchors.keys()) {
        constexpr qreal scale = SPATIUM20;
        QJsonObject anchors = glyphsWithAnchors.value(symName).toObject();
        SymId symId = Sym::lnhash.value(symName, SymId::noSym);
        if (symId == SymId::noSym) {
            // currently, Bravura contains a bunch of entries in glyphsWithAnchors
            // for glyph names that will not be found - flag32ndUpStraight, etc.
            //qDebug("ScoreFont: symId not found <%s> in <%s>", qPrintable(i), qPrintable(fi.fileName()));
            continue;
        }
        Sym* sym = &_symbols[int(symId)];
        for (const auto& j : anchors.keys()) {
            if (j == "stemDownNW") {
                qreal x = anchors.value(j).toArray().at(0).toDouble();
                qreal y = anchors.value(j).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemDownNW, QPointF(4.0 * DPI_F * x, 4.0 * DPI_F * -y));
            } else if (j == "stemUpSE") {
                qreal x = anchors.value(j).toArray().at(0).toDouble();
                qreal y = anchors.value(j).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemUpSE, QPointF(4.0 * DPI_F * x, 4.0 * DPI_F * -y));
            } else if (j == "stemDownSW") {
                qreal x = anchors.value(j).toArray().at(0).toDouble();
                qreal y = anchors.value(j).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemDownSW, QPointF(4.0 * DPI_F * x, 4.0 * DPI_F * -y));
            } else if (j == "stemUpNW") {
                qreal x = anchors.value(j).toArray().at(0).toDouble();
                qreal y = anchors.value(j).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemUpNW, QPointF(4.0 * DPI_F * x, 4.0 * DPI_F * -y));
            } else if (j == "cutOutNE") {
                qreal x = anchors.value(j).toArray().at(0).toDouble() * scale;
                qreal y = anchors.value(j).toArray().at(1).toDouble() * scale;
                sym->setSmuflAnchor(SmuflAnchorId::cutOutNE, QPointF(x, -y));
            } else if (j == "cutOutNW") {
                qreal x = anchors.value(j).toArray().at(0).toDouble() * scale;
                qreal y = anchors.value(j).toArray().at(1).toDouble() * scale;
                sym->setSmuflAnchor(SmuflAnchorId::cutOutNW, QPointF(x, -y));
            } else if (j == "cutOutSE") {
                qreal x = anchors.value(j).toArray().at(0).toDouble() * scale;
                qreal y = anchors.value(j).toArray().at(1).toDouble() * scale;
                sym->setSmuflAnchor(SmuflAnchorId::cutOutSE, QPointF(x, -y));
            } else if (j == "cutOutSW") {
                qreal x = anchors.value(j).toArray().at(0).toDouble() * scale;
                qreal y = anchors.value(j).toArray().at(1).toDouble() * scale;
                sym->setSmuflAnchor(SmuflAnchorId::cutOutSW, QPointF(x, -y));
            }
        }
    }
    glyphsWithAnchors = metadataJson.value("engravingDefaults").toObject();
    static std::list<std::pair<QString, Sid> > engravingDefaultsMapping = {
        { "staffLineThickness",            Sid::staffLineWidth },
        { "stemThickness",                 Sid::stemWidth },
        { "beamThickness",                 Sid::beamWidth },
        { "beamSpacing",                   Sid::beamDistance },
        { "legerLineThickness",            Sid::ledgerLineWidth },
        { "legerLineExtension",            Sid::ledgerLineLength },
        { "slurEndpointThickness",         Sid::SlurEndWidth },
        { "slurMidpointThickness",         Sid::SlurMidWidth },
        { "thinBarlineThickness",          Sid::barWidth },
        { "thinBarlineThickness",          Sid::doubleBarWidth },
        { "thickBarlineThickness",         Sid::endBarWidth },
        { "dashedBarlineThickness",        Sid::barWidth },
        { "barlineSeparation",             Sid::doubleBarDistance },
        { "barlineSeparation",             Sid::endBarDistance },
        { "repeatBarlineDotSeparation",    Sid::repeatBarlineDotSeparation },
        { "bracketThickness",              Sid::bracketWidth },
        { "hairpinThickness",              Sid::hairpinLineWidth },
        { "octaveLineThickness",           Sid::ottavaLineWidth },
        { "pedalLineThickness",            Sid::pedalLineWidth },
        { "repeatEndingLineThickness",     Sid::voltaLineWidth },
        { "lyricLineThickness",            Sid::lyricsLineThickness },
        { "tupletBracketThickness",        Sid::tupletBracketWidth }
    };

    for (const auto& i : glyphsWithAnchors.keys()) {
        for (auto mapping : engravingDefaultsMapping) {
            if (i == mapping.first) {
                qreal value = glyphsWithAnchors.value(i).toDouble();

                if (i == "beamSpacing") {
                    value /= glyphsWithAnchors.value("beamThickness").toDouble();
                }

                _engravingDefaults.push_back(std::make_pair(mapping.second, value));
            } else if (i == "textEnclosureThickness") {
                _textEnclosureThickness = glyphsWithAnchors.value(i).toDouble();
            }
        }
    }
    _engravingDefaults.push_back(std::make_pair(Sid::MusicalTextFont, QString("%1 Text").arg(_family)));

    // create missing composed glyphs
    struct Composed {
        SymId id;
        std::vector<SymId> rids;
    } composed[] = {
        { SymId::ornamentPrallMordent,
          {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentUpPrall,
          {
              SymId::ornamentBottomLeftConcaveStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentUpMordent,
          {
              SymId::ornamentBottomLeftConcaveStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentPrallDown,
          {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentBottomRightConcaveStroke,
          } },
#if 0
        {
            SymId::ornamentDownPrall,
            {
                SymId::ornamentTopLeftConvexStroke,
                SymId::ornamentZigZagLineNoRightEnd,
                SymId::ornamentZigZagLineNoRightEnd,
                SymId::ornamentZigZagLineWithRightEnd
            }
        },
#endif
        {
            SymId::ornamentDownMordent,
            {
                SymId::ornamentLeftVerticalStroke,
                SymId::ornamentZigZagLineNoRightEnd,
                SymId::ornamentZigZagLineNoRightEnd,
                SymId::ornamentMiddleVerticalStroke,
                SymId::ornamentZigZagLineWithRightEnd
            }
        },
        { SymId::ornamentPrallUp,
          {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentTopRightConvexStroke,
          } },
        { SymId::ornamentLinePrall,
          {
              SymId::ornamentLeftVerticalStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineWithRightEnd
          } }
    };

    for (const Composed& c : composed) {
        if (!_symbols[int(c.id)].isValid()) {
            Sym* sym = &_symbols[int(c.id)];
            std::vector<SymId> s;
            for (SymId id : c.rids) {
                s.push_back(id);
            }
            sym->setSymList(s);
            sym->setBbox(bbox(s, 1.0));
        }
    }

    // access needed stylistic alternates

    struct StylisticAlternate {
        QString key;
        QString altKey;
        SymId id;
    }
    alternate[] = {
        { QString("4stringTabClef"),
          QString("4stringTabClefSerif"),
          SymId::fourStringTabClefSerif
        },
        { QString("6stringTabClef"),
          QString("6stringTabClefSerif"),
          SymId::sixStringTabClefSerif
        },
        { QString("cClef"),
          QString("cClefFrench"),
          SymId::cClefFrench
        },
        { QString("cClef"),
          QString("cClefFrench20C"),
          SymId::cClefFrench20C
        },
        { QString("fClef"),
          QString("fClefFrench"),
          SymId::fClefFrench
        },
        { QString("fClef"),
          QString("fClef19thCentury"),
          SymId::fClef19thCentury
        },
        { QString("noteheadBlack"),
          QString("noteheadBlackOversized"),
          SymId::noteheadBlack
        },
        { QString("noteheadHalf"),
          QString("noteheadHalfOversized"),
          SymId::noteheadHalf
        },
        { QString("noteheadWhole"),
          QString("noteheadWholeOversized"),
          SymId::noteheadWhole
        },
        { QString("noteheadDoubleWhole"),
          QString("noteheadDoubleWholeOversized"),
          SymId::noteheadDoubleWhole
        },
        { QString("noteheadDoubleWholeSquare"),
          QString("noteheadDoubleWholeSquareOversized"),
          SymId::noteheadDoubleWholeSquare
        },
        { QString("noteheadDoubleWhole"),
          QString("noteheadDoubleWholeAlt"),
          SymId::noteheadDoubleWholeAlt
        },
        { QString("brace"),
          QString("braceSmall"),
          SymId::braceSmall
        },
        { QString("brace"),
          QString("braceLarge"),
          SymId::braceLarge
        },
        { QString("brace"),
          QString("braceLarger"),
          SymId::braceLarger
        }
    };

    // find each relevant alternate in "glyphsWithAlternates" value
    QJsonObject oa = metadataJson.value("glyphsWithAlternates").toObject();
    bool ok;
    for (const StylisticAlternate& c : alternate) {
        QJsonObject::const_iterator i = oa.find(c.key);
        if (i != oa.end()) {
            QJsonArray oaa = i.value().toObject().value("alternates").toArray();
            // locate the relevant altKey in alternate array
            for (const auto& j : qAsConst(oaa)) {
                QJsonObject jo = j.toObject();
                if (jo.value("name") == c.altKey) {
                    Sym* sym = &_symbols[int(c.id)];
                    int code = jo.value("codepoint").toString().midRef(2).toInt(&ok, 16);
                    if (ok) {
                        computeMetrics(sym, code);
                    }
                    break;
                }
            }
        }
    }

    // add space symbol
    Sym* sym = &_symbols[int(SymId::space)];
    computeMetrics(sym, 32);

#if 0
    //
    // check for missing symbols
    //
    ScoreFont* fb = ScoreFont::fallbackFont();
    if (fb && fb != this) {
        for (int i = 1; i < int(SymId::lastSym); ++i) {
            const Sym& sym = _symbols[i];
            if (!sym.isValid()) {
                qDebug("invalid symbol %s", Sym::id2name(SymId(i)));
            }
        }
    }
#endif
}

//---------------------------------------------------------
//   fontFactory
//---------------------------------------------------------

ScoreFont* ScoreFont::fontFactory(QString s)
{
    ScoreFont* f = 0;
    for (ScoreFont& sf : _scoreFonts) {
        if (sf.name().toLower() == s.toLower()) {     // ignore letter case
            f = &sf;
            break;
        }
    }
    if (!f) {
        qDebug("ScoreFont <%s> not found in list", qPrintable(s));
        for (ScoreFont& sf : _scoreFonts) {
            qDebug("   %s", qPrintable(sf.name()));
        }
        qDebug("Using fallback font <%s> instead", qPrintable(_scoreFonts[FALLBACK_FONT].name()));
        return fallbackFont();
    }

    if (!f->face) {
        f->load();
    }
    return f;
}

//---------------------------------------------------------
//   fallbackFont
//---------------------------------------------------------

ScoreFont* ScoreFont::fallbackFont()
{
    ScoreFont* f = &_scoreFonts[FALLBACK_FONT];
    if (!f->face) {
        f->load();
    }
    return f;
}

//---------------------------------------------------------
//   fallbackTextFont
//---------------------------------------------------------

const char* ScoreFont::fallbackTextFont()
{
    return "Bravura Text";
}

//---------------------------------------------------------
//   initGlyphNamesJson
//---------------------------------------------------------

QJsonObject ScoreFont::initGlyphNamesJson()
{
    QFile fi(":fonts/smufl/glyphnames.json");
    if (!fi.open(QIODevice::ReadOnly)) {
        qDebug("ScoreFont: open glyph names file <%s> failed", qPrintable(fi.fileName()));
        return QJsonObject();
    }
    QJsonParseError error;
    QJsonObject glyphNamesJson = QJsonDocument::fromJson(fi.readAll(), &error).object();
    if (error.error != QJsonParseError::NoError) {
        qDebug("Json parse error in <%s>(offset: %d): %s", qPrintable(fi.fileName()),
               error.offset, qPrintable(error.errorString()));
        return QJsonObject();
    }
    fi.close();
    return glyphNamesJson;
}

//---------------------------------------------------------
//   useFallbackFont
//---------------------------------------------------------

bool ScoreFont::useFallbackFont(SymId id) const
{
    return MScore::useFallbackFont && !sym(id).isValid() && this != ScoreFont::fallbackFont();
}

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF ScoreFont::bbox(SymId id, qreal mag) const
{
    return bbox(id, QSizeF(mag, mag));
}

const QRectF ScoreFont::bbox(SymId id, const QSizeF& mag) const
{
    if (useFallbackFont(id)) {
        return fallbackFont()->bbox(id, mag);
    }
    QRectF r = sym(id).bbox();
    return QRectF(r.x() * mag.width(), r.y() * mag.height(), r.width() * mag.width(), r.height() * mag.height());
}

const QRectF ScoreFont::bbox(const std::vector<SymId>& s, qreal mag) const
{
    return bbox(s, QSizeF(mag, mag));
}

const QRectF ScoreFont::bbox(const std::vector<SymId>& s, const QSizeF& mag) const
{
    QRectF r;
    QPointF pos;
    for (SymId id : s) {
        r |= bbox(id, mag).translated(pos);
        pos.rx() += advance(id, mag.width());
    }
    return r;
}

//---------------------------------------------------------
//   advance
//---------------------------------------------------------

qreal ScoreFont::advance(SymId id, qreal mag) const
{
    if (useFallbackFont(id)) {
        return fallbackFont()->advance(id, mag);
    }
    return sym(id).advance() * mag;
}

qreal ScoreFont::width(const std::vector<SymId>& s, qreal mag) const
{
    return bbox(s, mag).width();
}

QPointF ScoreFont::smuflAnchor(SymId symId, SmuflAnchorId anchorId, qreal mag) const
{
    if (useFallbackFont(symId)) {
        return fallbackFont()->smuflAnchor(symId, anchorId, mag);
    }
    return sym(symId).smuflAnchor(anchorId) * mag;
}

//---------------------------------------------------------
//   ScoreFont
//---------------------------------------------------------

ScoreFont::ScoreFont(const ScoreFont& f)
{
    face = 0;
    _symbols  = f._symbols;
    _name     = f._name;
    _family   = f._family;
    _fontPath = f._fontPath;
    _filename = f._filename;

    // fontImage;
    cache = 0;
}

ScoreFont::~ScoreFont()
{
    delete cache;
}
}
