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

#include "style.h"

#include "types/constants.h"
#include "compat/pageformat.h"
#include "rw/compat/readchordlisthook.h"
#include "rw/compat/compatutils.h"
#include "rw/xmlreader.h"
#include "rw/xmlwriter.h"
#include "types/typesconv.h"

#include "dom/mscore.h"
#include "dom/pedal.h"

#include "defaultstyle.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

const PropertyValue& MStyle::value(Sid idx) const
{
    if (idx == Sid::NOSTYLE) {
        static PropertyValue dummy;
        return dummy;
    }

    const PropertyValue& val = m_values[size_t(idx)];
    if (val.isValid()) {
        return val;
    }

    return StyleDef::styleValues[size_t(idx)].defaultValue();
}

Millimetre MStyle::valueMM(Sid idx) const
{
    if (idx == Sid::NOSTYLE) {
        return Millimetre();
    }

    return m_precomputedValues[size_t(idx)];
}

void MStyle::set(const Sid t, const PropertyValue& val)
{
    if (t == Sid::NOSTYLE) {
        return;
    }

    const size_t idx = size_t(t);
    m_values[idx] = val;
    if (t == Sid::spatium) {
        precomputeValues();
    } else {
        if (StyleDef::styleValues[idx].valueType() == P_TYPE::SPATIUM) {
            double _spatium = value(Sid::spatium).toReal();
            m_precomputedValues[idx] = m_values[idx].value<Spatium>().val() * _spatium;
        }
    }
}

double MStyle::defaultSpatium() const
{
    return DefaultStyle::resolveStyleDefaults(defaultStyleVersion()).spatium();
}

void MStyle::precomputeValues()
{
    double _spatium = value(Sid::spatium).toReal();
    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        if (t.valueType() == P_TYPE::SPATIUM) {
            m_precomputedValues[t.idx()] = value(t.styleIdx()).value<Spatium>().val() * _spatium;
        }
    }
}

bool MStyle::isDefault(Sid idx) const
{
    return value(idx) == DefaultStyle::resolveStyleDefaults(defaultStyleVersion()).value(idx);
}

void MStyle::setDefaultStyleVersion(const int defaultsVersion)
{
    set(Sid::defaultsVersion, defaultsVersion);
}

int MStyle::defaultStyleVersion() const
{
    return styleI(Sid::defaultsVersion);
}

