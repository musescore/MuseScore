/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "engravingfont.h"

#include "serialization/json.h"
#include "io/file.h"
#include "io/fileinfo.h"
#include "draw/painter.h"
#include "types/symnames.h"

#include "dom/mscore.h"

#include "smufl.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace muse::draw;
using namespace mu::engraving;

// =============================================
// ScoreFont
// =============================================

EngravingFont::EngravingFont(const std::string& name, const std::string& family, const path_t& filePath,
                             const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx),  m_symbols(static_cast<size_t>(SymId::lastSym) + 1),
    m_name(name),
    m_family(family),
    m_fontPath(filePath)
{
}

EngravingFont::EngravingFont(const EngravingFont& other)
    : muse::Injectable(other.iocContext())
{
    m_loaded = false;
    m_symbols  = other.m_symbols;
    m_name     = other.m_name;
    m_family   = other.m_family;
    m_fontPath = other.m_fontPath;
}

// =============================================
// Properties
// =============================================

const std::string& EngravingFont::name() const
{
    return m_name;
}

const std::string& EngravingFont::family() const
{
    return m_family;
}

std::unordered_map<Sid, PropertyValue> EngravingFont::engravingDefaults() const
{
    return m_engravingDefaults;
}

double EngravingFont::textEnclosureThickness()
{
    return m_textEnclosureThickness;
}

// =============================================
// Load
// =============================================

void EngravingFont::ensureLoad()
{
    if (m_loaded) {
        return;
    }

    if (-1 == fontProvider()->addSymbolFont(String::fromStdString(m_family), m_fontPath)) {
        LOGE() << "fatal error: cannot load internal font: " << m_fontPath;
        return;
    }

    m_font.setWeight(Font::Normal);
    m_font.setItalic(false);
    m_font.setFamily(String::fromStdString(m_family), Font::Type::MusicSymbol);
    m_font.setNoFontMerging(true);
    m_font.setHinting(Font::Hinting::PreferVerticalHinting);

    for (size_t id = 0; id < m_symbols.size(); ++id) {
        Smufl::Code code = Smufl::code(static_cast<SymId>(id));
        if (!code.isValid()) {
            continue;
        }
        Sym& sym = m_symbols[id];
        computeMetrics(sym, code);
    }

    File metadataFile(FileInfo(m_fontPath).path() + u"/metadata.json");
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

void EngravingFont::loadGlyphsWithAnchors(const JsonObject& glyphsWithAnchors)
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

void EngravingFont::loadComposedGlyphs()
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

void EngravingFont::loadStylisticAlternates(const JsonObject& glyphsWithAlternatesObject)
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

                Smufl::Code code;
                char32_t smuflCode = symObj.value("codepoint").toString().mid(2).toUInt(&ok, 16);
                if (ok) {
                    code.smuflCode = smuflCode;
                }

                char32_t musicSymBlockCode = symObj.value("alternateCodepoint").toString().mid(2).toUInt(&ok, 16);
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

void EngravingFont::loadEngravingDefaults(const JsonObject& engravingDefaultsObject)
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
        { "slurEndpointThickness",      { { Sid::slurEndWidth } } },
        { "slurMidpointThickness",      { { Sid::slurMidWidth } } },
        { "tieEndpointThickness",       { { Sid::tieEndWidth } } },
        { "tieMidpointThickness",       { { Sid::tieMidWidth } } },
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

    m_engravingDefaults.insert({ Sid::musicalTextFont, String(u"%1 Text").arg(String::fromStdString(m_family)) });
}

void EngravingFont::computeMetrics(EngravingFont::Sym& sym, const Smufl::Code& code)
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

EngravingFont::Sym& EngravingFont::sym(SymId id)
{
    return m_symbols[static_cast<size_t>(id)];
}

const EngravingFont::Sym& EngravingFont::sym(SymId id) const
{
    return m_symbols.at(static_cast<size_t>(id));
}

char32_t EngravingFont::symCode(SymId id) const
{
    const Sym& s = sym(id);
    if (s.isValid()) {
        return s.code;
    }

    // fallback: search in the common SMuFL table
    return Smufl::smuflCode(id);
}

