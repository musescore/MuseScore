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

#include "stafftype.h"

#include "translation.h"
#include "io/file.h"
#include "draw/fontmetrics.h"
#include "draw/types/pen.h"
#include "rw/xmlreader.h"
#include "types/typesconv.h"
#include "style/style.h"

#include "chord.h"
#include "mscore.h"
#include "navigate.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace muse::io;
using namespace mu::engraving;

#define TAB_DEFAULT_LINE_SP   (1.5)

namespace mu::engraving {
//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

#define TAB_DEFAULT_DUR_YOFFS (-1.0)

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
    setFretFontName(m_fretFonts[0].displayName);
}

StaffType::StaffType(StaffGroup sg, const String& xml, const String& name, int lines, int stpOff, double lineDist,
                     bool genClef, bool showBarLines, bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLines, bool invisible,
                     const Color& color)
    : m_group(sg), m_xmlName(xml), m_name(name),
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

StaffType::StaffType(StaffGroup sg, const String& xml, const String& name, int lines, int stpOff, double lineDist,
                     bool genClef,
                     bool showBarLines, bool stemless, bool genTimesig, bool invisible, const Color& color,
                     const String& durFontName, double durFontSize, double durFontUserY, double genDur,
                     const String& fretFontName, double fretFontSize, double fretFontUserY,
                     TablatureSymbolRepeat symRepeat, bool linesThrough, TablatureMinimStyle minimStyle, bool onLines,
                     bool showRests, bool stemsDown, bool stemThrough, bool upsideDown, bool showTabFingering, bool useNumbers,
                     bool showBackTied)
{
    UNUSED(invisible);
    m_color = color;
    m_group   = sg;
    m_xmlName = xml;
    m_name    = name;
    setLines(lines);
    setStepOffset(stpOff);
    setLineDistance(Spatium(lineDist));
    setGenClef(genClef);
    setShowBarlines(showBarLines);
    setStemless(stemless);
    setGenTimesig(genTimesig);
    setGenKeysig(sg != StaffGroup::TAB);
    setDurationFontName(durFontName);
    setDurationFontSize(durFontSize);
    setDurationFontUserY(durFontUserY);
    setGenDurations(genDur);
    setFretFontName(fretFontName);
    setFretFontSize(fretFontSize);
    setFretFontUserY(fretFontUserY);
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
    equal &= (m_xmlName == st.m_xmlName);
    equal &= (m_name == st.m_name);
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
    equal &= (m_durationMetricsValid == st.m_durationMetricsValid);
    equal &= (m_fretBoxH == st.m_fretBoxH);
    equal &= (m_deadFretBoxH == st.m_deadFretBoxH);
    equal &= (m_fretBoxY == st.m_fretBoxY);
    equal &= (m_deadFretBoxY == st.m_deadFretBoxY);
    equal &= (m_fretFont == st.m_fretFont);
    equal &= (m_fretFontIdx == st.m_fretFontIdx);
    equal &= (m_fretYOffset == st.m_fretYOffset);
    equal &= (m_fretMetricsValid == st.m_fretMetricsValid);
    equal &= (m_refDPI == st.m_refDPI);

    return equal;
}

