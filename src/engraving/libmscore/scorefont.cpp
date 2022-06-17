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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "io/file.h"
#include "draw/painter.h"
#include "types/symnames.h"

#include "mscore.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::draw;
using namespace mu::engraving;

static constexpr int FALLBACK_FONT_INDEX = 1; // Bravura

std::vector<ScoreFont> ScoreFont::s_scoreFonts {
    ScoreFont("Leland",     "Leland",      ":/fonts/leland/",    "Leland.otf"),
    ScoreFont("Bravura",    "Bravura",     ":/fonts/bravura/",   "Bravura.otf"),
    ScoreFont("Emmentaler", "MScore",      ":/fonts/mscore/",    "mscore.ttf"),
    ScoreFont("Gonville",   "Gootville",   ":/fonts/gootville/", "Gootville.otf"),
    ScoreFont("MuseJazz",   "MuseJazz",    ":/fonts/musejazz/",  "MuseJazz.otf"),
    ScoreFont("Petaluma",   "Petaluma",    ":/fonts/petaluma/",  "Petaluma.otf"),
};

std::array<uint, size_t(SymId::lastSym) + 1> ScoreFont::s_symIdCodes { { 0 } };

// =============================================
// ScoreFont
// =============================================

ScoreFont::ScoreFont(const char* name, const char* family, const char* path, const char* filename)
    : m_symbols(static_cast<size_t>(SymId::lastSym) + 1),
    m_name(name),
    m_family(family),
    m_fontPath(path),
    m_filename(filename)
{
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

const String& ScoreFont::name() const
{
    return m_name;
}

const String& ScoreFont::family() const
{
    return m_family;
}

const String& ScoreFont::fontPath() const
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
    IF_ASSERT_FAILED(!glyphNamesJson.empty()) {
        LOGE() << "Could not read glyph names JSON";
        return;
    }

    for (size_t i = 0; i < s_symIdCodes.size(); ++i) {
        String name(SymNames::nameForSymId(static_cast<SymId>(i)).toQLatin1String());

        bool ok;
        uint code = glyphNamesJson.value(name).toObject().value("codepoint").toString().midRef(2).toUInt(&ok, 16);
        if (ok) {
            s_symIdCodes[i] = code;
        } else if (MScore::debugMode) {
            LOGD() << "could not read codepoint for glyph " << name;
        }
    }

    fontProvider()->insertSubstitution(u"Leland Text",    u"Bravura Text");
    fontProvider()->insertSubstitution(u"Bravura Text",   u"Leland Text");
    fontProvider()->insertSubstitution(u"MScore Text",    u"Leland Text");
    fontProvider()->insertSubstitution(u"Gootville Text", u"Leland Text");
    fontProvider()->insertSubstitution(u"MuseJazz Text",  u"Leland Text");
    fontProvider()->insertSubstitution(u"Petaluma Text",  u"MuseJazz Text");
    fontProvider()->insertSubstitution(u"ScoreFont",      u"Leland Text"); // alias for current Musical Text Font

    fallbackFont(); // load fallback font
}

QJsonObject ScoreFont::initGlyphNamesJson()
{
    File file(":fonts/smufl/glyphnames.json");
    if (!file.open(IODevice::ReadOnly)) {
        LOGE() << "could not open glyph names JSON file.";
        return QJsonObject();
    }

    QJsonParseError error;
    QJsonObject glyphNamesJson = QJsonDocument::fromJson(file.readAll().toQByteArray(), &error).object();
    file.close();

    if (error.error != QJsonParseError::NoError) {
        LOGE() << "JSON parse error in glyph names file: " << error.errorString()
               << " (offset: " << error.offset << ")";
        return QJsonObject();
    }

    return glyphNamesJson;
}

// =============================================
// Available ScoreFonts
// =============================================

const std::vector<ScoreFont>& ScoreFont::scoreFonts()
{
    return s_scoreFonts;
}