SymId EngravingFont::fromCode(char32_t code) const
{
    auto it = std::find_if(m_symbols.begin(), m_symbols.end(), [code](const Sym& s) { return s.code == code; });
    return static_cast<SymId>(it == m_symbols.end() ? 0 : it - m_symbols.begin());
}

String EngravingFont::toString(SymId id) const
{
    return String::fromUcs4(symCode(id));
}

bool EngravingFont::isValid(SymId id) const
{
    return sym(id).isValid();
}

bool EngravingFont::useFallbackFont(SymId id) const
{
    return MScore::useFallbackFont && !sym(id).isValid() && !engravingFonts()->isFallbackFont(this);
}

// =============================================
// Symbol bounding box
// =============================================

RectF EngravingFont::bbox(SymId id, double mag) const
{
    return bbox(id, SizeF(mag, mag));
}

RectF EngravingFont::bbox(SymId id, const SizeF& mag) const
{
    if (useFallbackFont(id)) {
        return engravingFonts()->fallbackFont()->bbox(id, mag);
    }

    RectF r = sym(id).bbox;
    return r.scale(mag);
}

RectF EngravingFont::bbox(const SymIdList& s, double mag) const
{
    return bbox(s, SizeF(mag, mag));
}

RectF EngravingFont::bbox(const SymIdList& s, const SizeF& mag) const
{
    RectF r;
    PointF pos;
    for (SymId id : s) {
        r.unite(bbox(id, mag).translated(pos));
        pos.rx() += advance(id, mag.width());
    }
    return r;
}

Shape EngravingFont::shape(const SymIdList& s, double mag) const
{
    return shape(s, SizeF(mag, mag));
}

Shape EngravingFont::shape(const SymIdList& s, const SizeF& mag) const
{
    Shape sh;
    PointF pos(0.0, 0.0);
    for (SymId id : s) {
        sh.add(Shape(bbox(id, mag)).translate(pos));
        pos.rx() += advance(id, mag.width());
    }
    return sh;
}

Shape EngravingFont::shapeWithCutouts(SymId id, double mag)
{
    return shapeWithCutouts(id, SizeF(mag, mag));
}

Shape EngravingFont::shapeWithCutouts(SymId id, const SizeF& mag)
{
    Shape& shape = sym(id).shapeWithCutouts;
    if (shape.empty()) {
        constructShapeWithCutouts(shape, id);
    }

    return shape.scaled(mag);
}

void EngravingFont::constructShapeWithCutouts(Shape& shape, SymId id)
{
    RectF boundingBox = bbox(id, 1.0);
    double bottom = boundingBox.bottom();
    double top = boundingBox.top();
    double left = boundingBox.left();
    double right = boundingBox.right();

    PointF cutOutNW = smuflAnchor(id, SmuflAnchorId::cutOutNW, 1.0);
    PointF cutOutNE = smuflAnchor(id, SmuflAnchorId::cutOutNE, 1.0);
    PointF cutOutSW = smuflAnchor(id, SmuflAnchorId::cutOutSW, 1.0);
    PointF cutOutSE = smuflAnchor(id, SmuflAnchorId::cutOutSE, 1.0);

    bool nwNull = cutOutNW.isNull();
    bool neNull = cutOutNE.isNull();
    bool swNull = cutOutSW.isNull();
    bool seNull = cutOutSE.isNull();

    if (nwNull && neNull && swNull && seNull) {
        shape = Shape(bbox(id, 1.0));
        return;
    }

    if (nwNull) {
        cutOutNW = PointF(left, top);
    }
    if (neNull) {
        cutOutNE = PointF(right, top);
    }
    if (swNull) {
        cutOutSW = PointF(left, bottom);
    }
    if (seNull) {
        cutOutSE = PointF(right, bottom);
    }

    double leftInset = std::max(cutOutNW.x(), cutOutSW.x());
    double rightInset = std::min(cutOutNE.x(), cutOutSE.x());
    double topInset = std::max(cutOutNW.y(), cutOutNE.y());
    double bottomInset = std::min(cutOutSW.y(), cutOutSE.y());

    std::vector<RectF> rects;
    rects.reserve(6); //at most

    // bottom rect
    rects.emplace_back(RectF(PointF(cutOutSW.x(), bottom), PointF(cutOutSE.x(), topInset)).normalized());
    // right rect
    bool rightRectPlaced = false;
    if (!seNull) {
        rects.emplace_back(RectF(PointF(right, cutOutSE.y()), PointF(leftInset, cutOutNE.y())).normalized());
        rightRectPlaced = true;
    }
    // top rect
    bool topRectPlaced = false;
    if (!rightRectPlaced || !neNull) {
        rects.emplace_back(RectF(PointF(cutOutNW.x(), top), PointF(cutOutNE.x(), bottomInset)).normalized());
        topRectPlaced = true;
    }
    // left rect
    if (!topRectPlaced || !nwNull) {
        rects.emplace_back(RectF(PointF(left, cutOutSW.y()), PointF(rightInset, cutOutNW.y())).normalized());
    }
    // center horizontal rect if needed
    if (leftInset > rightInset && topInset < bottomInset) {
        rects.emplace_back(RectF(PointF(left, bottomInset), PointF(right, topInset)).normalized());
    }
    // center vertical rect if needed
    if (leftInset < rightInset && topInset > bottomInset) {
        rects.emplace_back(RectF(PointF(leftInset, bottom), PointF(rightInset, top)).normalized());
    }

    shape = Shape(rects);
}