StaffTypes StaffType::type() const
{
    static const std::map<String, StaffTypes> xmlNameToType {
        { u"stdNormal", StaffTypes::STANDARD },

        { u"perc1Line", StaffTypes::PERC_1LINE },
        { u"perc2Line", StaffTypes::PERC_2LINE },
        { u"perc3Line", StaffTypes::PERC_3LINE },
        { u"perc5Line", StaffTypes::PERC_5LINE },

        { u"tab4StrSimple", StaffTypes::TAB_4SIMPLE },
        { u"tab4StrCommon", StaffTypes::TAB_4COMMON },
        { u"tab4StrFull", StaffTypes::TAB_4FULL },

        { u"tab5StrSimple", StaffTypes::TAB_5SIMPLE },
        { u"tab5StrCommon", StaffTypes::TAB_5COMMON },
        { u"tab5StrFull", StaffTypes::TAB_5FULL },

        { u"tab6StrSimple", StaffTypes::TAB_6SIMPLE },
        { u"tab6StrCommon", StaffTypes::TAB_6COMMON },
        { u"tab6StrFull", StaffTypes::TAB_6FULL },

        { u"tabUkulele", StaffTypes::TAB_UKULELE },
        { u"tabBalajka", StaffTypes::TAB_BALALAJKA },
        { u"tabDulcimer", StaffTypes::TAB_DULCIMER },

        { u"tab6StrItalian", StaffTypes::TAB_ITALIAN },
        { u"tab6StrFrench", StaffTypes::TAB_FRENCH },

        { u"tab7StrCommon", StaffTypes::TAB_7COMMON },
        { u"tab8StrCommon", StaffTypes::TAB_8COMMON },
        { u"tab9StrCommon", StaffTypes::TAB_9COMMON },
        { u"tab10StrCommon", StaffTypes::TAB_10COMMON },

        { u"tab7StrSimple", StaffTypes::TAB_7SIMPLE },
        { u"tab8StrSimple", StaffTypes::TAB_8SIMPLE },
        { u"tab9StrSimple", StaffTypes::TAB_9SIMPLE },
        { u"tab10StrSimple", StaffTypes::TAB_10SIMPLE },
    };

    return muse::value(xmlNameToType, m_xmlName, StaffTypes::STANDARD);
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

bool StaffType::isHiddenElementOnTab(const MStyle& style, Sid commonTabStyle, Sid simpleTabStyle) const
{
    return (isCommonTabStaff() && !style.styleB(commonTabStyle)) || (isSimpleTabStaff() && !style.styleB(simpleTabStyle));
}

//---------------------------------------------------------
//   doty1
//    get y dot position of first repeat barline dot
//---------------------------------------------------------

double StaffType::doty1() const
{
    return m_lineDistance.val() * (static_cast<double>((m_lines - 1) / 2) - 0.5);
}

//---------------------------------------------------------
//   doty2
//    get y dot position of second repeat barline dot
//---------------------------------------------------------

double StaffType::doty2() const
{
    return m_lineDistance.val() * (static_cast<double>(m_lines / 2) + 0.5);
}

//---------------------------------------------------------
//   setOnLines
//---------------------------------------------------------

void StaffType::setOnLines(bool val)
{
    m_onLines = val;
    m_durationMetricsValid = m_fretMetricsValid = false;
}

//---------------------------------------------------------
//   setDurationMetrics
//    checks whether the internally computed metrics are is still valid and re-computes them, if not
//---------------------------------------------------------

void StaffType::setDurationMetrics() const
{
    if (m_durationMetricsValid && m_refDPI == DPI) {           // metrics are still valid
        return;
    }

// FontMetrics returns results unreliably rounded to integral pixels;
// use a scaled up font and then scale computed values down
    Font font(durationFont());
    font.setPointSizeF(m_durationFontSize);
    FontMetrics fm(font);
    String txt(m_durationFonts[m_durationFontIdx].displayValue, int(TabVal::NUM_OF));
    RectF bb(fm.tightBoundingRect(txt));
    // raise symbols by a default margin and, if marks are above lines, by half the line distance
    // (converted from spatium units to raster units)
    m_durationGridYOffset = (TAB_DEFAULT_DUR_YOFFS - (m_onLines ? 0.0 : lineDistance().val() * 0.5)) * SPATIUM20;
    // this is the bottomest point of any duration sign
    m_durationYOffset = m_durationGridYOffset;
    // move symbols so that the lowest margin 'sits' on the base line:
    // move down by the whole part above (negative) the base line
    // ( -bb.y() ) then up by the whole height ( -bb.height() )
    m_durationYOffset        -= (bb.height() + bb.y()) / 100.0;
    m_durationBoxH           = bb.height() / 100.0;
    m_durationBoxY           = m_durationGridYOffset - bb.height() / 100.0;
    // keep track of the conditions under which metrics have been computed
    m_refDPI = DPI;
    m_durationMetricsValid = true;
}

void StaffType::setFretMetrics(const MStyle& style) const
{
    if (m_fretMetricsValid && m_refDPI == DPI) {
        return;
    }

    FontMetrics fm(fretFont());
    RectF bb;
    // compute vertical displacement
    if (m_useNumbers) {
        // compute total height of used characters
        String txt;
        for (int idx = 0; idx < 10; idx++) {    // use only first 10 digits
            txt.append(m_fretFonts[m_fretFontIdx].displayDigit[idx]);
        }
        bb = fm.tightBoundingRect(txt);
        // for numbers: centre on '0': move down by the whole part above (negative)
        // the base line ( -bb.y() ) then up by half the whole height ( -bb.height()/2 )
        RectF bx(fm.tightBoundingRect(m_fretFonts[m_fretFontIdx].displayDigit[0]));
        m_fretYOffset = -(bx.y() + bx.height() / 2.0);
        // _fretYOffset = -(bb.y() + bb.height()/2.0);  // <- using bbox of all chars
    } else {
        // compute total height of used characters
        String txt(m_fretFonts[m_fretFontIdx].displayLetter, NUM_OF_LETTERFRETS);
        bb = fm.tightBoundingRect(txt);
        // for letters: centre on the 'a' ascender, by moving down half of the part above the base line in bx
        RectF bx(fm.tightBoundingRect(m_fretFonts[m_fretFontIdx].displayLetter[0]));
        m_fretYOffset = -bx.y() / 2.0;
    }

    // Calculate position for dead fret marks - these must be centred separately based on their glyph
    RectF deadBb = fm.tightBoundingRect(m_fretFonts[m_fretFontIdx].xChar);
    double lineThickness = style.styleS(Sid::staffLineWidth).val() * SPATIUM20 * 0.5;
    m_deadFretYOffset = -deadBb.y() / 2.0 + lineThickness;

    // if on string, we are done; if between strings, raise by half line distance
    if (!m_onLines) {
        double lineAdj = lineDistance().val() * SPATIUM20 * 0.5;
        m_fretYOffset -= lineAdj;
        m_deadFretYOffset -= lineAdj;
    }

    // from _fretYOffset, compute _fretBoxH and _fretBoxY
    m_fretBoxH = bb.height();
    m_fretBoxY = bb.y() + m_fretYOffset;

    m_deadFretBoxH = deadBb.height();
    m_deadFretBoxY = deadBb.y() + m_deadFretYOffset;

    // keep track of the conditions under which metrics have been computed
    m_refDPI = DPI;
    m_fretMetricsValid = true;
}

//---------------------------------------------------------
//   setDurationFontName / setFretFontName
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
    m_durationFont.setFamily(m_durationFonts[idx].family, Font::Type::Tablature);
    m_durationFontIdx = idx;
    m_durationMetricsValid = false;
}

