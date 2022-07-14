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

#include "serialization/json.h"
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
    ScoreFont("Finale Maestro", "Finale Maestro", ":/fonts/finalemaestro/", "FinaleMaestro.otf"),
    ScoreFont("Finale Broadway", "Finale Broadway", ":/fonts/finalebroadway/", "FinaleBroadway.otf"),
};

std::array<ScoreFont::Code, size_t(SymId::lastSym) + 1> ScoreFont::s_symIdCodes { {  } };

// =============================================
// ScoreFont
// =============================================

ScoreFont::ScoreFont(const char* name, const char* family, const char* path, const char* filename)
    : m_symbols(static_cast<size_t>(SymId::lastSym) + 1),
    m_name(String::fromUtf8(name)),
    m_family(String::fromUtf8(family)),
    m_fontPath(String::fromUtf8(path)),
    m_filename(String::fromUtf8(filename))
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

std::unordered_map<Sid, PropertyValue> ScoreFont::engravingDefaults()
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
    initGlyphNamesJson();

    fontProvider()->insertSubstitution(u"Leland Text",    u"Bravura Text");
    fontProvider()->insertSubstitution(u"Bravura Text",   u"Leland Text");
    fontProvider()->insertSubstitution(u"MScore Text",    u"Leland Text");
    fontProvider()->insertSubstitution(u"Gootville Text", u"Leland Text");
    fontProvider()->insertSubstitution(u"MuseJazz Text",  u"Leland Text");
    fontProvider()->insertSubstitution(u"Petaluma Text",  u"MuseJazz Text");
    fontProvider()->insertSubstitution(u"Finale Maestro Text", u"Leland Text");
    fontProvider()->insertSubstitution(u"Finale Broadway Text", u"MuseJazz Text");
    fontProvider()->insertSubstitution(u"ScoreFont",      u"Leland Text"); // alias for current Musical Text Font

    fallbackFont(); // load fallback font
}

