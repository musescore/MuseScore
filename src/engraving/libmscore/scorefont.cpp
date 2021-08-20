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
#include "scorefont.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "log.h"

#include "draw/painter.h"
#include "mscore.h"
#include "sym.h"

using namespace Ms;
using namespace mu;
using namespace mu::draw;

//---------------------------------------------------------
//   scoreFonts
//    this is the list of available score fonts
//---------------------------------------------------------

static const int FALLBACK_FONT_INDEX = 1; // Bravura

std::vector<ScoreFont> ScoreFont::s_scoreFonts {
    ScoreFont("Leland",     "Leland",      ":/fonts/leland/",    "Leland.otf"),
    ScoreFont("Bravura",    "Bravura",     ":/fonts/bravura/",   "Bravura.otf"),
    ScoreFont("Emmentaler", "MScore",      ":/fonts/mscore/",    "mscore.ttf"),
    ScoreFont("Gonville",   "Gootville",   ":/fonts/gootville/", "Gootville.otf"),
    ScoreFont("MuseJazz",   "MuseJazz",    ":/fonts/musejazz/",  "MuseJazz.otf"),
    ScoreFont("Petaluma",   "Petaluma",    ":/fonts/petaluma/",  "Petaluma.otf"),
};

std::array<uint, size_t(SymId::lastSym) + 1> ScoreFont::s_mainSymCodeTable { { 0 } };

// =============================================
// ScoreFont
// =============================================

ScoreFont::ScoreFont(const char* name, const char* family, const char* path, const char* filename)
    : m_name(name), m_family(family), m_fontPath(path), m_filename(filename)
{
    m_symbols = std::vector<Sym>(size_t(SymId::lastSym) + 1);
}

ScoreFont::ScoreFont(const ScoreFont& other)
{
    m_loaded = false;
    m_symbols  = other.m_symbols;
    m_name     = other.m_name;
    m_family   = other.m_family;
    m_fontPath = other.m_fontPath;
    m_filename = other.m_filename;
}

// =============================================
// Properties
// =============================================

const QString& ScoreFont::name() const
{
    return m_name;
}

const QString& ScoreFont::family() const
{
    return m_family;
}

QString ScoreFont::fontPath() const
{
    return m_fontPath;
}

std::list<std::pair<Sid, QVariant> > ScoreFont::engravingDefaults()
{
    return m_engravingDefaults;
}

double ScoreFont::textEnclosureThickness()
{
    return m_textEnclosureThickness;
}

// =============================================
// Init ScoreFonts
// =============================================

void ScoreFont::initScoreFonts()
{
    QJsonObject glyphNamesJson(ScoreFont::initGlyphNamesJson());
    if (glyphNamesJson.empty()) {
        qFatal("initGlyphNamesJson failed");
    }

    for (size_t i = 0; i < Sym::symNames.size(); ++i) {
        const char* name = Sym::symNames[i];
        Sym::nameToSymIdHash.insert(name, SymId(i));

        bool ok;
        uint code = glyphNamesJson.value(name).toObject().value("codepoint").toString().midRef(2).toUInt(&ok, 16);
        if (ok) {
            ScoreFont::s_mainSymCodeTable[i] = code;
        } else if (MScore::debugMode) {
            qDebug("codepoint not recognized for glyph %s", qPrintable(name));
        }
    }

    for (const Sym::OldName& i : qAsConst(Sym::oldNames)) {
        Sym::oldNameToSymIdHash.insert(i.name, SymId(i.symId));
    }

    auto fprovider = mu::modularity::ioc()->resolve<mu::draw::IFontProvider>("score");
    fprovider->insertSubstitution("Leland Text",    "Bravura Text");
    fprovider->insertSubstitution("Bravura Text",   "Leland Text");
    fprovider->insertSubstitution("MScore Text",    "Leland Text");
    fprovider->insertSubstitution("Gootville Text", "Leland Text");
    fprovider->insertSubstitution("MuseJazz Text",  "Leland Text");
    fprovider->insertSubstitution("Petaluma Text",  "MuseJazz Text");
    fprovider->insertSubstitution("ScoreFont",      "Leland Text"); // alias for current Musical Text Font

    fallbackFont(); // load fallback font
}

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

// =============================================
// Available ScoreFonts
// =============================================

const std::vector<ScoreFont>& ScoreFont::scoreFonts()
{
    return s_scoreFonts;
}