void StaffType::setFretFontName(const String& name)
{
    size_t idx;
    String locName = name;
    // convert old names for two built-in fonts which have changed of name
    if (name == "MuseScore Tab Late Renaiss") {
        locName = u"MuseScore Phalèse";
    }
    for (idx = 0; idx < m_fretFonts.size(); idx++) {
        if (m_fretFonts[idx].displayName == locName) {
            break;
        }
    }
    if (idx >= m_fretFonts.size()) {
        idx = 0;              // if name not found, use first font
    }
    m_fretFont.setFamily(m_fretFonts[idx].family, Font::Type::Tablature);
    m_fretFontIdx = idx;
    m_fretMetricsValid = false;
}

//---------------------------------------------------------
//   durationBoxH / durationBoxY
//---------------------------------------------------------

double StaffType::durationBoxH() const
{
    if (!m_genDurations && !m_stemless) {
        return 0.0;
    }
    setDurationMetrics();
    return m_durationBoxH;
}

double StaffType::durationBoxY() const
{
    if (!m_genDurations && !m_stemless) {
        return 0.0;
    }
    setDurationMetrics();
    return m_durationBoxY + m_durationFontUserY * SPATIUM20;
}

//---------------------------------------------------------
//   setDurationFontSize / setFretFontSize
//---------------------------------------------------------

void StaffType::setDurationFontSize(double val)
{
    m_durationFontSize = val;
    m_durationFont.setPointSizeF(val);
    m_durationMetricsValid = false;
}

void StaffType::setFretFontSize(double val)
{
    m_fretFontSize = val;
    m_fretFont.setPointSizeF(val);
    m_fretMetricsValid = false;
}

//---------------------------------------------------------
//   chordRestStemPosY / chordStemPos / chordStemPosBeam / chordStemLength
//
//    computes the stem data for the given chord, according to TAB settings
//    NOTE: unit: spatium, position: relative to chord (DIFFERENT from Chord own functions)
//
//   chordRestStemPosY
//          returns the vertical position of stem start point
//---------------------------------------------------------

double StaffType::chordRestStemPosY(const ChordRest* chordRest) const
{
    if (stemThrough()) {            // does not make sense for "stems through staves" setting; just return top line vert. position
        return 0.0;
    }

    // if stems beside staff, position are fixed, but take into account delta for half notes
    double delta                                 // displacement for half note stems (if used)
        =   // if half notes have not a short stem OR not a half note => 0
          (minimStyle() != TablatureMinimStyle::SHORTER || chordRest->durationType().type() != DurationType::V_HALF)
          ? 0.0
          :       // if stem is up, displace of half stem length down (positive)
                  // if stem is down, displace of half stem length up (negative)
          (chordRest->up()
           ? -STAFFTYPE_TAB_DEFAULTSTEMLEN_UP : STAFFTYPE_TAB_DEFAULTSTEMLEN_DN) * 0.5;
    // if fret marks above lines and chordRest is up, move half a line distance up
    if (!onLines() && chordRest->up()) {
        delta -= m_lineDistance.val() * 0.5;
    }
    double y
        = (chordRest->up() ? STAFFTYPE_TAB_DEFAULTSTEMPOSY_UP : (m_lines - 1) * m_lineDistance.val() + STAFFTYPE_TAB_DEFAULTSTEMPOSY_DN)
          + delta;
    return y;
}