bool ScoreFont::initGlyphNamesJson()
{
    File file(":fonts/smufl/glyphnames.json");
    if (!file.open(IODevice::ReadOnly)) {
        LOGE() << "could not open glyph names JSON file.";
        return false;
    }

    std::string error;
    JsonObject glyphNamesJson = JsonDocument::fromJson(file.readAll(), &error).rootObject();
    file.close();

    if (!error.empty()) {
        LOGE() << "JSON parse error in glyph names file: " << error;
        return false;
    }

    IF_ASSERT_FAILED(!glyphNamesJson.empty()) {
        LOGE() << "Could not read glyph names JSON";
        return false;
    }

    for (size_t i = 0; i < s_symIdCodes.size(); ++i) {
        SymId sym = static_cast<SymId>(i);
        if (sym == SymId::noSym || sym == SymId::lastSym) {
            continue;
        }

        std::string name(SymNames::nameForSymId(sym).ascii());
        JsonObject symObj = glyphNamesJson.value(name).toObject();
        if (!symObj.isValid()) {
            continue;
        }

        bool ok;
        uint code = symObj.value("codepoint").toString().mid(2).toUInt(&ok, 16);
        if (ok) {
            s_symIdCodes[i].smuflCode = code;
        } else if (MScore::debugMode) {
            LOGD() << "could not read codepoint for glyph " << name;
        }

        uint alernativeCode = symObj.value("alternateCodepoint").toString().mid(2).toUInt(&ok, 16);
        if (ok) {
            s_symIdCodes[i].musicSymBlockCode = alernativeCode;
        } else if (MScore::debugMode) {
            LOGD() << "could not read alternate codepoint for glyph " << name;
        }
    }
    return true;
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
        Code code = s_symIdCodes[id];
        if (code.smuflCode == 0 && code.musicSymBlockCode == 0) {
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

    std::string error;
    JsonObject metadataJson = JsonDocument::fromJson(metadataFile.readAll(), &error).rootObject();
    if (!error.empty()) {
        LOGE() << "Json parse error in " << metadataFile.filePath() << ", error: " << error;
        return;
    }

    loadGlyphsWithAnchors(metadataJson.value("glyphsWithAnchors").toObject());
    loadComposedGlyphs();
    loadStylisticAlternates(metadataJson.value("glyphsWithAlternates").toObject());
    loadEngravingDefaults(metadataJson.value("engravingDefaults").toObject());

    m_loaded = true;
}

void ScoreFont::loadGlyphsWithAnchors(const JsonObject& glyphsWithAnchors)
{
    for (const std::string& symName : glyphsWithAnchors.keys()) {
        SymId symId = SymNames::symIdByName(symName);
        if (symId == SymId::noSym) {
            //! NOTE currently, Bravura contains a bunch of entries in glyphsWithAnchors
            //! for glyph names that will not be found - flag32ndUpStraight, etc.
            continue;
        }

        Sym& sym = this->sym(symId);
        JsonObject anchors = glyphsWithAnchors.value(symName).toObject();

        static const std::unordered_map<std::string, SmuflAnchorId> smuflAnchorIdNames {
            { "stemDownNW", SmuflAnchorId::stemDownNW },
            { "stemUpSE", SmuflAnchorId::stemUpSE },
            { "stemDownSW", SmuflAnchorId::stemDownSW },
            { "stemUpNW", SmuflAnchorId::stemUpNW },
            { "cutOutNE", SmuflAnchorId::cutOutNE },
            { "cutOutNW", SmuflAnchorId::cutOutNW },
            { "cutOutSE", SmuflAnchorId::cutOutSE },
            { "cutOutSW", SmuflAnchorId::cutOutSW },
            { "opticalCenter", SmuflAnchorId::opticalCenter },
        };

        for (const std::string& anchorId : anchors.keys()) {
            auto search = smuflAnchorIdNames.find(anchorId);
            if (search == smuflAnchorIdNames.cend()) {
                //LOGD() << "Unhandled SMuFL anchorId: " << anchorId;
                continue;
            }

            JsonArray arr = anchors.value(anchorId).toArray();
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

void ScoreFont::loadStylisticAlternates(const JsonObject& glyphsWithAlternatesObject)
{
    if (!glyphsWithAlternatesObject.isValid()) {
        return;
    }

    static const struct GlyphWithAlternates {
        const std::string key;
        const std::string alternateKey;
        const SymId alternateSymId;
    } glyphsWithAlternates[] = {
        { std::string("4stringTabClef"),
          std::string("4stringTabClefSerif"),
          SymId::fourStringTabClefSerif
        },
        { std::string("6stringTabClef"),
          std::string("6stringTabClefSerif"),
          SymId::sixStringTabClefSerif
        },
        { std::string("cClef"),
          std::string("cClefFrench"),
          SymId::cClefFrench
        },
        { std::string("cClef"),
          std::string("cClefFrench20C"),
          SymId::cClefFrench20C
        },
        { std::string("fClef"),
          std::string("fClefFrench"),
          SymId::fClefFrench
        },
        { std::string("fClef"),
          std::string("fClef19thCentury"),
          SymId::fClef19thCentury
        },
        { std::string("noteheadBlack"),
          std::string("noteheadBlackOversized"),
          SymId::noteheadBlack
        },
        { std::string("noteheadHalf"),
          std::string("noteheadHalfOversized"),
          SymId::noteheadHalf
        },
        { std::string("noteheadWhole"),
          std::string("noteheadWholeOversized"),
          SymId::noteheadWhole
        },
        { std::string("noteheadDoubleWhole"),
          std::string("noteheadDoubleWholeOversized"),
          SymId::noteheadDoubleWhole
        },
        { std::string("noteheadDoubleWholeSquare"),
          std::string("noteheadDoubleWholeSquareOversized"),
          SymId::noteheadDoubleWholeSquare
        },
        { std::string("noteheadDoubleWhole"),
          std::string("noteheadDoubleWholeAlt"),
          SymId::noteheadDoubleWholeAlt
        },
        { std::string("brace"),
          std::string("braceSmall"),
          SymId::braceSmall
        },
        { std::string("brace"),
          std::string("braceLarge"),
          SymId::braceLarge
        },
        { std::string("brace"),
          std::string("braceLarger"),
          SymId::braceLarger
        },
        { std::string("flag1024thDown"),
          std::string("flag1024thDownStraight"),
          SymId::flag1024thDownStraight
        },
        { std::string("flag1024thUp"),
          std::string("flag1024thUpStraight"),
          SymId::flag1024thUpStraight
        },
        { std::string("flag128thDown"),
          std::string("flag128thDownStraight"),
          SymId::flag128thDownStraight
        },
        { std::string("flag128thUp"),
          std::string("flag128thUpStraight"),
          SymId::flag128thUpStraight
        },
        { std::string("flag16thDown"),
          std::string("flag16thDownStraight"),
          SymId::flag16thDownStraight
        },
        { std::string("flag16thUp"),
          std::string("flag16thUpStraight"),
          SymId::flag16thUpStraight
        },
        { std::string("flag256thDown"),
          std::string("flag256thDownStraight"),
          SymId::flag256thDownStraight
        },
        { std::string("flag256thUp"),
          std::string("flag256thUpStraight"),
          SymId::flag256thUpStraight
        },
        { std::string("flag32ndDown"),
          std::string("flag32ndDownStraight"),
          SymId::flag32ndDownStraight
        },
        { std::string("flag32ndUp"),
          std::string("flag32ndUpStraight"),
          SymId::flag32ndUpStraight
        },
        { std::string("flag512thDown"),
          std::string("flag512thDownStraight"),
          SymId::flag512thDownStraight
        },
        { std::string("flag512thUp"),
          std::string("flag512thUpStraight"),
          SymId::flag512thUpStraight
        },
        { std::string("flag64thDown"),
          std::string("flag64thDownStraight"),
          SymId::flag64thDownStraight
        },
        { std::string("flag64thUp"),
          std::string("flag64thUpStraight"),
          SymId::flag64thUpStraight
        },
        { std::string("flag8thDown"),
          std::string("flag8thDownStraight"),
          SymId::flag8thDownStraight
        },
        { std::string("flag8thUp"),
          std::string("flag8thUpStraight"),
          SymId::flag8thUpStraight
        }
    };

    bool ok;
    for (const GlyphWithAlternates& glyph : glyphsWithAlternates) {
        if (glyphsWithAlternatesObject.contains(glyph.key)) {
            const JsonArray alternatesArray = glyphsWithAlternatesObject.value(glyph.key).toObject().value("alternates").toArray();

            JsonValue val;
            for (size_t i = 0; i < alternatesArray.size(); ++i) {
                JsonValue v = alternatesArray.at(i);
                if (v.toObject().value("name").toStdString() == glyph.alternateKey) {
                    val = v;
                    break;
                }
            }

            if (!val.isNull()) {
                JsonObject symObj = val.toObject();
                Sym& sym = this->sym(glyph.alternateSymId);

                Code code;
                uint smuflCode = symObj.value("codepoint").toString().mid(2).toUInt(&ok, 16);
                if (ok) {
                    code.smuflCode = smuflCode;
                }

                uint musicSymBlockCode = symObj.value("alternateCodepoint").toString().mid(2).toUInt(&ok, 16);
                if (ok) {
                    code.musicSymBlockCode = musicSymBlockCode;
                }

                if (code.smuflCode || code.musicSymBlockCode) {
                    computeMetrics(sym, code);
                }
            }
        }
    }
}

void ScoreFont::loadEngravingDefaults(const JsonObject& engravingDefaultsObject)
{
    struct EngravingDefault {
        std::vector<Sid> sids;

        // If a childKey is not specified in `engravingDefaultsObject`,
        // it will receive the value for the key of `this` EngravingDefault.
        // This is done for compatibility with fonts made for older SMuFL versions:
        // in newer versions, some settings have been split into two.
        std::vector<std::string> childKeys = {};

        EngravingDefault(const std::vector<Sid>& sids)
            : sids(sids), childKeys() {}
        EngravingDefault(const std::vector<Sid>& sids, const std::vector<std::string>& childKeys)
            : sids(sids), childKeys(childKeys) {}
    };

    // https://w3c.github.io/smufl/latest/specification/engravingdefaults.html
    static const std::unordered_map<std::string, EngravingDefault> engravingDefaultsMapping = {
        // "textFontFamily" not supported
        { "staffLineThickness",         { { Sid::staffLineWidth } } },
        { "stemThickness",              { { Sid::stemWidth } } },
        { "beamThickness",              { { Sid::beamWidth } } },
        // "beamSpacing" handled separately
        { "legerLineThickness",         { { Sid::ledgerLineWidth } } },
        { "legerLineExtension",         { { Sid::ledgerLineLength } } },
        { "slurEndpointThickness",      { { Sid::SlurEndWidth } } },
        { "slurMidpointThickness",      { { Sid::SlurMidWidth } } },
        // "tieEndpointThickness" not supported
        // "tieMidpointThickness" not supported
        { "thinBarlineThickness",       { { Sid::barWidth, Sid::doubleBarWidth } } },
        { "thickBarlineThickness",      { { Sid::endBarWidth } } },
        // "dashedBarlineThickness" not supported
        // "dashedBarlineDashLength" not supported
        // "dashedBarlineGapLength" not supported
        { "barlineSeparation",          { { Sid::doubleBarDistance }, { "thinThickBarlineSeparation" } } },
        { "thinThickBarlineSeparation", { { Sid::endBarDistance } } },
        { "repeatBarlineDotSeparation", { { Sid::repeatBarlineDotSeparation } } },
        { "bracketThickness",           { { Sid::bracketWidth } } },
        // "subBracketThickness" not supported
        { "hairpinThickness",           { { Sid::hairpinLineWidth } } },
        { "octaveLineThickness",        { { Sid::ottavaLineWidth } } },
        { "pedalLineThickness",         { { Sid::pedalLineWidth } } },
        { "repeatEndingLineThickness",  { { Sid::voltaLineWidth } } },
        // "arrowShaftThickness" not supported
        { "lyricLineThickness",         { { Sid::lyricsLineThickness } } },
        // "textEnclosureThickness" handled separately
        { "tupletBracketThickness",     { { Sid::tupletBracketWidth } } },
        { "hBarThickness",              { { Sid::mmRestHBarThickness } } }
    };

    std::function<void(const std::string& key, const PropertyValue& value)> applyEngravingDefault;

    applyEngravingDefault = [&](const std::string& key, const PropertyValue& value) {
        auto search = engravingDefaultsMapping.find(key);
        if (search != engravingDefaultsMapping.cend()) {
            for (Sid sid : search->second.sids) {
                m_engravingDefaults.insert({ sid, value });
            }

            for (const std::string& childKey : search->second.childKeys) {
                if (!engravingDefaultsObject.contains(childKey)) {
                    applyEngravingDefault(childKey, value);
                }
            }
        }
    };

    for (const std::string& key : engravingDefaultsObject.keys()) {
        if (key == "textEnclosureThickness") {
            m_textEnclosureThickness = engravingDefaultsObject.value(key).toDouble();
            continue;
        }

        if (key == "beamSpacing") {
            bool value = engravingDefaultsObject.value(key).toDouble() > 0.75;
            m_engravingDefaults.insert({ Sid::useWideBeams, value });
            continue;
        }

        applyEngravingDefault(key, engravingDefaultsObject.value(key).toDouble());
    }

    m_engravingDefaults.insert({ Sid::MusicalTextFont, String(u"%1 Text").arg(m_family) });
}

void ScoreFont::computeMetrics(ScoreFont::Sym& sym, const Code& code)
{
    if (fontProvider()->inFontUcs4(m_font, code.smuflCode)) {
        sym.code = code.smuflCode;
    } else if (fontProvider()->inFontUcs4(m_font, code.musicSymBlockCode)) {
        sym.code = code.musicSymBlockCode;
    }

    if (sym.code > 0) {
        sym.bbox = fontProvider()->symBBox(m_font, sym.code, DPI_F);
        sym.advance = fontProvider()->symAdvance(m_font, sym.code, DPI_F);
    }
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
    return s_symIdCodes.at(static_cast<size_t>(id)).smuflCode;
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

const RectF ScoreFont::bbox(SymId id, double mag) const
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

const RectF ScoreFont::bbox(const SymIdList& s, double mag) const
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

double ScoreFont::width(SymId id, double mag) const
{
    return bbox(id, mag).width();
}

double ScoreFont::height(SymId id, double mag) const
{
    return bbox(id, mag).height();
}

double ScoreFont::advance(SymId id, double mag) const
{
    if (useFallbackFont(id)) {
        return fallbackFont()->advance(id, mag);
    }

    return sym(id).advance * mag;
}

double ScoreFont::width(const SymIdList& s, double mag) const
{
    return bbox(s, mag).width();
}

PointF ScoreFont::smuflAnchor(SymId symId, SmuflAnchorId anchorId, double mag) const
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
    double size = 20.0 * MScore::pixelRatio;
    m_font.setPointSizeF(size);
    painter->scale(mag.width(), mag.height());
    painter->setFont(m_font);
    painter->drawSymbol(PointF(pos.x() / mag.width(), pos.y() / mag.height()), symCode(id));
    painter->restore();
}

void ScoreFont::draw(SymId id, Painter* painter, double mag, const PointF& pos) const
{
    draw(id, painter, SizeF(mag, mag), pos);
}

void ScoreFont::draw(SymId id, mu::draw::Painter* painter, double mag, const PointF& pos, int n) const
{
    SymIdList list(n, id);
    draw(list, painter, mag, pos);
}

void ScoreFont::draw(const SymIdList& ids, Painter* painter, double mag, const PointF& startPos) const
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