ScoreFont* ScoreFont::fontByName(const String& name)
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
    String facePath = m_fontPath + m_filename;
    if (-1 == fontProvider()->addApplicationFont(m_family, facePath)) {
        LOGE() << "fatal error: cannot load internal font: " << facePath;
        return;
    }

    m_font.setWeight(mu::draw::Font::Normal);
    m_font.setItalic(false);
    m_font.setFamily(m_family);
    m_font.setNoFontMerging(true);
    m_font.setHinting(mu::draw::Font::Hinting::PreferVerticalHinting);

    for (size_t id = 0; id < s_symIdCodes.size(); ++id) {
        uint code = s_symIdCodes[id];
        if (code == 0) {
            continue;
        }
        Sym& sym = m_symbols[id];
        computeMetrics(sym, code);
    }

    File metadataFile(m_fontPath + u"metadata.json");
    if (!metadataFile.open(IODevice::ReadOnly)) {
        LOGE() << "Failed to open glyph metadata file: " << metadataFile.filePath();
        return;
    }

    QJsonParseError error;
    QJsonObject metadataJson = QJsonDocument::fromJson(metadataFile.readAll().toQByteArray(), &error).object();
    if (error.error != QJsonParseError::NoError) {
        LOGE() << "Json parse error in " << metadataFile.filePath()
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
    for (const String& symName : glyphsWithAnchors.keys()) {
        SymId symId = SymNames::symIdByName(symName);
        if (symId == SymId::noSym) {
            //! NOTE currently, Bravura contains a bunch of entries in glyphsWithAnchors
            //! for glyph names that will not be found - flag32ndUpStraight, etc.
            continue;
        }

        Sym& sym = this->sym(symId);
        QJsonObject anchors = glyphsWithAnchors.value(symName).toObject();

        static const std::map<String, SmuflAnchorId> smuflAnchorIdNames {
            { u"stemDownNW", SmuflAnchorId::stemDownNW },
            { u"stemUpSE", SmuflAnchorId::stemUpSE },
            { u"stemDownSW", SmuflAnchorId::stemDownSW },
            { u"stemUpNW", SmuflAnchorId::stemUpNW },
            { u"cutOutNE", SmuflAnchorId::cutOutNE },
            { u"cutOutNW", SmuflAnchorId::cutOutNW },
            { u"cutOutSE", SmuflAnchorId::cutOutSE },
            { u"cutOutSW", SmuflAnchorId::cutOutSW },
            { u"opticalCenter", SmuflAnchorId::opticalCenter },
        };

        for (const QString& anchorId : anchors.keys()) {
            auto search = smuflAnchorIdNames.find(anchorId);
            if (search == smuflAnchorIdNames.cend()) {
                //LOGD() << "Unhandled SMuFL anchorId: " << anchorId;
                continue;
            }

            QJsonArray arr = anchors.value(anchorId).toArray();
            double x = arr.at(0).toDouble();
            double y = arr.at(1).toDouble();

            sym.smuflAnchors[search->second] = PointF(x, -y) * SPATIUM20;
        }
    }
}