//---------------------------------------------------------
//   chordStemPos
//    return position of note at other side of beam
//---------------------------------------------------------

PointF StaffType::chordStemPos(const Chord* chord) const
{
    double y;
    if (stemThrough()) {
        // if stems are through staff, stem goes from farthest note string
        y = (chord->up() ? chord->downString() : chord->upString()) * m_lineDistance.val();
    } else {
        // if stems are beside staff, stem start point has a fixed vertical position,
        // according to TAB parameters and stem up/down
        y = chordRestStemPosY(chord);
    }
    return PointF(chordStemPosX(chord), y);
}

//---------------------------------------------------------
//   chordStemPosBeam
//          return position of note at beam side of stem
//---------------------------------------------------------

PointF StaffType::chordStemPosBeam(const Chord* chord) const
{
    double y = (stemsDown() ? chord->downString() : chord->upString()) * m_lineDistance.val();

    return PointF(chordStemPosX(chord), y);
}

//---------------------------------------------------------
//   chordStemLength
//          return length of stem
//---------------------------------------------------------

double StaffType::chordStemLength(const Chord* chord) const
{
    double stemLen;
    // if stems are through staff, length should be computed by relevant chord algorithm;
    // here, just return default length (= 1 'octave' = 3.5 line spaces)
    if (stemThrough()) {
        return STAFFTYPE_TAB_DEFAULTSTEMLEN_THRU * m_lineDistance.val();
    }
    // if stems beside staff, length is fixed, but take into account shorter half note stems
    else {
        bool shrt = (minimStyle() == TablatureMinimStyle::SHORTER) && (chord->durationType().type() == DurationType::V_HALF);
        stemLen = (stemsDown() ? STAFFTYPE_TAB_DEFAULTSTEMLEN_DN : STAFFTYPE_TAB_DEFAULTSTEMLEN_UP)
                  * (shrt ? STAFFTYPE_TAB_SHORTSTEMRATIO : 1.0) * (chord->style().styleB(Sid::useWideBeams) ? 1.25 : 1.0);
    }
    // scale length by scale of parent chord, but relative to scale of context staff
    return stemLen * chord->mag() / chord->staff()->staffMag(chord->tick());
}

//---------------------------------------------------------
//   fretString / durationString
//
//    construct the text string for a given fret / duration
//---------------------------------------------------------

static const String unknownFret = String(u"?");

String StaffType::fretString(int fret, int string, bool deadNote) const
{
    if (fret == INVALID_FRET_INDEX) {
        return unknownFret;
    }
    if (deadNote) {
        return String(m_fretFonts[m_fretFontIdx].xChar);
    } else {
        bool hasFret;
        String text  = tabBassStringPrefix(string, &hasFret);
        if (!hasFret) {             // if the notation does not allow to fret this string,
            return text;            // return the prefix only
        }
        // otherwise, add to prefix the relevant digit/letter string
        return text
               + (m_useNumbers
                  ? (fret >= NUM_OF_DIGITFRETS ? unknownFret : m_fretFonts[m_fretFontIdx].displayDigit[fret])
                  : (fret >= NUM_OF_LETTERFRETS ? unknownFret : m_fretFonts[m_fretFontIdx].displayLetter[fret]));
    }
}

String StaffType::durationString(DurationType type, int dots) const
{
    String s = m_durationFonts[m_durationFontIdx].displayValue[int(type)];
    for (int count=0; count < dots; count++) {
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
            return m_fretFonts[m_fretFontIdx].displayDigit[strg + 1];
        }
        // if a frettable bass string, return an empty string
        return String();
    } else {
        // bass string notation
        // if above the max bass string which can be fretted with letter notation
        // return a number with the bass string index itself
        if (bassStrgIdx > NUM_OF_BASSSTRINGS_WITH_LETTER) {
            *hasFret    = false;
            return m_fretFonts[m_fretFontIdx].displayDigit[bassStrgIdx - 1];
        }
        // if a frettable bass string, return a character with the relevant num. of slashes;
        // note that the number of slashes is bassStrgIdx-1 (1st bass has no slash)
        // and slashChar[] is 0-based (slashChar[0] => 1 slash, ...), whence the -2
        String prefix    = bassStrgIdx > 1
                           ? String(m_fretFonts[m_fretFontIdx].slashChar[bassStrgIdx - 2]) : String();
        return prefix;
    }
}