// =============================================
// Symbol metrics
// =============================================

double EngravingFont::width(SymId id, double mag) const
{
    return bbox(id, mag).width();
}

double EngravingFont::height(SymId id, double mag) const
{
    return bbox(id, mag).height();
}

double EngravingFont::advance(SymId id, double mag) const
{
    if (useFallbackFont(id)) {
        return engravingFonts()->fallbackFont()->advance(id, mag);
    }

    return sym(id).advance * mag;
}

double EngravingFont::width(const SymIdList& s, double mag) const
{
    return bbox(s, mag).width();
}

PointF EngravingFont::smuflAnchor(SymId symId, SmuflAnchorId anchorId, double mag) const
{
    if (useFallbackFont(symId)) {
        return engravingFonts()->fallbackFont()->smuflAnchor(symId, anchorId, mag);
    }

    const std::map<SmuflAnchorId, PointF>& smuflAnchors = sym(symId).smuflAnchors;

    auto it = smuflAnchors.find(anchorId);
    if (it == smuflAnchors.cend()) {
        return PointF();
    }

    return it->second * mag;
}

// =============================================
// Draw
// =============================================

void EngravingFont::draw(SymId id, Painter* painter, const SizeF& mag, const PointF& pos, const double angle) const
{
    const Sym& sym = this->sym(id);
    if (sym.isCompound()) { // is this a compound symbol?
        draw(sym.subSymbolIds, painter, mag, pos, angle);
        return;
    }

    if (!sym.isValid()) {
        if (MScore::useFallbackFont && !engravingFonts()->isFallbackFont(this)) {
            engravingFonts()->fallbackFont()->draw(id, painter, mag, pos, angle);
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
    if (angle != 0) {
        const double _width = sym.bbox.width() / 2;
        const double _height = sym.bbox.height() / 2;
        painter->translate(_width, -_height);
        painter->rotate(angle);
        painter->translate(-_width, _height);
    }
    painter->drawSymbol(PointF(pos.x() / mag.width(), pos.y() / mag.height()), symCode(id));
    painter->restore();
}

void EngravingFont::draw(SymId id, Painter* painter, double mag, const PointF& pos, const double angle) const
{
    draw(id, painter, SizeF(mag, mag), pos, angle);
}

void EngravingFont::draw(const SymIdList& ids, Painter* painter, double mag, const PointF& startPos, const double angle) const
{
    PointF pos(startPos);
    for (SymId id : ids) {
        draw(id, painter, mag, pos, angle);
        pos.setX(pos.x() + advance(id, mag));
    }
}

void EngravingFont::draw(const SymIdList& ids, Painter* painter, const SizeF& mag, const PointF& startPos, const double angle) const
{
    PointF pos(startPos);
    for (SymId id : ids) {
        draw(id, painter, mag, pos, angle);
        pos.setX(pos.x() + advance(id, mag.width()));
    }
}