bool MStyle::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());

    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        Sid idx = t.styleIdx();
        if (t.name() == tag) {
            P_TYPE type = t.valueType();
            switch (type) {
            case P_TYPE::SPATIUM:
                set(idx, Spatium(e.readDouble()));
                break;
            case P_TYPE::REAL:
                set(idx, e.readDouble());
                break;
            case P_TYPE::BOOL:
                set(idx, bool(e.readInt()));
                break;
            case P_TYPE::INT:
                set(idx, e.readInt());
                break;
            case P_TYPE::DIRECTION_V:
                set(idx, DirectionV(e.readInt()));
                break;
            case P_TYPE::STRING:
                set(idx, e.readText());
                break;
            case P_TYPE::ALIGN: {
                Align align = TConv::fromXml(e.readText(), Align());
                set(idx, align);
            } break;
            case P_TYPE::ALIGN_H: {
                AlignH align = TConv::fromXml(e.readAsciiText(), AlignH::HCENTER);
                set(idx, align);
            } break;
            case P_TYPE::POINT: {
                double x = e.doubleAttribute("x", 0.0);
                double y = e.doubleAttribute("y", 0.0);
                set(idx, PointF(x, y));
                e.readText();
            } break;
            case P_TYPE::SIZE: {
                double x = e.doubleAttribute("w", 0.0);
                double y = e.doubleAttribute("h", 0.0);
                set(idx, SizeF(x, y));
                e.readText();
            } break;
            case P_TYPE::SCALE: {
                double sx = e.doubleAttribute("w", 0.0);
                double sy = e.doubleAttribute("h", 0.0);
                set(idx, ScaleF(sx, sy));
                e.readText();
            } break;
            case P_TYPE::COLOR: {
                Color c;
                c.setRed(e.intAttribute("r"));
                c.setGreen(e.intAttribute("g"));
                c.setBlue(e.intAttribute("b"));
                c.setAlpha(e.intAttribute("a", 255));
                set(idx, c);
                e.readText();
            } break;
            case P_TYPE::PLACEMENT_V:
                set(idx, PlacementV(e.readText().toInt()));
                break;
            case P_TYPE::PLACEMENT_H:
                set(idx, PlacementH(e.readText().toInt()));
                break;
            case P_TYPE::HOOK_TYPE:
                set(idx, HookType(e.readText().toInt()));
                break;
            case P_TYPE::LINE_TYPE:
                set(idx, TConv::fromXml(e.readAsciiText(), LineType::SOLID));
                break;
            case P_TYPE::CLEF_TO_BARLINE_POS:
                set(idx, ClefToBarlinePosition(e.readInt()));
                break;
            case P_TYPE::TIE_PLACEMENT:
                set(idx, TConv::fromXml(e.readAsciiText(), TiePlacement::AUTO));
                break;
            case P_TYPE::TIE_DOTS_PLACEMENT:
                set(idx, TConv::fromXml(e.readAsciiText(), TieDotsPlacement::AUTO));
                break;
            case P_TYPE::GLISS_STYLE:
                set(idx, GlissandoStyle(e.readText().toInt()));
                break;
            case P_TYPE::GLISS_TYPE:
                set(idx, GlissandoType(e.readText().toInt()));
                break;
            case P_TYPE::TIMESIG_PLACEMENT:
                set(idx, TConv::fromXml(e.readAsciiText(), TimeSigPlacement::NORMAL));
                break;
            case P_TYPE::TIMESIG_STYLE:
                set(idx, TConv::fromXml(e.readAsciiText(), TimeSigStyle::NORMAL));
                break;
            case P_TYPE::TIMESIG_MARGIN:
                set(idx, TConv::fromXml(e.readAsciiText(), TimeSigVSMargin::RIGHT_ALIGN_TO_BARLINE));
                break;
            case P_TYPE::NOTE_SPELLING_TYPE:
                set(idx, TConv::fromXml(e.readAsciiText(), NoteSpellingType::STANDARD));
                break;
            case P_TYPE::CHORD_PRESET_TYPE:
                set(idx, TConv::fromXml(e.readAsciiText(), ChordStylePreset::STANDARD));
                break;
            case P_TYPE::LH_TAPPING_SYMBOL:
                set(idx, TConv::fromXml(e.readAsciiText(), LHTappingSymbol::DOT));
                break;
            case P_TYPE::RH_TAPPING_SYMBOL:
                set(idx, TConv::fromXml(e.readAsciiText(), RHTappingSymbol::T));
                break;
            default:
                ASSERT_X(u"unhandled type " + String::number(int(type)));
            }
            return true;
        }
    }
    if (readStyleValCompat(e)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   readStyleValCompat
//    Read obsolete style values which may appear in files
//    produced by older versions of MuseScore.
//---------------------------------------------------------

bool MStyle::readStyleValCompat(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    if (tag == "tempoOffset") {   // pre-3.0-beta
        const double x = e.doubleAttribute("x", 0.0);
        const double y = e.doubleAttribute("y", 0.0);
        const PointF val(x, y);
        set(Sid::tempoPosAbove, val);
        set(Sid::tempoPosBelow, val);
        e.readText();
        return true;
    }
    if (readTextStyleValCompat(e)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   readTextStyleValCompat
//    Handle transition from separate bold, underline, strike
//    and italic style properties to the single *FontStyle
//    property set.
//---------------------------------------------------------

bool MStyle::readTextStyleValCompat(XmlReader& e)
{
    static const std::array<std::pair<String, FontStyle>, 4> styleNamesEndings { {
        { u"FontBold",      FontStyle::Bold },
        { u"FontItalic",    FontStyle::Italic },
        { u"FontUnderline", FontStyle::Underline },
        { u"FontStrike",    FontStyle::Strike }
    } };

    const String tag = String::fromAscii(e.name().ascii());
    FontStyle readFontStyle = FontStyle::Normal;
    String typeName;
    for (auto& fontStyle : styleNamesEndings) {
        if (tag.endsWith(fontStyle.first)) {
            readFontStyle = fontStyle.second;
            typeName = tag.mid(0, tag.size() - fontStyle.first.size());
            break;
        }
    }
    if (readFontStyle == FontStyle::Normal) {
        return false;
    }

    const String newFontStyleName = typeName + u"FontStyle";
    const Sid sid = MStyle::styleIdx(newFontStyleName);
    if (sid == Sid::NOSTYLE) {
        LOGW() << "readFontStyleValCompat: couldn't read text readFontStyle value:" << tag;
        return false;
    }

    const bool readVal = bool(e.readText().toInt());
    const PropertyValue& val = value(sid);
    FontStyle newFontStyle = val.isValid() ? FontStyle(val.toInt()) : FontStyle::Normal;
    if (readVal) {
        newFontStyle = newFontStyle + readFontStyle;
    } else {
        newFontStyle = newFontStyle - readFontStyle;
    }

    set(sid, int(newFontStyle));
    return true;
}

void MStyle::readVersion(String versionTag)
{
    versionTag.remove(u".");
    m_version = versionTag.toInt();
}

bool MStyle::read(IODevice* device, bool ign)
{
    UNUSED(ign);
    XmlReader e(device);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            readVersion(e.attribute("version"));
            while (e.readNextStartElement()) {
                if (e.name() == "Style") {
                    read(e, nullptr);
                } else {
                    e.unknown();
                }
            }
        }
    }
    return true;
}

bool MStyle::isValid(IODevice* device)
{
    XmlReader e(device);
    while (!e.isError() && e.readNextStartElement()) {
        if (e.name() == "museScore") {
            while (e.readNextStartElement()) {
                if (e.name() == "Style") {
                    return true;
                }
            }
        }
    }
    return false;
}

void MStyle::read(XmlReader& e, compat::ReadChordListHook* readChordListHook)
{
    TRACEFUNC;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "TextStyle") {
            //readTextStyle206(this, e);        // obsolete
            e.readText();
        } else if (tag == "ottavaHook") {       // obsolete, for 3.0dev bw. compatibility, should be removed in final release
            double y = std::abs(e.readDouble());
            set(Sid::ottavaHookAbove, y);
            set(Sid::ottavaHookBelow, -y);
        } else if (tag == "Spatium" || tag == "spatium") {
            set(Sid::spatium, e.readDouble() * DPMM);
        } else if (tag == "page-layout") {      // obsolete
            compat::readPageFormat206(this, e);
        } else if (tag == "displayInConcertPitch") {
            set(Sid::concertPitch, e.readBool());
        } else if (tag == "ChordList") {
            if (readChordListHook) {
                readChordListHook->read(e);
            }
        } else if (tag == "lyricsDashMaxLegth") { // pre-3.6 typo
            set(Sid::lyricsDashMaxLength, Spatium(e.readDouble()));
        } else if (tag == "dontHidStavesInFirstSystm") { // pre-3.6.3/4.0 typo
            set(Sid::dontHideStavesInFirstSystem, e.readBool());
        } else if (tag == "minSpreadSpread") { // pre-4.4 typo
            set(Sid::minStaffSpread, Spatium(e.readDouble()));
        } else if (tag == "maxSpreadSpread") { // pre-4.4 typo
            set(Sid::maxStaffSpread, Spatium(e.readDouble()));
        } else if (tag == "beamDistance") { // beamDistance maps to useWideBeams in 4.0
            set(Sid::useWideBeams, e.readDouble() > 0.75);
        } else if (tag == "hairpinWidth") { // pre-4.4 typo
            set(Sid::hairpinLineWidth, Spatium(e.readDouble()));
        } else if (tag == "chordSymbolPosAbove") { // pre-4.4 typo
            set(Sid::chordSymbolAPosAbove, e.readPoint());
        } else if (tag == "chordSymbolPosBelow") { // pre-4.4 typo
            set(Sid::chordSymbolAPosBelow, e.readPoint());
        } else if (tag == "measureNumberAllStaffs") { // pre-4.4 typo
            set(Sid::measureNumberAllStaves, e.readBool());
        } else if (tag == "dontHidStavesInFirstSystm") { // pre-3.6.3/4.0 typo
            set(Sid::dontHideStavesInFirstSystem, e.readBool());
        } else if (tag == "firstSystemInsNameVisibility") { // pre-4.4 typo
            set(Sid::firstSystemInstNameVisibility, e.readInt());
        } else if ((tag == "articulationMinDistance"
                    || tag == "propertyDistanceHead"
                    || tag == "propertyDistanceStem"
                    || tag == "propertyDistance"
                    || tag == "bracketDistance")
                   && m_version < 400) {
            // Ignoring pre-4.0 articulation style and brackets distance settings. Using the new defaults instead
            e.skipCurrentElement();
        } else if (tag == "pedalListStyle") { // pre-3.6.3/4.0 typo
            set(Sid::pedalLineStyle, TConv::fromXml(e.readAsciiText(), LineType::SOLID));
        } else if (tag == "chordlineThickness" && m_version < 410) {
            // Ignoring pre-4.1 value as it was wrong (it wasn't user-editable anyway)
            e.skipCurrentElement();
        } else if (tag == "pedalText" && m_version < 420) {
            // Ignore old default
            String pedText = e.readText();
            if (!pedText.empty()) {
                set(Sid::pedalText, pedText);
            }
        } else if (tag == "pedalContinueText" && m_version < 420 && e.readAsciiText() == "") {
            // Ignore old default
            String pedContText = e.readText();
            if (!pedContText.empty()) {
                set(Sid::pedalText, pedContText);
            }
        } else if (tag == "ArpeggioNoteDistance") { // pre-4.4 typo
            set(Sid::arpeggioNoteDistance, Spatium(e.readDouble()));
        } else if (tag == "ArpeggioAccidentalDistance") { // pre-4.4 typo
            set(Sid::arpeggioAccidentalDistance, Spatium(e.readDouble()));
        } else if (tag == "ArpeggioAccidentalDistanceMin") { // pre-4.4 typo
            set(Sid::arpeggioAccidentalDistanceMin, Spatium(e.readDouble()));
        } else if (tag == "ArpeggioLineWidth") { // pre-x.4 typo
            set(Sid::arpeggioLineWidth, Spatium(e.readDouble()));
        } else if (tag == "ArpeggioHookLen") { // pre-x.4 typo
            set(Sid::arpeggioHookLen, Spatium(e.readDouble()));
        } else if (tag == "ArpeggioHiddenInStdIfTab") { // pre-x.4 typo
            set(Sid::arpeggioHiddenInStdIfTab, e.readBool());
        } else if ((tag == "slurEndWidth"
                    || tag == "slurMidWidth"
                    || tag == "slurDottedWidth"
                    || tag == "slurMinDistance")
                   && m_version < 430) {
            // Pre-4.3 scores used identical style values for slurs and ties.
            // When opening older scores, use the same values for both.
            double _val = e.readDouble();
            if (tag == "slurEndWidth") {
                set(Sid::tieEndWidth,     Spatium(_val));
                set(Sid::slurEndWidth,    Spatium(_val));
            } else if (tag == "slurMidWidth") {
                set(Sid::tieMidWidth,     Spatium(_val));
                set(Sid::slurMidWidth,    Spatium(_val));
            } else if (tag == "slurDottedWidth") {
                set(Sid::tieDottedWidth,  Spatium(_val));
                set(Sid::slurDottedWidth, Spatium(_val));
            } else if (tag == "slurMinDistance") {
                set(Sid::tieMinDistance,  Spatium(_val));
                set(Sid::slurMinDistance, Spatium(_val));
            }
        } else if (tag == "measureNumberOffset" && m_version < 440) { // pre-4.4 typo
            set(Sid::measureNumberPosAbove, PointF(e.readPoint()));
        } else if (tag == "measureNumberPosAbove" && m_version < 440) { // pre-4.4 typo
            set(Sid::mmRestRangePosAbove, PointF(e.readPoint()));
        } else if (tag == "ottavaTextAlign") {
            // Pre-x.x (?) scores used identical style values for Above and Below
            // apparently the old default was "VCENTER",
            // so better ignore and take the new defaults
            e.skipCurrentElement(); // obsolete
        } else if (tag == "tremoloStrokeStyle") { // pre-4.4 typo
            set(Sid::tremoloStyle, e.readInt());
        } else if (tag == "systemFontFace") { // pre-4.4 typo
            set(Sid::systemTextFontFace, e.readText());
        } else if (tag == "systemFontSize") { // pre-4.4 typo
            set(Sid::systemTextFontSize, e.readDouble());
        } else if (tag == "systemFontSpatiumDependent") { // pre-4.4 typo
            set(Sid::systemTextFontSpatiumDependent, bool(e.readInt()));
        } else if (tag == "systemFontStyle") { // pre-4.4 typo
            set(Sid::systemTextFontStyle, e.readInt());
        } else if (tag == "systemAlign") { // pre-4.4 typo
            set(Sid::systemTextAlign, TConv::fromXml(e.readText(), Align()));
        } else if (tag == "systemOffsetType") { // pre-4.4 typo
            set(Sid::systemTextOffsetType, e.readInt());
        } else if (tag == "systemPlacement") { // pre-4.4 typo
            set(Sid::systemTextPlacement, PlacementV(e.readText().toInt()));
        } else if (tag == "systemPosAbove") { // pre-4.4 typo
            set(Sid::systemTextPosAbove, PointF(e.readPoint()));
        } else if (tag == "systemPosBelow") { // pre-4.4 typo
            set(Sid::systemTextPosBelow, PointF(e.readPoint()));
        } else if (tag == "systemMinDistance") { // pre-4.4 typo
            set(Sid::systemTextMinDistance, Spatium(e.readDouble()));
        } else if (tag == "systemFrameType") { // pre-4.4 typo
            set(Sid::systemTextFrameType, e.readInt());
        } else if (tag == "systemFramePadding") { // pre-4.4 typo
            set(Sid::systemTextFramePadding, e.readDouble());
        } else if (tag == "systemFrameWidth") { // pre-4.4 typo
            set(Sid::systemTextFrameWidth, e.readDouble());
        } else if (tag == "systemFrameRound") { // pre-4.4 typo
            set(Sid::systemTextFrameRound, e.readInt());
        } else if (tag == "systemFrameFgColor") { // pre-4.4 typo
            set(Sid::systemTextFrameFgColor, e.readColor());
        } else if (tag == "systemFrameBgColor") { // pre-4.4 typo
            set(Sid::systemTextFrameBgColor, e.readColor());
        } else if (tag == "staffFontFace") { // pre-4.4 typo
            set(Sid::staffTextFontFace, e.readText());
        } else if (tag == "staffFontSize") { // pre-4.4 typo
            set(Sid::staffTextFontSize, e.readDouble());
        } else if (tag == "staffFontSpatiumDependent") { // pre-4.4 typo
            set(Sid::staffTextFontSpatiumDependent, e.readBool());
        } else if (tag == "staffFontStyle") { // pre-4.4 typo
            set(Sid::staffTextFontStyle, e.readInt());
        } else if (tag == "staffAlign") { // pre-4.4 typo
            set(Sid::staffTextAlign, TConv::fromXml(e.readText(), Align()));
        } else if (tag == "staffOffsetType") { // pre-4.4 typo
            set(Sid::staffTextOffsetType, e.readInt());
        } else if (tag == "staffPlacement") { // pre-4.4 typo
            set(Sid::staffTextPlacement, PlacementV(e.readText().toInt()));
        } else if (tag == "staffTextPosAbove"  // pre-4.4 typo, certainly before 3.6, even before 3.5
                   && m_version <= 410) { // so we might test for < 302, however m_version seems set to 410 here?!?
            double staffTextPosAboveY = e.readDouble();
            set(Sid::staffTextPosAbove, PointF(0.0, staffTextPosAboveY));
        } else if (tag == "staffPosAbove") { // pre-4.4 typo
            set(Sid::staffTextPosAbove, e.readPoint());
        } else if (tag == "staffPosBelow") { // pre-4.4 typo
            set(Sid::staffTextPosBelow, e.readPoint());
        } else if (tag == "staffTextMinDistance" // pre-4.4 typo, certainly before 3.6, even before 3.5
                   && m_version <= 410) { // so we might test for < 302, however m_version seems set to 410 here?!?
            set(Sid::staffTextMinDistance, Spatium(e.readDouble()));
        } else if (tag == "staffMinDistance") { // pre-4.4 typo
            set(Sid::staffTextMinDistance, Spatium(e.readDouble()));
        } else if (tag == "staffFrameType") { // pre-4.4 typo
            set(Sid::staffTextFrameType, e.readInt());
        } else if (tag == "staffFramePadding") { // pre-4.4 typo
            set(Sid::staffTextFramePadding, e.readDouble());
        } else if (tag == "staffFrameWidth") { // pre-4.4 typo
            set(Sid::staffTextFrameWidth, e.readDouble());
        } else if (tag == "staffFrameRound") { // pre-4.4 typo
            set(Sid::staffTextFrameRound, e.readInt());
        } else if (tag == "staffFrameFgColor") { // pre-4.4 typo
            set(Sid::staffTextFrameFgColor, e.readColor());
        } else if (tag == "staffFrameBgColor") { // pre-4.4 typo
            set(Sid::staffTextFrameBgColor, e.readColor());
        } else if (tag == "dymanicsShowTabCommon") { // pre-4.4 typo in gp-style.mss
            set(Sid::dynamicsShowTabCommon, bool(e.readInt()));
        } else if (tag == "tupletOufOfStaff") {
            set(Sid::tupletOutOfStaff, bool(e.readInt()));
        } else if (tag == "pedalBeginTextOffset"
                   || tag == "letRingBeginTextOffset"
                   || tag == "palmMuteBeginTextOffset"
                   || tag == "defaultFontSpatiumDependent"
                   || tag == "usePre_3_6_defaults") {
            e.skipCurrentElement(); // obsolete
        } else if (tag == "articulationAnchorDefault" && m_version < 410) {
            set(Sid::articulationAnchorDefault, (int)compat::CompatUtils::translateToNewArticulationAnchor(e.readInt()));
        } else if (tag == "articulationAnchorLuteFingering" && m_version < 410) {
            set(Sid::articulationAnchorLuteFingering, (int)compat::CompatUtils::translateToNewArticulationAnchor(e.readInt()));
        } else if (tag == "articulationAnchorOther" && m_version < 410) {
            set(Sid::articulationAnchorOther, (int)compat::CompatUtils::translateToNewArticulationAnchor(e.readInt()));
        } else if (tag == "lineEndToSystemEndDistance") { // renamed in 4.5
            set(Sid::lineEndToBarlineDistance, Spatium(e.readDouble()));
        } else if (tag == "useStandardNoteNames") {     // These settings were collapsed into one enum in 4.6
            if (e.readBool()) {
                set(Sid::chordSymbolSpelling, NoteSpellingType::STANDARD);
            }
        } else if (tag == "useGermanNoteNames") {
            if (e.readBool()) {
                set(Sid::chordSymbolSpelling, NoteSpellingType::GERMAN);
            }
        } else if (tag == "useFullGermanNoteNames") {
            if (e.readBool()) {
                set(Sid::chordSymbolSpelling, NoteSpellingType::GERMAN_PURE);
            }
        } else if (tag == "useSolfeggioNoteNames") {
            if (e.readBool()) {
                set(Sid::chordSymbolSpelling, NoteSpellingType::SOLFEGGIO);
            }
        } else if (tag == "useFrenchNoteNames") {
            if (e.readBool()) {
                set(Sid::chordSymbolSpelling, NoteSpellingType::FRENCH);
            }
        } else if (tag == "chordModifierAdjust" && m_version < 460) {
            set(Sid::chordModifierAdjust, compat::CompatUtils::convertChordExtModUnits(e.readDouble()));
        } else if (tag == "chordExtensionAdjust" && m_version < 460) {
            set(Sid::chordExtensionAdjust, compat::CompatUtils::convertChordExtModUnits(e.readDouble()));
        } else if (tag == "chordDescriptionFile" && m_version < 460) {
            AsciiStringView val = e.readAsciiText();
            if (val == "chords_std.xml") {
                set(Sid::chordDescriptionFile, String(u"chords_legacy.xml"));
            } else {
                set(Sid::chordDescriptionFile, String::fromAscii(val.ascii()));
            }
        } else if (tag == "chordStyle" && m_version < 460) {
            AsciiStringView val = e.readAsciiText();
            if (val == "std") {
                set(Sid::chordStyle, ChordStylePreset::LEGACY);
            } else {
                set(Sid::chordStyle, TConv::fromXml(val, ChordStylePreset::STANDARD));
            }
        } else if (tag == "fretFrets" && m_version < 460) {
            e.skipCurrentElement();
        } else if (!readProperties(e)) {
            e.unknown();
        }
    }

    if (m_version < 460) {
        bool verticalChordAlign = value(Sid::maxChordShiftAbove).value<Spatium>() != Spatium(0.0)
                                  || value(Sid::maxChordShiftBelow).value<Spatium>() != Spatium(0.0)
                                  || value(Sid::maxFretShiftAbove).value<Spatium>() != Spatium(0.0)
                                  || value(Sid::maxFretShiftBelow).value<Spatium>() != Spatium(0.0);
        set(Sid::verticallyAlignChordSymbols, verticalChordAlign);
        // Make sure new position styles are initially the same as align values
        for (const StyleDef::StyleValue& st : StyleDef::styleValues) {
            Sid positionSid = compat::CompatUtils::positionStyleFromAlign(st.styleIdx());
            if (positionSid == Sid::NOSTYLE) {
                continue;
            }
            AlignH val = value(st.styleIdx()).value<Align>().horizontal;
            set(positionSid, val);
        }
    }

    if (m_version == 450) {
        // 450 spacing was a bit narrower
        set(Sid::spacingDensity, 1.30);
    }

    if (m_version < 450) {
        // Didn't exist before 4.5. Default to false for compatibility.
        set(Sid::scaleRythmicSpacingForSmallNotes, false);
        set(Sid::maskBarlinesForText, false);
        set(Sid::showCourtesiesRepeats, false);
        set(Sid::showCourtesiesOtherJumps, false);
        set(Sid::showCourtesiesAfterCancellingRepeats, false);
        set(Sid::showCourtesiesAfterCancellingOtherJumps, false);
        set(Sid::changesBeforeBarlineRepeats, false);
        set(Sid::changesBeforeBarlineOtherJumps, false);
    }

    if (m_version < 420 && !MScore::testMode) {
        // This style didn't exist before version 4.2. For files older than 4.2, defaults
        // to INSIDE for compatibility. For files 4.2 and newer, defaults to OUTSIDE.
        set(Sid::tiePlacementChord, TiePlacement::INSIDE);
    }

    if (readChordListHook) {
        readChordListHook->validate();
    }
}