//---------------------------------------------------------
//   drawInputStringMarks
//
//    in TAB's, draws the marks within the input 'blue cursor' required to identify the current target input string.
//
//    Implements the specific of historic TAB styles for instruments with more strings than TAB lines.
//    For strings normally represented by TAB lines, no mark is required.
//    For strings not represented by TAB lines (e.g. bass strings in lutes and similar),
//    either a sequence of slashes OR some ledger line-like lines OR the ordinal of the string
//    are used, according to the TAB style (French or Italian) and the string position.
//
//    Note: assumes the string parameter is within legal bounds, i.e.:
//    0 <= string <= [instrument strings] - 1
//
//    p       the Painter to draw into
//    string  the instrument physical string for which to draw the mark (0 = top string)
//    voice   the current input voice (affects mark colour)
//    rect    the rect of the 'blue rectangle' showing the input position
//---------------------------------------------------------

void StaffType::drawInputStringMarks(Painter* p, int string, const Color& selectionColor, const RectF& rect) const
{
    if (m_group != StaffGroup::TAB) {
        return;
    }

    static constexpr double LEDGER_LINE_THICKNESS = 0.15; // in sp
    static constexpr double LEDGER_LINE_LEFTX = 0.25; // in % of cursor rectangle width
    static constexpr double LEDGER_LINE_RIGHTX = 0.75; // in % of cursor rectangle width

    double spatium = SPATIUM20;
    double lineDist = m_lineDistance.val() * spatium;
    bool hasFret = false;
    String text = tabBassStringPrefix(string, &hasFret);
    double lw = LEDGER_LINE_THICKNESS * spatium; // use a fixed width
    Pen pen(selectionColor, lw);
    p->setPen(pen);
    // draw conventional 'ledger lines', if required
    int numOfLedgerLines  = numOfTabLedgerLines(string);
    double x1 = rect.x() + rect.width() * LEDGER_LINE_LEFTX;
    double x2 = rect.x() + rect.width() * LEDGER_LINE_RIGHTX;
    // cursor rect is 1 line dist. high, and it is:
    // centred on the line for "frets on strings"    => lower top ledger line 1/2 line dist.
    // sitting on the line for "frets above strings" => lower top ledger line 1 full line dist
    double y = rect.top() + lineDist * (m_onLines ? 0.5 : 1.0);
    for (int i = 0; i < numOfLedgerLines; i++) {
        p->drawLine(LineF(x1, y, x2, y));
        y += lineDist / numOfLedgerLines;     // insert other lines between top line and tab body
    }
    // draw the text, if any
    if (!text.isEmpty()) {
        Font f = fretFont();
        f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
        p->setFont(f);
        p->drawText(PointF(rect.left(), rect.top() + lineDist), text);
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

double StaffType::physStringToYOffset(int strg) const
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
    return yOffset * m_lineDistance.val();
}

//---------------------------------------------------------
//   TabDurationSymbol
//---------------------------------------------------------

TabDurationSymbol::TabDurationSymbol(ChordRest* parent)
    : EngravingItem(ElementType::TAB_DURATION_SYMBOL, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
    m_tab        = 0;
    m_text       = String();
}

TabDurationSymbol::TabDurationSymbol(ChordRest* parent, const StaffType* tab, DurationType type, int dots)
    : EngravingItem(ElementType::TAB_DURATION_SYMBOL, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
    setDuration(type, dots, tab);
}

TabDurationSymbol::TabDurationSymbol(const TabDurationSymbol& e)
    : EngravingItem(e)
{
    m_tab = e.m_tab;
    m_text = e.m_text;
}

//---------------------------------------------------------
//   layout2
//
//    Second step: after horizontal positions of elements involved are defined,
//    compute width of 'grid beams'
//---------------------------------------------------------

void TabDurationSymbol::layout2()
{
    LayoutData* ldata = mutldata();
    // if not within a TAB or not a MEDIALFINAL grid element, do nothing
    if (!m_tab || ldata->beamGrid != TabBeamGrid::MEDIALFINAL) {
        return;
    }

    // get 'grid' beam length from page positions of this' chord and previous chord
    Chord* chord       = toChord(explicitParent());
    ChordRest* prevChord   = prevChordRest(chord, true);
    if (chord == nullptr || prevChord == nullptr) {
        return;
    }
    double mags        = magS();
    double beamLen     = prevChord->pagePos().x() - chord->pagePos().x();            // negative
    // page pos. difference already includes any magnification in effect:
    // scale it down, as it will be magnified again during drawing
    ldata->beamLength = beamLen / mags;
    // update bbox x and w, but keep current y and h
    RectF bbox = ldata->bbox();
    bbox.setLeft(beamLen);
    // set bbox width to half a stem width (magnified) plus beam length (already magnified)
    bbox.setWidth(m_tab->m_durationFonts[m_tab->m_durationFontIdx].gridStemWidth * spatium() * 0.5 * mags - beamLen);

    ldata->setBbox(bbox);
}

//---------------------------------------------------------
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

bool TablatureFretFont::read(XmlReader& e)
{
    defPitch    = 9.0;
    defYOffset  = 0.0;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        int val = e.intAttribute("value");

        if (tag == "family") {
            family = e.readText();
        } else if (tag == "displayName") {
            displayName = e.readText();
        } else if (tag == "defaultPitch") {
            defPitch = e.readDouble();
        } else if (tag == "defaultYOffset") {
            defYOffset = e.readDouble();
        } else if (tag == "mark") {
            String sval = e.attribute("value");
            int num  = e.intAttribute("number", 1);
            String txt(e.readText());
            if (sval.size() < 1) {
                return false;
            }
            if (sval == "x") {
                xChar = txt.at(0);
            } else if (sval == "ghost") {
                deadNoteChar = txt.at(0);
            } else if (sval == "slash") {
                // limit within legal range
                if (num < 1) {
                    num = 1;
                }
                if (num > NUM_OF_BASSSTRING_SLASHES) {
                    num = NUM_OF_BASSSTRING_SLASHES;
                }
                slashChar[num - 1] = txt;
            }
        } else if (tag == "fret") {
            bool bLetter = e.intAttribute("letter");
            String txt(e.readText());
            if (bLetter) {
                if (val >= 0 && val < NUM_OF_LETTERFRETS) {
                    displayLetter[val] = txt.at(0);
                }
            } else {
                if (val >= 0 && val < NUM_OF_DIGITFRETS) {
                    displayDigit[val] = txt;
                }
            }
        } else {
            e.unknown();
            return false;
        }
    }
    return true;
}