ScoreFont* ScoreFont::fontByName(const QString& name)
{
    ScoreFont* font = nullptr;
    for (ScoreFont& f : s_scoreFonts) {
        if (f.name().toLower() == name.toLower()) { // case insensitive
            font = &f;
            break;
        }
    }

    if (!font) {
        LOGE() << "ScoreFont not found in list: " << name;
        LOGE() << "ScoreFonts in list:";

        for (const ScoreFont& f : s_scoreFonts) {
            LOGE() << "    " << f.name();
        }

        font = fallbackFont();

        LOGE() << "Using fallback font " << font->name() << " instead.";
        return font;
    }

    if (!font->m_loaded) {
        font->load();
    }

    return font;
}

ScoreFont* ScoreFont::fallbackFont()
{
    ScoreFont* font = &s_scoreFonts[FALLBACK_FONT_INDEX];

    if (!font->m_loaded) {
        font->load();
    }

    return font;
}

const char* ScoreFont::fallbackTextFont()
{
    return "Bravura Text";
}

// =============================================
// Load
// =============================================

void ScoreFont::load()
{
    QString facePath = m_fontPath + m_filename;
    if (-1 == fontProvider()->addApplicationFont(m_family, facePath)) {
        LOGE() << "fatal error: cannot load internal font: " << facePath;
        return;
    }

    m_font.setWeight(mu::draw::Font::Normal);
    m_font.setItalic(false);
    m_font.setFamily(m_family);
    m_font.setNoFontMerging(true);
    m_font.setHinting(mu::draw::Font::Hinting::PreferVerticalHinting);

    for (size_t id = 0; id < s_mainSymCodeTable.size(); ++id) {
        uint code = s_mainSymCodeTable[id];
        if (code == 0) {
            continue;
        }
        SymId symId = SymId(id);
        Sym* sym    = &m_symbols[int(symId)];
        computeMetrics(sym, code);
    }

    QFile metadataFile(m_fontPath + "metadata.json");
    if (!metadataFile.open(QIODevice::ReadOnly)) {
        LOGE() << "Failed to open glyph metadata file: " << metadataFile.fileName();
        return;
    }

    QJsonParseError error;
    QJsonObject metadataJson = QJsonDocument::fromJson(metadataFile.readAll(), &error).object();
    if (error.error != QJsonParseError::NoError) {
        LOGE() << "Json parse error in " << metadataFile.fileName()
               << ", offset " << error.offset << ": " << error.errorString();
        return;
    }

    loadGlyphsWithAnchors(metadataJson.value("glyphsWithAnchors").toObject());
    loadComposedGlyphs();
    loadStylisticAlternates(metadataJson.value("glyphsWithAlternates").toObject());
    loadEngravingDefaults(metadataJson.value("engravingDefaults").toObject());

    m_loaded = true;
}

void ScoreFont::loadGlyphsWithAnchors(const QJsonObject& glyphsWithAnchors)
{
    for (const QString& symName : glyphsWithAnchors.keys()) {
        constexpr qreal scale = SPATIUM20;
        QJsonObject anchors = glyphsWithAnchors.value(symName).toObject();

        SymId symId = Sym::nameToSymIdHash.value(symName, SymId::noSym);
        if (symId == SymId::noSym) {
            //! NOTE currently, Bravura contains a bunch of entries in glyphsWithAnchors
            //! for glyph names that will not be found - flag32ndUpStraight, etc.
            continue;
        }

        Sym* sym = &m_symbols[int(symId)];

        for (const QString& anchorKey : anchors.keys()) {
            if (anchorKey == "stemDownNW") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemDownNW, PointF(x, -y) * 4.0 * DPI_F);
            } else if (anchorKey == "stemUpSE") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemUpSE, PointF(x, -y) * 4.0 * DPI_F);
            } else if (anchorKey == "stemDownSW") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemDownSW, PointF(x, -y) * 4.0 * DPI_F);
            } else if (anchorKey == "stemUpNW") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::stemUpNW, PointF(x, -y) * 4.0 * DPI_F);
            } else if (anchorKey == "cutOutNE") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::cutOutNE, PointF(x, -y) * scale);
            } else if (anchorKey == "cutOutNW") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::cutOutNW, PointF(x, -y) * scale);
            } else if (anchorKey == "cutOutSE") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::cutOutSE, PointF(x, -y) * scale);
            } else if (anchorKey == "cutOutSW") {
                qreal x = anchors.value(anchorKey).toArray().at(0).toDouble();
                qreal y = anchors.value(anchorKey).toArray().at(1).toDouble();
                sym->setSmuflAnchor(SmuflAnchorId::cutOutSW, PointF(x, -y) * scale);
            }
        }
    }
}