bool MStyle::write(IODevice* device)
{
    XmlWriter xml(device);
    xml.startDocument();
    xml.startElement("museScore", { { "version", Constants::MSC_VERSION_STR } });
    save(xml, false);
    xml.endElement();
    return true;
}

void MStyle::save(XmlWriter& xml, bool optimize)
{
    xml.startElement("Style");

    for (const StyleDef::StyleValue& st : StyleDef::styleValues) {
        Sid idx = st.styleIdx();
        if (idx == Sid::spatium) {         // special handling for spatium
            continue;
        }
        if (optimize && isDefault(idx)) {
            continue;
        }
        P_TYPE type = st.valueType();
        if (P_TYPE::SPATIUM == type) {
            xml.tag(st.name(), value(idx).value<Spatium>().val());
        } else if (P_TYPE::DIRECTION_V == type) {
            xml.tag(st.name(), int(value(idx).value<DirectionV>()));
        } else if (P_TYPE::ALIGN == type) {
            Align a = value(idx).value<Align>();
            // Don't write if it's the default value
            if (optimize && a == st.defaultValue().value<Align>()) {
                continue;
            }
            xml.tag(st.name(), TConv::toXml(a));
        } else if (P_TYPE::ALIGN_H == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<AlignH>()));
        } else if (P_TYPE::LINE_TYPE == type) {
            xml.tagProperty(st.name(), value(idx));
        } else if (P_TYPE::TIE_PLACEMENT == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<TiePlacement>()));
        } else if (P_TYPE::TIE_DOTS_PLACEMENT == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<TieDotsPlacement>()));
        } else if (P_TYPE::TIMESIG_PLACEMENT == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<TimeSigPlacement>()));
        } else if (P_TYPE::TIMESIG_STYLE == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<TimeSigStyle>()));
        } else if (P_TYPE::TIMESIG_MARGIN == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<TimeSigVSMargin>()));
        } else if (P_TYPE::CHORD_PRESET_TYPE == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<ChordStylePreset>()));
        } else if (P_TYPE::NOTE_SPELLING_TYPE == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<NoteSpellingType>()));
        } else if (P_TYPE::LH_TAPPING_SYMBOL == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<LHTappingSymbol>()));
        } else if (P_TYPE::RH_TAPPING_SYMBOL == type) {
            xml.tag(st.name(), TConv::toXml(value(idx).value<RHTappingSymbol>()));
        } else {
            PropertyValue val = value(idx);
            //! NOTE for compatibility
            if (val.isEnum()) {
                val = val.value<int>();
            }
            xml.tagProperty(st.name(), val);
        }
    }

    xml.tag("spatium", value(Sid::spatium).toReal() / DPMM);
    xml.endElement();
}

// ====================================================
// Static
// ====================================================

P_TYPE MStyle::valueType(const Sid i)
{
    return StyleDef::styleValues[size_t(i)].valueType();
}

const char* MStyle::valueName(const Sid i)
{
    if (i == Sid::NOSTYLE) {
        static const char* no_style = "no style";
        return no_style;
    }
    return StyleDef::styleValues[size_t(i)].name().ascii();
}

Sid MStyle::styleIdx(const String& name)
{
    muse::ByteArray ba = name.toAscii();
    for (const StyleDef::StyleValue& st : StyleDef::styleValues) {
        if (st.name() == ba.constChar()) {
            return st.styleIdx();
        }
    }
    return Sid::NOSTYLE;
}