bool TablatureDurationFont::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "family") {
            family = e.readText();
        } else if (tag == "displayName") {
            displayName = e.readText();
        } else if (tag == "defaultPitch") {
            defPitch = e.readDouble();
        } else if (tag == "defaultYOffset") {
            defYOffset = e.readDouble();
        } else if (tag == "beamWidth") {
            gridBeamWidth = e.readDouble();
        } else if (tag == "stemHeight") {
            gridStemHeight = e.readDouble();
        } else if (tag == "stemWidth") {
            gridStemWidth = e.readDouble();
        } else if (tag == "zeroBeamValue") {
            String val(e.readText());
            if (val == "longa") {
                zeroBeamLevel = DurationType::V_LONG;
            } else if (val == "brevis") {
                zeroBeamLevel = DurationType::V_BREVE;
            } else if (val == "semibrevis") {
                zeroBeamLevel = DurationType::V_WHOLE;
            } else if (val == "minima") {
                zeroBeamLevel = DurationType::V_HALF;
            } else if (val == "semiminima") {
                zeroBeamLevel = DurationType::V_QUARTER;
            } else if (val == "fusa") {
                zeroBeamLevel = DurationType::V_EIGHTH;
            } else if (val == "semifusa") {
                zeroBeamLevel = DurationType::V_16TH;
            } else if (val == "32") {
                zeroBeamLevel = DurationType::V_32ND;
            } else if (val == "64") {
                zeroBeamLevel = DurationType::V_64TH;
            } else if (val == "128") {
                zeroBeamLevel = DurationType::V_128TH;
            } else if (val == "256") {
                zeroBeamLevel = DurationType::V_256TH;
            } else if (val == "512") {
                zeroBeamLevel = DurationType::V_512TH;
            } else if (val == "1024") {
                zeroBeamLevel = DurationType::V_1024TH;
            } else {
                e.unknown();
            }
        } else if (tag == "duration") {
            String val = e.attribute("value");
            String txt(e.readText());
            Char chr = txt.at(0);
            if (val == "longa") {
                displayValue[int(TabVal::VAL_LONGA)] = chr;
            } else if (val == "brevis") {
                displayValue[int(TabVal::VAL_BREVIS)] = chr;
            } else if (val == "semibrevis") {
                displayValue[int(TabVal::VAL_SEMIBREVIS)] = chr;
            } else if (val == "minima") {
                displayValue[int(TabVal::VAL_MINIMA)] = chr;
            } else if (val == "semiminima") {
                displayValue[int(TabVal::VAL_SEMIMINIMA)] = chr;
            } else if (val == "fusa") {
                displayValue[int(TabVal::VAL_FUSA)] = chr;
            } else if (val == "semifusa") {
                displayValue[int(TabVal::VAL_SEMIFUSA)] = chr;
            } else if (val == "32") {
                displayValue[int(TabVal::VAL_32)] = chr;
            } else if (val == "64") {
                displayValue[int(TabVal::VAL_64)] = chr;
            } else if (val == "128") {
                displayValue[int(TabVal::VAL_128)] = chr;
            } else if (val == "256") {
                displayValue[int(TabVal::VAL_256)] = chr;
            } else if (val == "512") {
                displayValue[int(TabVal::VAL_512)] = chr;
            } else if (val == "1024") {
                displayValue[int(TabVal::VAL_1024)] = chr;
            } else if (val == "dot") {
                displayDot = chr;
            } else {
                e.unknown();
            }
        } else {
            e.unknown();
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   Read Configuration File
//
//    reads a configuration and appends read data to g_TABFonts
//    resets everything and reads the built-in config file if fileName is null or empty
//---------------------------------------------------------

bool StaffType::readConfigFile(const String& fileName)
{
    muse::io::path_t path;

    if (fileName.isEmpty()) {         // defaults to built-in xml
        path = ":/fonts/fonts_tablature.xml";
        m_durationFonts.clear();
        m_fretFonts.clear();
    } else {
        path = fileName;
    }

    File f(path);
    if (!f.exists() || !f.open(IODevice::ReadOnly)) {
        LOGE() << "Cannot open tablature font description: " << f.filePath();
        return false;
    }

    XmlReader e(&f);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            while (e.readNextStartElement()) {
                const AsciiStringView tag(e.name());
                if (tag == "fretFont") {
                    TablatureFretFont ff;
                    if (ff.read(e)) {
                        m_fretFonts.push_back(ff);
                    } else {
                        continue;
                    }
                } else if (tag == "durationFont") {
                    TablatureDurationFont df;
                    if (df.read(e)) {
                        m_durationFonts.push_back(df);
                    } else {
                        continue;
                    }
                } else {
                    e.unknown();
                }
            }
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   fontNames
//
//    returns a list of display names for the fonts  configured to work with Tablatures;
//    the index of a name in the list can be used to retrieve the font data with fontData()
//---------------------------------------------------------

std::vector<String> StaffType::fontNames(bool bDuration)
{
    std::vector<String> names;
    if (bDuration) {
        for (const TablatureDurationFont& f : m_durationFonts) {
            names.push_back(f.displayName);
        }
    } else {
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
// any of the pointer parameter can be null, if that datum is not needed
//---------------------------------------------------------

bool StaffType::fontData(bool bDuration, size_t nIdx, String* pFamily, String* pDisplayName,
                         double* pSize, double* pYOff)
{
    if (bDuration) {
        if (nIdx < m_durationFonts.size()) {
            TablatureDurationFont f = m_durationFonts.at(nIdx);
            if (pFamily) {
                *pFamily          = f.family;
            }
            if (pDisplayName) {
                *pDisplayName     = f.displayName;
            }
            if (pSize) {
                *pSize            = f.defPitch;
            }
            if (pYOff) {
                *pYOff            = f.defYOffset;
            }
            return true;
        }
    } else {
        if (nIdx < m_fretFonts.size()) {
            TablatureFretFont f = m_fretFonts.at(nIdx);
            if (pFamily) {
                *pFamily          = f.family;
            }
            if (pDisplayName) {
                *pDisplayName     = f.displayName;
            }
            if (pSize) {
                *pSize            = f.defPitch;
            }
            if (pYOff) {
                *pYOff            = f.defYOffset;
            }
            return true;
        }
    }
    return false;
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
    readConfigFile(String());            // get TAB font config, before initStaffTypes()

    // keep in sync with enum class StaffTypes
    m_presets = {
//                       group,              xml-name,  human-readable-name,          lin stpOff  dist clef   bars stmless time  key    ledger invis     color
        StaffType(StaffGroup::STANDARD,   u"stdNormal", muse::mtrc("engraving", "Standard"),        5, 0,     1,   true,  true, false, true, true, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, u"perc1Line", muse::mtrc("engraving", "Perc. 1 line"),    1, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, u"perc2Line", muse::mtrc("engraving", "Perc. 2 lines"),   2, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, u"perc3Line", muse::mtrc("engraving", "Perc. 3 lines"),   3, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),
        StaffType(StaffGroup::PERCUSSION, u"perc5Line", muse::mtrc("engraving", "Perc. 5 lines"),   5, 0,     1,   true,  true, false, true, false, true, false,  defaultColor),

//                 group            xml-name,     human-readable-name                  lin stpOff dist clef   bars stemless time   invis     color   duration font     size off genDur     fret font          size off  duration symbol repeat      thru       minim style              onLin  rests  stmDn  stmThr upsDn  sTFing nums  bkTied
        StaffType(StaffGroup::TAB, u"tab6StrSimple", muse::mtrc("engraving", "Tab. 6-str. simple"), 6, 0,     1.5, true,  true, true,  false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, u"tab6StrCommon", muse::mtrc("engraving", "Tab. 6-str. common"), 6, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab6StrFull",   muse::mtrc("engraving", "Tab. 6-str. full"),   6, 0,     1.5, true,  true, false, true, false,  defaultColor,  u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,  true,  true,  true,  true,  false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab4StrSimple", muse::mtrc("engraving", "Tab. 4-str. simple"), 4, 0,     1.5, true,  true, true,  false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, u"tab4StrCommon", muse::mtrc("engraving", "Tab. 4-str. common"), 4, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab4StrFull",   muse::mtrc("engraving", "Tab. 4-str. full"),   4, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,  true,  true,  true,  true,  false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab5StrSimple", muse::mtrc("engraving", "Tab. 5-str. simple"), 5, 0,     1.5, true,  true, true,  false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, u"tab5StrCommon", muse::mtrc("engraving", "Tab. 5-str. common"), 5, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab5StrFull",   muse::mtrc("engraving", "Tab. 5-str. full"),   5, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,  true,  true,  true,  true,  false, true, true, true),
        StaffType(StaffGroup::TAB, u"tabUkulele",    muse::mtrc("engraving", "Tab. ukulele"),       4, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true,  true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tabBalajka",    muse::mtrc("engraving", "Tab. balalaika"),     3, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true,  true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tabDulcimer",   muse::mtrc("engraving", "Tab. dulcimer"),      3, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true,  true,  false, true,  true, true, true),
        StaffType(StaffGroup::TAB, u"tab6StrItalian",muse::mtrc("engraving", "Tab. 6-str. Italian"),6, 0,     1.5, false, true, true,  true, false,  defaultColor,  u"MuseScore Tab Italian",15, 0, true,  u"MuseScore Tab Renaiss",10, 0, TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,   true,  true,  false, false, true,  false, true, false),
        StaffType(StaffGroup::TAB, u"tab6StrFrench", muse::mtrc("engraving", "Tab. 6-str. French"), 6, 0,     1.5, false, true, true,  true, false,  defaultColor,  u"MuseScore Tab French", 15, 0, true,  u"MuseScore Tab Renaiss",10, 0, TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,   false, false, false, false, false, false, false, false),
        StaffType(StaffGroup::TAB, u"tab7StrCommon", muse::mtrc("engraving", "Tab. 7-str. common"), 7, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab8StrCommon", muse::mtrc("engraving", "Tab. 8-str. common"), 8, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab9StrCommon", muse::mtrc("engraving", "Tab. 9-str. common"), 9, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab10StrCommon", muse::mtrc("engraving", "Tab. 10-str. common"), 10, 0,     1.5, true,  true, false, true, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true, true,  false, false, true, true, true),
        StaffType(StaffGroup::TAB, u"tab7StrSimple", muse::mtrc("engraving", "Tab. 7-str. simple"), 7, 0,     1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, u"tab8StrSimple", muse::mtrc("engraving", "Tab. 8-str. simple"), 8, 0,     1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, u"tab9StrSimple", muse::mtrc("engraving", "Tab. 9-str. simple"), 9, 0,     1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, u"tab10StrSimple", muse::mtrc("engraving", "Tab. 10-str. simple"), 10, 0,     1.5, true,  true, true, false, false,  defaultColor, u"MuseScore Tab Modern", 15, 0, false, u"MuseScore Tab Sans",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,true,  false, true,  false, false, false, true, false),
    };
}
/* *INDENT-ON* */
//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

double StaffType::spatium(const MStyle& style) const
{
    return style.spatium() * (isSmall() ? style.styleD(Sid::smallStaffMag) : 1.0) * userMag();
}
} // namespace mu::engraving