void ScoreFont::loadComposedGlyphs()
{
    static const struct ComposedGlyph {
        const SymId id;
        const SymIdList subSymbolIds;
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
        { SymId::ornamentDownMordent, {
              SymId::ornamentLeftVerticalStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
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
        Sym& sym = this->sym(c.id);
        if (!sym.isValid()) {
            sym.subSymbolIds = c.subSymbolIds;
            sym.bbox = bbox(c.subSymbolIds, 1.0);
        }
    }
}

void ScoreFont::loadStylisticAlternates(const QJsonObject& glyphsWithAlternatesObject)
{
    static const struct GlyphWithAlternates {
        const String key;
        const String alternateKey;
        const SymId alternateSymId;
    } glyphsWithAlternates[] = {
        { String("4stringTabClef"),
          String("4stringTabClefSerif"),
          SymId::fourStringTabClefSerif
        },
        { String("6stringTabClef"),
          String("6stringTabClefSerif"),
          SymId::sixStringTabClefSerif
        },
        { String("cClef"),
          String("cClefFrench"),
          SymId::cClefFrench
        },
        { String("cClef"),
          String("cClefFrench20C"),
          SymId::cClefFrench20C
        },
        { String("fClef"),
          String("fClefFrench"),
          SymId::fClefFrench
        },
        { String("fClef"),
          String("fClef19thCentury"),
          SymId::fClef19thCentury
        },
        { String("noteheadBlack"),
          String("noteheadBlackOversized"),
          SymId::noteheadBlack
        },
        { String("noteheadHalf"),
          String("noteheadHalfOversized"),
          SymId::noteheadHalf
        },
        { String("noteheadWhole"),
          String("noteheadWholeOversized"),
          SymId::noteheadWhole
        },
        { String("noteheadDoubleWhole"),
          String("noteheadDoubleWholeOversized"),
          SymId::noteheadDoubleWhole
        },
        { String("noteheadDoubleWholeSquare"),
          String("noteheadDoubleWholeSquareOversized"),
          SymId::noteheadDoubleWholeSquare
        },
        { String("noteheadDoubleWhole"),
          String("noteheadDoubleWholeAlt"),
          SymId::noteheadDoubleWholeAlt
        },
        { String("brace"),
          String("braceSmall"),
          SymId::braceSmall
        },
        { String("brace"),
          String("braceLarge"),
          SymId::braceLarge
        },
        { String("brace"),
          String("braceLarger"),
          SymId::braceLarger
        },
        { String("flag1024thDown"),
          String("flag1024thDownStraight"),
          SymId::flag1024thDownStraight
        },
        { String("flag1024thUp"),
          String("flag1024thUpStraight"),
          SymId::flag1024thUpStraight
        },
        { String("flag128thDown"),
          String("flag128thDownStraight"),
          SymId::flag128thDownStraight
        },
        { String("flag128thUp"),
          String("flag128thUpStraight"),
          SymId::flag128thUpStraight
        },
        { String("flag16thDown"),
          String("flag16thDownStraight"),
          SymId::flag16thDownStraight
        },
        { String("flag16thUp"),
          String("flag16thUpStraight"),
          SymId::flag16thUpStraight
        },
        { String("flag256thDown"),
          String("flag256thDownStraight"),
          SymId::flag256thDownStraight
        },
        { String("flag256thUp"),
          String("flag256thUpStraight"),
          SymId::flag256thUpStraight
        },
        { String("flag32ndDown"),
          String("flag32ndDownStraight"),
          SymId::flag32ndDownStraight
        },
        { String("flag32ndUp"),
          String("flag32ndUpStraight"),
          SymId::flag32ndUpStraight
        },
        { String("flag512thDown"),
          String("flag512thDownStraight"),
          SymId::flag512thDownStraight
        },
        { String("flag512thUp"),
          String("flag512thUpStraight"),
          SymId::flag512thUpStraight
        },
        { String("flag64thDown"),
          String("flag64thDownStraight"),
          SymId::flag64thDownStraight
        },
        { String("flag64thUp"),
          String("flag64thUpStraight"),
          SymId::flag64thUpStraight
        },
        { String("flag8thDown"),
          String("flag8thDownStraight"),
          SymId::flag8thDownStraight
        },
        { String("flag8thUp"),
          String("flag8thUpStraight"),
          SymId::flag8thUpStraight
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
                return value.toObject().value("name") == glyph.alternateKey.toQString();
            });

            if (alternateIt != alternatesArray.cend()) {
                Sym& sym = this->sym(glyph.alternateSymId);
                uint code = alternateIt->toObject().value("codepoint").toString().midRef(2).toUInt(&ok, 16);
                if (ok) {
                    computeMetrics(sym, code);
                }
            }
        }
    }
}

void ScoreFont::loadEngravingDefaults(const QJsonObject& engravingDefaultsObject)
{
    static const std::list<std::pair<String, Sid> > engravingDefaultsMapping = {
        { u"staffLineThickness",            Sid::staffLineWidth },
        { u"stemThickness",                 Sid::stemWidth },
        { u"beamThickness",                 Sid::beamWidth },
        { u"beamSpacing",                   Sid::useWideBeams },
        { u"legerLineThickness",            Sid::ledgerLineWidth },
        { u"legerLineExtension",            Sid::ledgerLineLength },
        { u"slurEndpointThickness",         Sid::SlurEndWidth },
        { u"slurMidpointThickness",         Sid::SlurMidWidth },
        { u"thinBarlineThickness",          Sid::barWidth },
        { u"thinBarlineThickness",          Sid::doubleBarWidth },
        { u"thickBarlineThickness",         Sid::endBarWidth },
        { u"dashedBarlineThickness",        Sid::barWidth },
        { u"barlineSeparation",             Sid::doubleBarDistance },
        { u"barlineSeparation",             Sid::endBarDistance },
        { u"repeatBarlineDotSeparation",    Sid::repeatBarlineDotSeparation },
        { u"bracketThickness",              Sid::bracketWidth },
        { u"hairpinThickness",              Sid::hairpinLineWidth },
        { u"octaveLineThickness",           Sid::ottavaLineWidth },
        { u"pedalLineThickness",            Sid::pedalLineWidth },
        { u"repeatEndingLineThickness",     Sid::voltaLineWidth },
        { u"lyricLineThickness",            Sid::lyricsLineThickness },
        { u"tupletBracketThickness",        Sid::tupletBracketWidth }
    };

    for (const String& key : engravingDefaultsObject.keys()) {
        if (key == "textEnclosureThickness") {
            m_textEnclosureThickness = engravingDefaultsObject.value(key).toDouble();
            continue;
        }

        for (auto mapping : engravingDefaultsMapping) {
            if (key == mapping.first) {
                qreal value = engravingDefaultsObject.value(key).toDouble();

                if (key == "beamSpacing") {
                    value = value > 0.75;
                }

                m_engravingDefaults.push_back({ mapping.second, value });
            }
        }
    }

    m_engravingDefaults.push_back({ Sid::MusicalTextFont, QString("%1 Text").arg(m_family.toQString()) });
}

