/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "stafftype.h"

#include "draw/fontmetrics.h"
#include "translation.h"

#include "iengravingconfiguration.h"

#include "style/defaultstyle.h"
#include "style/style.h"
#include "style/textstyle.h"
#include "types/typesconv.h"

#include "mscore.h"
#include "score.h"
#include "staff.h"

using namespace muse::draw;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::engraving {
// HISTORIC TAB BASS STRING NOTATION
// The following constants refer to the specifics of bass string notation in historic
//    (Renaiss./Baroque French and Italian) tablatures.

// how much to lower a bass string note with slashes with respect to line distance (in fraction of line distance)
constexpr double STAFFTYPE_TAB_BASSSLASH_YOFFSET = 0.33;

// The following constants could ideally be customizable values;
//    they are currently constants to simplify implementation;
// Note that these constants do not constrain which strings of an instrument are
//    physically frettable (which is defined in the instrument itself) but fix the
//    number of bass strings for which the notation is able to express a fret number
//    rather than simply a string ordinal.
constexpr int NUM_OF_BASSSTRINGS_WITH_LETTER = 4;     // the max number of bass strings frettable with letter notation (French)
constexpr int NUM_OF_BASSSTRINGS_WITH_NUMBER = 2;     // the max number of bass strings frettable with number notation (Italian)

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

constexpr double TAB_DEFAULT_DUR_YOFFS = -1.0;

muse::GlobalInject<IEngravingConfiguration> StaffType::configuration;

std::vector<TablatureFretFont> StaffType::m_fretFonts = {};
std::vector<TablatureDurationFont> StaffType::m_durationFonts = {};

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

StaffType::StaffType()
{
    m_color = configuration()->defaultColor();
    // set reasonable defaults for type-specific members */
    m_symRepeat = TablatureSymbolRepeat::NEVER;
    setDurationFontName(m_durationFonts[0].displayName);
    setFretPresetIdx(0);
}

StaffType::StaffType(StaffGroup sg, StaffTypes staffType, int lines, int stpOff, double lineDist,
                     bool genClef, bool showBarLines, bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLines, bool invisible,
                     const Color& color)
    : m_group(sg),
    m_staffType(staffType),
    m_invisible(invisible),
    m_color(color),
    m_lines(lines),
    m_stepOffset(stpOff),
    m_lineDistance(Spatium(lineDist)),
    m_showBarlines(showBarLines),
    m_showLedgerLines(showLedgerLines),
    m_stemless(stemless),
    m_genClef(genClef),
    m_genTimesig(genTimeSig),
    m_genKeysig(genKeySig)
{
}

StaffType::StaffType(StaffGroup sg, StaffTypes staffType, int lines, int stpOff, double lineDist,
                     bool genClef, bool showBarLines, bool stemless, bool genTimesig, bool invisible, const Color& color,
                     const String& durFontName, double genDur, bool fretFontUseTextStyle, const String& fretFontName,
                     TablatureSymbolRepeat symRepeat, bool linesThrough, TablatureMinimStyle minimStyle, bool onLines, bool showRests,
                     bool stemsDown, bool stemThrough, bool upsideDown, bool showTabFingering, bool useNumbers, bool showBackTied)
{
    UNUSED(invisible);
    m_color = color;
    m_group   = sg;
    m_staffType = staffType;
    setLines(lines);
    setStepOffset(stpOff);
    setLineDistance(Spatium(lineDist));
    setGenClef(genClef);
    setShowBarlines(showBarLines);
    setStemless(stemless);
    setGenTimesig(genTimesig);
    setGenKeysig(sg != StaffGroup::TAB);
    setDurationFontName(durFontName);
    setGenDurations(genDur);
    setFretUseTextStyle(fretFontUseTextStyle);
    if (fretFontUseTextStyle) {
        setFretTextStyle(TextStyleType::TAB_FRET_NUMBER);
    } else {
        setFretPreset(fretFontName);
    }
    setSymbolRepeat(symRepeat);
    setLinesThrough(linesThrough);
    setMinimStyle(minimStyle);
    setOnLines(onLines);
    setShowRests(showRests);
    setStemsDown(stemsDown);
    setStemsThrough(stemThrough);
    setUpsideDown(upsideDown);
    setShowTabFingering(showTabFingering);
    setUseNumbers(useNumbers);
    setShowBackTied(showBackTied);
}

//---------------------------------------------------------
//   translatedGroupName
//---------------------------------------------------------

String StaffType::translatedGroupName() const
{
    return TConv::translatedUserName(m_group);
}

int StaffType::middleLine() const
{
    return m_lines - 1 - m_stepOffset;
}

int StaffType::bottomLine() const
{
    return (m_lines - 1) * 2;
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool StaffType::operator==(const StaffType& st) const
{
    bool equal = true;

    equal &= (m_group == st.m_group);
    equal &= (m_staffType == st.m_staffType);
    equal &= (m_staffLabel == st.m_staffLabel);
    equal &= (m_userMag == st.m_userMag);
    equal &= (m_yoffset == st.m_yoffset);
    equal &= (m_small == st.m_small);
    equal &= (m_invisible == st.m_invisible);
    equal &= (m_color == st.m_color);
    equal &= (m_lines == st.m_lines);
    equal &= (m_stepOffset == st.m_stepOffset);
    equal &= (m_lineDistance == st.m_lineDistance);
    equal &= (m_showBarlines == st.m_showBarlines);
    equal &= (m_showLedgerLines == st.m_showLedgerLines);
    equal &= (m_stemless == st.m_stemless);
    equal &= (m_genClef == st.m_genClef);
    equal &= (m_genTimesig == st.m_genTimesig);
    equal &= (m_genKeysig == st.m_genKeysig);
    equal &= (m_noteHeadScheme == st.m_noteHeadScheme);
    equal &= (m_durationFontSize == st.m_durationFontSize);
    equal &= (m_durationFontUserY == st.m_durationFontUserY);
    equal &= (m_fretFontSize == st.m_fretFontSize);
    equal &= (m_fretFontUserY == st.m_fretFontUserY);
    equal &= (m_genDurations == st.m_genDurations);
    equal &= (m_linesThrough == st.m_linesThrough);
    equal &= (m_minimStyle == st.m_minimStyle);
    equal &= (m_symRepeat == st.m_symRepeat);
    equal &= (m_onLines == st.m_onLines);
    equal &= (m_showRests == st.m_showRests);
    equal &= (m_stemsDown == st.m_stemsDown);
    equal &= (m_stemsThrough == st.m_stemsThrough);
    equal &= (m_upsideDown == st.m_upsideDown);
    equal &= (m_showTabFingering == st.m_showTabFingering);
    equal &= (m_useNumbers == st.m_useNumbers);
    equal &= (m_showBackTied == st.m_showBackTied);
    equal &= (m_durationBoxH == st.m_durationBoxH);
    equal &= (m_durationBoxY == st.m_durationBoxY);
    equal &= (m_durationFont == st.m_durationFont);
    equal &= (m_durationFontIdx == st.m_durationFontIdx);
    equal &= (m_durationYOffset == st.m_durationYOffset);
    equal &= (m_durationGridYOffset == st.m_durationGridYOffset);
    equal &= (m_fretBoxH == st.m_fretBoxH);
    equal &= (m_deadFretBoxH == st.m_deadFretBoxH);
    equal &= (m_fretBoxY == st.m_fretBoxY);
    equal &= (m_deadFretBoxY == st.m_deadFretBoxY);
    equal &= (m_fretFont == st.m_fretFont);
    equal &= (m_fretFontInfo.family == st.m_fretFontInfo.family);
    equal &= (m_fretYOffset == st.m_fretYOffset);
    equal &= (m_fretUseTextStyle == st.m_fretUseTextStyle);
    equal &= (m_fretTextStyle == st.m_fretTextStyle);
    equal &= (m_fretPresetIdx == st.m_fretPresetIdx);

    return equal;
}

StaffTypes StaffType::type() const
{
    return m_staffType;
}

String StaffType::staffTypeName() const
{
    return TConv::translatedUserName(m_staffType);
}

String StaffType::xmlName() const
{
    return String::fromAscii(TConv::toXml(m_staffType).ascii());
}

//---------------------------------------------------------
//   isSimpleTabStaff
//---------------------------------------------------------

bool StaffType::isSimpleTabStaff() const
{
    if (!isTabStaff()) {
        return false;
    }

    StaffTypes stType = type();

    switch (stType) {
    case StaffTypes::TAB_4SIMPLE:
    case StaffTypes::TAB_5SIMPLE:
    case StaffTypes::TAB_6SIMPLE:
    case StaffTypes::TAB_7SIMPLE:
    case StaffTypes::TAB_8SIMPLE:
    case StaffTypes::TAB_9SIMPLE:
    case StaffTypes::TAB_10SIMPLE:
    case StaffTypes::TAB_ITALIAN:
    case StaffTypes::TAB_FRENCH:
        return true;

    default:
        break;
    }

    return false;
}

//---------------------------------------------------------
//   isCommonTabStaff
//---------------------------------------------------------

bool StaffType::isCommonTabStaff() const
{
    return !isTabStaff() ? false : !isSimpleTabStaff();
}

//---------------------------------------------------------
//   isHiddenElementOnTab
//---------------------------------------------------------

bool StaffType::isHiddenElementOnTab(Sid commonTabStyle, Sid simpleTabStyle) const
{
    return (isCommonTabStaff() && !style().styleB(commonTabStyle)) || (isSimpleTabStaff() && !style().styleB(simpleTabStyle));
}

void StaffType::styleChanged()
{
    if (!m_fretUseTextStyle) {
        return;
    }
    setFretTextStyle(m_fretTextStyle);
}

//---------------------------------------------------------
//   setOnLines
//---------------------------------------------------------

void StaffType::setOnLines(bool val)
{
    m_onLines = val;
    setDurationMetrics();
    setFretMetrics();
}

void StaffType::setUseNumbers(bool val)
{
    m_useNumbers = val;
    setFretMetrics();
}

void StaffType::setFretTextStyle(const TextStyleType& val)
{
    m_fretTextStyle = val;
    m_fretFontInfo = TablatureFretFont();
    m_fretFont = Font();

    const TextStyle* ts = textStyle(m_fretTextStyle);

    for (const TextStyleProperty property : *ts) {
        switch (property.type) {
        case TextStylePropertyType::FontFace: {
            String fontName = style().styleSt(property.sid);
            m_fretFontInfo.family = fontName;
            m_fretFont.setFamily(fontName, Font::Type::Tablature);
        } break;
        case TextStylePropertyType::FontSize: {
            double fontSize = style().styleD(property.sid);
            setFretFontSize(fontSize);
        } break;
        case TextStylePropertyType::FontStyle: {
            FontStyle fStyle = style().styleV(property.sid).value<FontStyle>();
            m_fretFont.setBold(fStyle & FontStyle::Bold);
            m_fretFont.setItalic(fStyle & FontStyle::Italic);
            m_fretFont.setUnderline(fStyle & FontStyle::Underline);
            m_fretFont.setStrike(fStyle & FontStyle::Strike);
        } break;
        default:
            continue;
        }
    }

    PointF offset = style().styleV(ts->offsetSids.above).value<PointF>();
    setFretFontUserY(offset.y());

    setFretMetrics();
}

void StaffType::setFretPresetIdx(size_t idx)
{
    // Clear all previous formatting
    m_fretFont = Font();
    if (idx >= m_fretFonts.size()) {
        m_fretPresetIdx = 0;
        m_fretFontInfo = m_fretFonts[0];
    } else {
        m_fretPresetIdx = idx;
        m_fretFontInfo = m_fretFonts[idx];
    }

    m_fretFont.setFamily(m_fretFontInfo.family, Font::Type::Tablature);
    setFretFontSize(m_fretFontInfo.defSize);
    setFretFontUserY(m_fretFontInfo.defYOffset);
    setFretMetrics();
}

void StaffType::setFretPreset(const String& name)
{
    String locName = name;
    // convert old names for two built-in fonts which have changed of name
    if (name == "MuseScore Tab Late Renaiss") {
        locName = u"MuseScore Phalèse";
    }
    size_t idx = 0;
    for (idx = 0; idx < m_fretFonts.size(); idx++) {
        if (m_fretFonts[idx].displayName == locName) {
            break;
        }
    }

    setFretPresetIdx(idx);
}

//---------------------------------------------------------
//   setDurationMetrics
//    checks whether the internally computed metrics are is still valid and re-computes them, if not
//---------------------------------------------------------

void StaffType::setDurationMetrics()
{
// FontMetrics returns results unreliably rounded to integral pixels;
// use a scaled up font and then scale computed values down
    Font font(durationFont());
    font.setPointSizeF(m_durationFontSize);
    FontMetrics fm(font);
    String txt(m_durationFonts[m_durationFontIdx].displayValue, size_t(TabVal::NUM_OF));
    RectF bb(fm.tightBoundingRect(txt));
    // raise symbols by a default margin and, if marks are above lines, by half the line distance
    // (converted from spatium units to raster units)
    m_durationGridYOffset = (TAB_DEFAULT_DUR_YOFFS - (m_onLines ? 0.0 : lineDistance().val() * 0.5)) * defaultSpatium();
    // this is the bottomest point of any duration sign
    m_durationYOffset = m_durationGridYOffset;
    // move symbols so that the lowest margin 'sits' on the base line:
    // move down by the whole part above (negative) the base line
    // ( -bb.y() ) then up by the whole height ( -bb.height() )
    m_durationYOffset        -= (bb.height() + bb.y()) / 100.0;
    m_durationBoxH           = bb.height() / 100.0;
    m_durationBoxY           = m_durationGridYOffset - bb.height() / 100.0;
}

void StaffType::setFretMetrics()
{
    FontMetrics fm(fretFont());
    RectF bb;
    // compute vertical displacement
    if (m_useNumbers) {
        // compute total height of used characters
        String txt;
        for (int idx = 0; idx < 10; idx++) {    // use only first 10 digits
            txt.append(m_fretFontInfo.displayDigit[idx]);
        }
        bb = fm.tightBoundingRect(txt);
        // for numbers: centre on '0': move down by the whole part above (negative)
        // the base line ( -bb.y() ) then up by half the whole height ( -bb.height()/2 )
        RectF bx(fm.tightBoundingRect(m_fretFontInfo.displayDigit[0]));
        m_fretYOffset = -(bx.y() + bx.height() / 2.0);
        // _fretYOffset = -(bb.y() + bb.height()/2.0);  // <- using bbox of all chars
    } else {
        // compute total height of used characters
        const String txt(m_fretFontInfo.displayLetter.data(), NUM_OF_LETTERFRETS);
        bb = fm.tightBoundingRect(txt);
        // for letters: centre on the 'a' ascender, by moving down half of the part above the base line in bx
        RectF bx(fm.tightBoundingRect(m_fretFontInfo.displayLetter[0]));
        m_fretYOffset = -bx.y() / 2.0;
    }

    // Calculate position for dead fret marks - these must be centred separately based on their glyph
    RectF deadBb = fm.tightBoundingRect(m_fretFontInfo.xChar);
    double lineThickness = style().styleS(Sid::staffLineWidth).val() * defaultSpatium() * 0.5;
    m_deadFretYOffset = -deadBb.y() / 2.0 + lineThickness;

    // if on string, we are done; if between strings, raise by half line distance
    if (!m_onLines) {
        double lineAdj = lineDistance().val() * defaultSpatium() * 0.5;
        m_fretYOffset -= lineAdj;
        m_deadFretYOffset -= lineAdj;
    }

    // from _fretYOffset, compute _fretBoxH and _fretBoxY
    m_fretBoxH = bb.height();
    m_fretBoxY = bb.y() + m_fretYOffset;

    m_deadFretBoxH = deadBb.height();
    m_deadFretBoxY = deadBb.y() + m_deadFretYOffset;
}

//---------------------------------------------------------
//   setDurationFontName
//---------------------------------------------------------

void StaffType::setDurationFontName(const String& name)
{
    size_t idx;
    for (idx = 0; idx < m_durationFonts.size(); idx++) {
        if (m_durationFonts[idx].displayName == name) {
            break;
        }
    }
    if (idx >= m_durationFonts.size()) {
        idx = 0;              // if name not found, use first font
    }
    m_durationFontIdx = idx;
    m_durationFont.setFamily(m_durationFonts[idx].family, Font::Type::Tablature);
    setDurationFontSize(m_durationFonts[idx].defSize);
    setDurationFontUserY(m_durationFonts[idx].defYOffset);
    setDurationMetrics();
}

//---------------------------------------------------------
//   durationBoxH / durationBoxY
//---------------------------------------------------------

double StaffType::defaultSpatium() const
{
    return StyleDef::styleValues[static_cast<size_t>(Sid::spatium)].defaultValue.toDouble();
}

double StaffType::durationBoxH() const
{
    if (!m_genDurations && !m_stemless) {
        return 0.0;
    }
    return m_durationBoxH;
}

double StaffType::durationBoxY() const
{
    if (!m_genDurations && !m_stemless) {
        return 0.0;
    }
    return m_durationBoxY + m_durationFontUserY * defaultSpatium();
}

double StaffType::durationFontYOffset() const
{
    return m_durationYOffset + m_durationFontUserY * defaultSpatium();
}

double StaffType::fretBoxY() const
{
    return m_fretBoxY + m_fretFontUserY * defaultSpatium();
}

double StaffType::deadFretBoxY() const
{
    return m_deadFretBoxY + m_fretFontUserY * defaultSpatium();
}

double StaffType::fretMaskH() const
{
    return m_lineDistance.val() * defaultSpatium();
}

double StaffType::fretMaskY() const
{
    return (m_onLines ? -0.5 : -1.0) * m_lineDistance.val() * defaultSpatium();
}

double StaffType::fretFontYOffset() const
{
    return m_fretYOffset + m_fretFontUserY * defaultSpatium();
}

//---------------------------------------------------------
//   setDurationFontSize / setFretFontSize
//---------------------------------------------------------

void StaffType::setDurationFontSize(double val)
{
    m_durationFontSize = val;
    m_durationFont.setPointSizeF(val);
    setDurationMetrics();
}

void StaffType::setFretFontSize(double val)
{
    m_fretFontSize = val;
    m_fretFont.setPointSizeF(val);
    setFretMetrics();
}

//---------------------------------------------------------
//   fretString / durationString
//
//    construct the text string for a given fret / duration
//---------------------------------------------------------

static const String UNKNOWN_FRET = String(u"?");

String StaffType::fretString(int fret, int string, bool deadNote) const
{
    if (fret == INVALID_FRET_INDEX) {
        return UNKNOWN_FRET;
    }
    if (deadNote) {
        return String(m_fretFontInfo.xChar);
    } else {
        bool hasFret;
        String text  = tabBassStringPrefix(string, &hasFret);
        if (!hasFret) {             // if the notation does not allow to fret this string,
            return text;            // return the prefix only
        }
        // otherwise, add to prefix the relevant digit/letter string
        return text
               + (m_useNumbers
                  ? (fret >= NUM_OF_DIGITFRETS ? UNKNOWN_FRET : m_fretFontInfo.displayDigit[fret])
                  : (fret >= NUM_OF_LETTERFRETS ? UNKNOWN_FRET : m_fretFontInfo.displayLetter[fret]));
    }
}

String StaffType::durationString(DurationType type, int dots) const
{
    String s = m_durationFonts[m_durationFontIdx].displayValue[size_t(type)];
    for (int count = 0; count < dots; count++) {
        s.append(m_durationFonts[m_durationFontIdx].displayDot);
    }
    return s;
}

//---------------------------------------------------------
//    tabBassStringPrefix
//
//    returns a String (possibly empty) with the prefix identifying a bass string in TAB's;
//    can deal with non-bass strings (i.e. regular TAB lines).
//
//    Implements the specifics of historic notations for bass lines (i.e. strings outside
//    the lines of the tab), both Italian and French.
//
//    strg   the instrument physical string ordinal (0 = topmost string, may exceed the number
//                of lines actually present in the TAB to reference a bass string)
//    bool   pntr to a bool receiving the info if notation allows to express a fret number or not
//                (this is potentially different from the fact that the instrument string itself can be fretted or not)
//---------------------------------------------------------

String StaffType::tabBassStringPrefix(int strg, bool* hasFret) const
{
    *hasFret    = true;             // assume notation allows to fret this string
    int bassStrgIdx  = (strg >= m_lines ? strg - m_lines + 1 : 0);
    if (m_useNumbers) {
        // if above the max bass string which can be fretted with number notation
        // return a number with the string index
        if (bassStrgIdx > NUM_OF_BASSSTRINGS_WITH_NUMBER) {
            *hasFret    = false;
            return m_fretFontInfo.displayDigit[strg + 1];
        }
        // if a frettable bass string, return an empty string
        return String();
    } else {
        // bass string notation
        // if above the max bass string which can be fretted with letter notation
        // return a number with the bass string index itself
        if (bassStrgIdx > NUM_OF_BASSSTRINGS_WITH_LETTER) {
            *hasFret    = false;
            return m_fretFontInfo.displayDigit[bassStrgIdx - 1];
        }
        // if a frettable bass string, return a character with the relevant num. of slashes;
        // note that the number of slashes is bassStrgIdx-1 (1st bass has no slash)
        // and slashChar[] is 0-based (slashChar[0] => 1 slash, ...), whence the -2
        String prefix    = bassStrgIdx > 1
                           ? String(m_fretFontInfo.slashChar[bassStrgIdx - 2]) : String();
        return prefix;
    }
}

//---------------------------------------------------------
//   numOfLedgerLines
//
//    in TAB's, returns the number of ledgerlines needed by bass lines in some TAB styles.
//
//    Returns 0 if staff is not a TAB, if a TAB but style does not use ledger lines
//    or ledger lines do not apply to the given string.
//---------------------------------------------------------

int StaffType::numOfTabLedgerLines(int string) const
{
    if (m_group != StaffGroup::TAB || !m_useNumbers) {
        return 0;
    }

    int numOfLedgers= string < 0 ? -string : string - m_lines + 1;
    return numOfLedgers >= 1 && numOfLedgers <= NUM_OF_BASSSTRINGS_WITH_NUMBER ? numOfLedgers : 0;
}

//---------------------------------------------------------
//   physStringToVisual / visualStringToPhys
//
//    returns the string ordinal in visual order (top to down) from a string ordinal in physical order
//    or vice-versa: manages upsideDown
//---------------------------------------------------------

int StaffType::physStringToVisual(int strg) const
{
    if (strg < 0) {                       // if above top string, return top string
        strg = 0;
    }
//      // NO: bass strings may exist, which are in addition to tab string lines
//      if (strg >= _lines)                 // if physical string has no visual representation,
//            strg = _lines - 1;            // reduce to nearest visual line
    // if TAB upside down, flip around top line
    return m_upsideDown ? m_lines - 1 - strg : strg;
}

int StaffType::visualStringToPhys(int line) const
{
    // if TAB upside down, reverse string number
    line = (m_upsideDown ? m_lines - 1 - line : line);

    if (line < 0) {           // if above top string, reduce to top string
        line = 0;
    }
// NO: bass strings may exist, which are in addition to tab string lines
//      if (line >= _lines)
//            line = _lines - 1;
    return line;
}

//---------------------------------------------------------
//   physStringToYOffset
//
//    returns the string Y offset from a string ordinal in physical order:
//    manages upsideDown and extra bass strings.
//
//    The returned values is in sp. and is relative to the staff top line.
//
//    Note: the difference with physStringToVisual() is that this function takes into account
//          peculiarities of bass string notations.
//---------------------------------------------------------

Spatium StaffType::physStringToYOffset(int strg) const
{
    double yOffset = strg;                       // the y offset of the visual string, as a multiple of line distance
    if (yOffset < 0) {                          // if above top physical string, limit to top string
        yOffset = 0;
    }
    if (yOffset >= m_lines) {                    // if physical string 'below' tab lines,
        yOffset = m_lines;                       // reduce to first string 'below' tab body
        if (!m_useNumbers) {                     // with letters, add some space for the slashes ascender
            yOffset = m_onLines ? m_lines : m_lines + STAFFTYPE_TAB_BASSSLASH_YOFFSET;
        }
    }
    // if TAB upside down, flip around top line
    yOffset = m_upsideDown ? (double)(m_lines - 1) - yOffset : yOffset;
    return yOffset * m_lineDistance;
}

//---------------------------------------------------------
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

TablatureFretFont::TablatureFretFont()
{
    // Set up defaults
    for (size_t i = 0; i < NUM_OF_DIGITFRETS; i++) {
        displayDigit.at(i) = String::number(i);
    }

    for (size_t i = 0; i < NUM_OF_LETTERFRETS; i++) {
        displayLetter[i] = Char(u'a' + static_cast<char16_t>(i));
    }

    for (size_t i = 0; i < NUM_OF_BASSSTRING_SLASHES; i++) {
        for (size_t j = 0; j < i; j++) {
            slashChar.at(i).append(u"/");
        }
    }
}

TablatureDurationFont::TablatureDurationFont()
{
    for (size_t i = 0; i < size_t(TabVal::NUM_OF); i++) {
        displayValue[i] = Char(u'a' + static_cast<char16_t>(i));
    }
}

//---------------------------------------------------------
//   Tablature font presets
//---------------------------------------------------------

static TablatureFretFont makeFretFontPreset(const String& family, const String& displayName)
{
    // Create fret font from a regular font
    TablatureFretFont f;
    f.family = family;
    f.displayName = displayName;
    return f;
}

static TablatureFretFont makeMuseScoreFretFontPreset(const String& family, const String& displayName)
{
    // Create fret font from a specialist MuseScore tablature font
    TablatureFretFont f;
    f.family = family;
    f.displayName = displayName;
    f.defSize = 10.0;
    f.displayDigit[10] = String(u"X");
    for (size_t i = 0; i < NUM_OF_BASSSTRING_SLASHES; ++i) {
        f.slashChar[i] = String(Char(u'A' + static_cast<char16_t>(i)));
    }
    return f;
}

static TablatureDurationFont makeDurationFontPreset(const String& family, const String& displayName,
                                                    Spatium beamWidth = GRID_BEAM_DEF_WIDTH,
                                                    Spatium stemHeight = GRID_STEM_DEF_HEIGHT,
                                                    Spatium stemWidth = GRID_STEM_DEF_WIDTH,
                                                    DurationType zeroBeamLevel = DurationType::V_QUARTER)
{
    TablatureDurationFont f;
    f.family = family;
    f.displayName = displayName;
    f.gridBeamWidth = beamWidth;
    f.gridStemHeight = stemHeight;
    f.gridStemWidth = stemWidth;
    f.zeroBeamLevel = zeroBeamLevel;
    return f;
}

void StaffType::initTabFonts()
{
    // keep displayName strings and the order/count of these presets unchanged:
    // saved scores reference a preset by displayName (<460) or by
    // positional index (>=460)
    m_fretFonts = {
        makeFretFontPreset(u"FreeSans", u"MuseScore Tab Sans"),
        makeFretFontPreset(u"FreeSerif", u"MuseScore Tab Serif"),
        makeMuseScoreFretFontPreset(u"MuseScoreTabRenaiss", u"MuseScore Tab Renaiss"),
        makeMuseScoreFretFontPreset(u"MuseScoreTabPhalese", u"MuseScore Phalèse"),
        makeMuseScoreFretFontPreset(u"MuseScoreTabBonneuilDeVisee", u"MuseScore Bonneuil-de Visée"),
        makeMuseScoreFretFontPreset(u"MuseScoreTabBonneuilGaultier", u"MuseScore Bonneuil-Gaultier"),
        makeMuseScoreFretFontPreset(u"MuseScoreTabDowland", u"MuseScore Dowland"),
        makeMuseScoreFretFontPreset(u"MuseScoreTabLuteDidactic", u"MuseScore Lute Didactic"),
    };

    m_durationFonts = {
        makeDurationFontPreset(u"MuseScoreTabModern", u"MuseScore Tab Modern",
                               0.5_sp, 3.0_sp, 0.1_sp, DurationType::V_QUARTER),
        makeDurationFontPreset(u"MuseScoreTabItalian", u"MuseScore Tab Italian",
                               0.15_sp, 1.75_sp, 0.15_sp, DurationType::V_WHOLE),
        makeDurationFontPreset(u"MuseScoreTabFrench", u"MuseScore Tab French",
                               0.30_sp, 3.125_sp, 0.21_sp, DurationType::V_QUARTER),
        makeDurationFontPreset(u"MuseScoreTabFrenchBaroqueHeadless", u"MuseScore French Baroque (headless)"),
        makeDurationFontPreset(u"MuseScoreTabFrenchBaroque", u"MuseScore French Baroque"),
    };
}

//---------------------------------------------------------
//   fontNames
//
//    returns a list of display names for the fonts  configured to work with Tablatures;
//    the index of a name in the list can be used to retrieve the font data with fontData()
//---------------------------------------------------------

std::vector<String> StaffType::tabFontNames(bool bDuration)
{
    std::vector<String> names;
    if (bDuration) {
        names.reserve(m_durationFonts.size());
        for (const TablatureDurationFont& f : m_durationFonts) {
            names.push_back(f.displayName);
        }
    } else {
        names.reserve(m_fretFonts.size());
        for (const TablatureFretFont& f : m_fretFonts) {
            names.push_back(f.displayName);
        }
    }
    return names;
}

//---------------------------------------------------------
//   fontData
//
//    retrieves data about a Tablature font.
//    returns: true if idx is valid | false if it is not
//---------------------------------------------------------

bool StaffType::tabFontData(bool bDuration, size_t nIdx, double& pSize, double& pYOff)
{
    if (bDuration) {
        if (nIdx < m_durationFonts.size()) {
            TablatureDurationFont f = m_durationFonts.at(nIdx);
            pSize = f.defSize;
            pYOff = f.defYOffset;
            return true;
        }
    } else {
        TablatureFretFont f = nIdx < m_fretFonts.size() ? m_fretFonts.at(nIdx) : TablatureFretFont();
        pSize = f.defSize;
        pYOff = f.defYOffset;
        return true;
    }
    return false;
}

const MStyle& StaffType::style() const
{
    if (!m_score) {
        return DefaultStyle::defaultStyle();
    }

    return m_score->style();
}

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

double StaffType::spatium() const
{
    return style().spatium() * (isSmall() ? style().styleD(Sid::smallStaffMag) : 1.0) * userMag();
}

//=========================================================
//
//   BUILT-IN STAFF TYPES and STAFF TYPE PRESETS
//
//=========================================================

static const int _defaultPreset[STAFF_GROUP_MAX] =
{ 0,                    // default pitched preset is "stdNormal"
  4,                    // default percussion preset is "perc5lines"
  5                     // default tab preset is "tab6StrCommon"
};

//---------------------------------------------------------
//   Static functions for StaffType presets
//---------------------------------------------------------

const StaffType* StaffType::preset(StaffTypes idx)
{
    if (int(idx) < 0 || int(idx) >= int(m_presets.size())) {
        return &m_presets[0];
    }

    return &m_presets[int(idx)];
}

const StaffType* StaffType::presetFromXmlName(const String& xmlName)
{
    for (size_t i = 0; i < m_presets.size(); ++i) {
        if (m_presets[i].xmlName() == xmlName) {
            return &m_presets[i];
        }
    }

    return nullptr;
}

const StaffType* StaffType::getDefaultPreset(StaffGroup grp)
{
    int _idx = _defaultPreset[int(grp)];
    return &m_presets[_idx];
}

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

std::vector<StaffType> StaffType::m_presets;
/* *INDENT-OFF* */
void StaffType::initStaffTypes(const Color& defaultColor)
{
    initTabFonts();                         // set up TAB font presets, before initStaffTypes()

    // keep in sync with enum class StaffTypes
    m_presets = {
//                       group,              staff type,                    lin stpOff  dist clef   bars stmless time  key    ledger invis     color
        StaffType(StaffGroup::STANDARD,   StaffTypes::STANDARD,             5, 0,     1,   true,  true, false, true, true, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, StaffTypes::PERC_1LINE,           1, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, StaffTypes::PERC_2LINE,           2, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, StaffTypes::PERC_3LINE,           3, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, StaffTypes::PERC_5LINE,           5, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),

//                 group            staff type,               lin stpOff dist clef   bars stemless time  invis     color       duration font       genDur textStyle fret font                               duration symbol repeat       thru    minim style                  onLin  rests  stmDn  stmThr upsDn  sTFing nums  bkTied
        StaffType(StaffGroup::TAB, StaffTypes::TAB_6SIMPLE,   6,  0, 1.5, true,  true, true,  false, false, defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_6COMMON,   6,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_6FULL,     6,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED, true,  true,  true,  true,  false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_4SIMPLE,   4,  0, 1.5, true,  true, true,  false, false, defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_4COMMON,   4,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_4FULL,     4,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED, true,  true,  true,  true,  false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_5SIMPLE,   5,  0, 1.5, true,  true, true,  false, false, defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_5COMMON,   5,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_5FULL,     5,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED, true,  true,  true,  true,  false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_UKULELE,   4,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_BALALAJKA, 3,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_DULCIMER,  3,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, true,  true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_ITALIAN,   6,  0, 1.5, false, true, true,  true, false,  defaultColor, u"MuseScore Tab Italian",true,  false, u"MuseScore Tab Renaiss",                  TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,    true,  true,  false, false, true,  false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_FRENCH,    6,  0, 1.5, false, true, true,  true, false,  defaultColor, u"MuseScore Tab French", true,  false, u"MuseScore Tab Renaiss",                  TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,    false, false, false, false, false, false, false, false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_7COMMON,   7,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_8COMMON,   8,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_9COMMON,   9,  0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_10COMMON,  10, 0, 1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER, true,  true,  true,  false, false, true,  true,  true),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_7SIMPLE,   7,  0, 1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_8SIMPLE,   8,  0, 1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_9SIMPLE,   9,  0, 1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
        StaffType(StaffGroup::TAB, StaffTypes::TAB_10SIMPLE,  10, 0, 1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", false, true,  u"MuseScore Tab Sans",                     TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,    true,  false, true,  false, false, false, true,  false),
    };
}
/* *INDENT-ON* */
} // namespace mu::engraving