void ScoreFont::loadComposedGlyphs()
{
    static const struct ComposedGlyph {
        const SymId id;
        const std::vector<SymId> subSymbolIds;
    } composedGlyphs[] = {
        { SymId::ornamentPrallMordent, {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentUpPrall, {
              SymId::ornamentBottomLeftConcaveStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentUpMordent, {
              SymId::ornamentBottomLeftConcaveStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentPrallDown, {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentBottomRightConcaveStroke,
          } },
        {
            SymId::ornamentDownMordent, {
                SymId::ornamentLeftVerticalStroke,
                SymId::ornamentZigZagLineNoRightEnd,
                SymId::ornamentZigZagLineNoRightEnd,
                SymId::ornamentMiddleVerticalStroke,
                SymId::ornamentZigZagLineWithRightEnd
            }
        },
        { SymId::ornamentPrallUp, {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentTopRightConvexStroke,
          } },
        { SymId::ornamentLinePrall, {
              SymId::ornamentLeftVerticalStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineWithRightEnd
          } }
    };

    for (const ComposedGlyph& c : composedGlyphs) {
        if (!m_symbols[int(c.id)].isValid()) {
            Sym* sym = &m_symbols[int(c.id)];
            std::vector<SymId> subSymbolIds { c.subSymbolIds };
            sym->setSubSymbols(subSymbolIds);
            sym->setBbox(bbox(subSymbolIds, 1.0));
        }
    }
}

void ScoreFont::loadStylisticAlternates(const QJsonObject& glyphsWithAlternatesObject)
{
    static const struct GlyphWithAlternates {
        const QString key;
        const QString alternateKey;
        const SymId alternateSymId;
    } glyphsWithAlternates[] = {
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

    bool ok;
    for (const GlyphWithAlternates& glyph : glyphsWithAlternates) {
        const QJsonObject::const_iterator glyphIt = glyphsWithAlternatesObject.find(glyph.key);

        if (glyphIt != glyphsWithAlternatesObject.end()) {
            const QJsonArray alternatesArray = glyphIt.value().toObject().value("alternates").toArray();

            // locate the relevant altKey in alternate array
            const QJsonArray::const_iterator alternateIt
                = std::find_if(alternatesArray.cbegin(), alternatesArray.cend(), [&glyph](const QJsonValue& value) {
                return value.toObject().value("name") == glyph.alternateKey;
            });

            if (alternateIt != alternatesArray.cend()) {
                Sym* sym = &m_symbols[int(glyph.alternateSymId)];
                int codepoint = alternateIt->toObject().value("codepoint").toString().midRef(2).toInt(&ok, 16);
                if (ok) {
                    computeMetrics(sym, codepoint);
                }
            }
        }
    }
}

void ScoreFont::loadEngravingDefaults(const QJsonObject& engravingDefaultsObject)
{
    static const std::list<std::pair<QString, Sid> > engravingDefaultsMapping = {
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

    for (const QString& key : engravingDefaultsObject.keys()) {
        if (key == "textEnclosureThickness") {
            m_textEnclosureThickness = engravingDefaultsObject.value(key).toDouble();
            continue;
        }

        for (auto mapping : engravingDefaultsMapping) {
            if (key == mapping.first) {
                qreal value = engravingDefaultsObject.value(key).toDouble();

                if (key == "beamSpacing") {
                    value /= engravingDefaultsObject.value("beamThickness").toDouble();
                }

                m_engravingDefaults.push_back({ mapping.second, value });
            }
        }
    }

    m_engravingDefaults.push_back({ Sid::MusicalTextFont, QString("%1 Text").arg(m_family) });
}

void ScoreFont::computeMetrics(Sym* sym, int code)
{
    RectF bbox = fontProvider()->symBBox(m_font, code, DPI_F);
    qreal advance = fontProvider()->symAdvance(m_font, code, DPI_F);

    sym->setCode(code);
    sym->setBbox(bbox);
    sym->setAdvance(advance);
}

// =============================================
// Symbol properties
// =============================================

Sym ScoreFont::sym(SymId id) const
{
    size_t index = static_cast<size_t>(id);
    if (index < m_symbols.size()) {
        return m_symbols[index];
    }
    return Sym();
}

uint ScoreFont::symCode(SymId id) const
{
    const Sym& s = sym(id);
    uint code;
    if (s.isValid()) {
        code = static_cast<uint>(s.code());
    } else {
        // fallback: search in the common SMuFL table
        code = s_mainSymCodeTable[size_t(id)];
    }
    return code;
}

SymId ScoreFont::fromCode(uint code) const
{
    auto it = std::find_if(m_symbols.begin(), m_symbols.end(), [code](const Ms::Sym& s) { return s.code() == static_cast<int>(code); });
    return static_cast<SymId>(it == m_symbols.end() ? 0 : it - m_symbols.begin());
}

static QString codeToString(uint code)
{
    return QString::fromUcs4(&code, 1);
}

QString ScoreFont::toString(SymId id) const
{
    return codeToString(symCode(id));
}

bool ScoreFont::isValid(SymId id) const
{
    return sym(id).isValid();
}

bool ScoreFont::useFallbackFont(SymId id) const
{
    return MScore::useFallbackFont && !sym(id).isValid() && this != ScoreFont::fallbackFont();
}

// =============================================
// Symbol bouding box
// =============================================

const RectF ScoreFont::bbox(SymId id, qreal mag) const
{
    return bbox(id, SizeF(mag, mag));
}

const RectF ScoreFont::bbox(SymId id, const SizeF& mag) const
{
    if (useFallbackFont(id)) {
        return fallbackFont()->bbox(id, mag);
    }

    RectF r = sym(id).bbox();
    return RectF(r.x() * mag.width(), r.y() * mag.height(),
                 r.width() * mag.width(), r.height() * mag.height());
}

const RectF ScoreFont::bbox(const std::vector<SymId>& s, qreal mag) const
{
    return bbox(s, SizeF(mag, mag));
}

const RectF ScoreFont::bbox(const std::vector<SymId>& s, const SizeF& mag) const
{
    RectF r;
    PointF pos;
    for (SymId id : s) {
        r.unite(bbox(id, mag).translated(pos));
        pos.rx() += advance(id, mag.width());
    }
    return r;
}

// =============================================
// Symbol metrics
// =============================================

qreal ScoreFont::width(SymId id, qreal mag) const
{
    return bbox(id, mag).width();
}

qreal ScoreFont::height(SymId id, qreal mag) const
{
    return bbox(id, mag).width();
}

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

PointF ScoreFont::smuflAnchor(SymId symId, SmuflAnchorId anchorId, qreal mag) const
{
    if (useFallbackFont(symId)) {
        return fallbackFont()->smuflAnchor(symId, anchorId, mag);
    }

    return sym(symId).smuflAnchor(anchorId) * mag;
}

// =============================================
// Draw
// =============================================

void ScoreFont::draw(SymId id, Painter* painter, const SizeF& mag, const PointF& pos, qreal worldScale) const
{
    if (!sym(id).subSymbols().empty()) { // is this a compound symbol?
        draw(sym(id).subSymbols(), painter, mag, pos);
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

    qreal size = 20.0 * MScore::pixelRatio;
    m_font.setPointSizeF(size);
    SizeF imag = SizeF(1.0 / mag.width(), 1.0 / mag.height());
    painter->scale(mag.width(), mag.height());
    painter->setFont(m_font);
    painter->drawSymbol(PointF(pos.x() * imag.width(), pos.y() * imag.height()), symCode(id));
    painter->scale(imag.width(), imag.height());
}

void ScoreFont::draw(SymId id, Painter* painter, qreal mag, const PointF& pos) const
{
    qreal worldScale = painter->worldTransform().m11();
    draw(id, painter, mag, pos, worldScale);
}

void ScoreFont::draw(SymId id, Painter* painter, const SizeF& mag, const PointF& pos) const
{
    qreal worldScale = painter->worldTransform().m11();
    draw(id, painter, mag, pos, worldScale);
}

void ScoreFont::draw(SymId id, Painter* painter, qreal mag, const PointF& pos, qreal worldScale) const
{
    draw(id, painter, SizeF(mag, mag), pos, worldScale);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, qreal mag, const PointF& pos, int n) const
{
    std::vector<SymId> d;
    d.reserve(n);
    for (int i = 0; i < n; ++i) {
        d.push_back(id);
    }

    draw(d, painter, mag, pos);
}

void ScoreFont::draw(const std::vector<SymId>& ids, Painter* p, qreal mag, const PointF& _pos, qreal scale) const
{
    PointF pos(_pos);
    for (SymId id : ids) {
        draw(id, p, mag, pos, scale);
        pos.setX(pos.x() + advance(id, mag));
    }
}

void ScoreFont::draw(const std::vector<SymId>& ids, Painter* p, const SizeF& mag, const PointF& _pos) const
{
    qreal scale = p->worldTransform().m11();
    draw(ids, p, mag, _pos, scale);
}

void ScoreFont::draw(const std::vector<SymId>& ids, Painter* p, const SizeF& mag, const PointF& _pos,
                     qreal scale) const
{
    PointF pos(_pos);
    for (SymId id : ids) {
        draw(id, p, mag, pos, scale);
        pos.setX(pos.x() + advance(id, mag.width()));
    }
}

void ScoreFont::draw(const std::vector<SymId>& ids, Painter* p, qreal mag, const PointF& _pos) const
{
    qreal scale = p->worldTransform().m11();
    draw(ids, p, mag, _pos, scale);
}