void ScoreFont::computeMetrics(ScoreFont::Sym& sym, uint code)
{
    sym.code = code;
    sym.bbox = fontProvider()->symBBox(m_font, code, DPI_F);
    sym.advance = fontProvider()->symAdvance(m_font, code, DPI_F);
}

// =============================================
// Symbol properties
// =============================================

ScoreFont::Sym& ScoreFont::sym(SymId id)
{
    return m_symbols[static_cast<size_t>(id)];
}

const ScoreFont::Sym& ScoreFont::sym(SymId id) const
{
    return m_symbols.at(static_cast<size_t>(id));
}

uint ScoreFont::symCode(SymId id) const
{
    const Sym& s = sym(id);
    if (s.isValid()) {
        return s.code;
    }

    // fallback: search in the common SMuFL table
    return s_symIdCodes.at(static_cast<size_t>(id));
}

SymId ScoreFont::fromCode(uint code) const
{
    auto it = std::find_if(m_symbols.begin(), m_symbols.end(), [code](const Sym& s) { return s.code == code; });
    return static_cast<SymId>(it == m_symbols.end() ? 0 : it - m_symbols.begin());
}

static String codeToString(char32_t code)
{
    return String::fromUcs4(&code, 1);
}

String ScoreFont::toString(SymId id) const
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
// Symbol bounding box
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

    RectF r = sym(id).bbox;
    return RectF(r.x() * mag.width(), r.y() * mag.height(),
                 r.width() * mag.width(), r.height() * mag.height());
}

const RectF ScoreFont::bbox(const SymIdList& s, qreal mag) const
{
    return bbox(s, SizeF(mag, mag));
}

const RectF ScoreFont::bbox(const SymIdList& s, const SizeF& mag) const
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
    return bbox(id, mag).height();
}

qreal ScoreFont::advance(SymId id, qreal mag) const
{
    if (useFallbackFont(id)) {
        return fallbackFont()->advance(id, mag);
    }

    return sym(id).advance * mag;
}

qreal ScoreFont::width(const SymIdList& s, qreal mag) const
{
    return bbox(s, mag).width();
}

PointF ScoreFont::smuflAnchor(SymId symId, SmuflAnchorId anchorId, qreal mag) const
{
    if (useFallbackFont(symId)) {
        return fallbackFont()->smuflAnchor(symId, anchorId, mag);
    }

    return const_cast<Sym&>(sym(symId)).smuflAnchors[anchorId] * mag;
}

// =============================================
// Draw
// =============================================

void ScoreFont::draw(SymId id, Painter* painter, const SizeF& mag, const PointF& pos) const
{
    const Sym& sym = this->sym(id);
    if (sym.isCompound()) { // is this a compound symbol?
        draw(sym.subSymbolIds, painter, mag, pos);
        return;
    }

    if (!sym.isValid()) {
        if (MScore::useFallbackFont && this != ScoreFont::fallbackFont()) {
            fallbackFont()->draw(id, painter, mag, pos);
        } else {
            LOGE() << "invalid sym: " << static_cast<size_t>(id);
        }

        return;
    }

    painter->save();
    qreal size = 20.0 * MScore::pixelRatio;
    m_font.setPointSizeF(size);
    painter->scale(mag.width(), mag.height());
    painter->setFont(m_font);
    painter->drawSymbol(PointF(pos.x() / mag.width(), pos.y() / mag.height()), symCode(id));
    painter->restore();
}

void ScoreFont::draw(SymId id, Painter* painter, qreal mag, const PointF& pos) const
{
    draw(id, painter, SizeF(mag, mag), pos);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, qreal mag, const PointF& pos, int n) const
{
    SymIdList list(n, id);
    draw(list, painter, mag, pos);
}

void ScoreFont::draw(const SymIdList& ids, Painter* painter, qreal mag, const PointF& startPos) const
{
    PointF pos(startPos);
    for (SymId id : ids) {
        draw(id, painter, mag, pos);
        pos.setX(pos.x() + advance(id, mag));
    }
}

void ScoreFont::draw(const SymIdList& ids, Painter* painter, const SizeF& mag, const PointF& startPos) const
{
    PointF pos(startPos);
    for (SymId id : ids) {
        draw(id, painter, mag, pos);
        pos.setX(pos.x() + advance(id, mag.width()));
    }
}
