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

#include "figuredbass.h"

#include "io/file.h"

#include "rw/xmlreader.h"

#include "style/textstyle.h"

#include "chord.h"
#include "factory.h"
#include "linkedobjects.h"
#include "measure.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::engraving {
static std::vector<FiguredBassFont> g_FBFonts;

const std::vector<FiguredBassFont>& FiguredBass::FBFonts()
{
    return g_FBFonts;
}

//---------------------------------------------------------
//   figuredBassStyle
//---------------------------------------------------------

static const ElementStyle figuredBassStyle {
    { Sid::figuredBassMinDistance,             Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   figuredBassTextStyle
//---------------------------------------------------------

static const ElementStyle figuredBassTextStyle {
    { Sid::figuredBassFontFace,                Pid::FONT_FACE },
    { Sid::figuredBassFontSize,                Pid::FONT_SIZE },
    { Sid::figuredBassFontStyle,               Pid::FONT_STYLE },
};

//---------------------------------------------------------
//   F I G U R E D   B A S S   I T E M
//---------------------------------------------------------

// used for indexed access to parenthesis chars
// (these is no normAccidToChar[], as accidentals may use mult. chars in normalized display):
const Char FiguredBassItem::NORM_PARENTH_TO_CHAR[int(FiguredBassItem::Parenthesis::NUMOF)] =
{ 0, '(', ')', '[', ']' };

FiguredBassItem::FiguredBassItem(FiguredBass* parent, int l)
    : EngravingItem(ElementType::FIGURED_BASS_ITEM, parent), m_ord(l)
{
    m_prefix     = m_suffix = Modifier::NONE;
    m_digit      = FBIDigitNone;
    m_parenth[0]  = m_parenth[1] = m_parenth[2] = m_parenth[3] = m_parenth[4] = Parenthesis::NONE;
    m_contLine   = ContLine::NONE;
}

FiguredBassItem::FiguredBassItem(const FiguredBassItem& item)
    : EngravingItem(item)
{
    m_ord         = item.m_ord;
    m_prefix     = item.m_prefix;
    m_digit      = item.m_digit;
    m_suffix     = item.m_suffix;
    m_parenth[0]  = item.m_parenth[0];
    m_parenth[1]  = item.m_parenth[1];
    m_parenth[2]  = item.m_parenth[2];
    m_parenth[3]  = item.m_parenth[3];
    m_parenth[4]  = item.m_parenth[4];
    m_contLine   = item.m_contLine;
}

FiguredBassItem::~FiguredBassItem()
{
}

//---------------------------------------------------------
//   FiguredBassItem parse()
//
// converts a string into a property-based representation, if possible;
// return true on success | false if the string is non-conformant
//---------------------------------------------------------

bool FiguredBassItem::parse(String& str)
{
    int retVal;

    parseParenthesis(str, 0);
    retVal = parsePrefixSuffix(str, true);            // prefix
    if (retVal == -1) {
        return false;
    }
    parseParenthesis(str, 1);
    retVal = parseDigit(str);                         // digit
    if (retVal == -1) {
        return false;
    }
    parseParenthesis(str, 2);
    retVal = parsePrefixSuffix(str, false);           // suffix
    if (retVal == -1) {
        return false;
    }
    parseParenthesis(str, 3);
    // check for a possible cont. line symbol(s)
    m_contLine = ContLine::NONE;                         // contLine
    if (!str.empty() && (str.at(0) == u'-' || str.at(0) == u'_')) {             // 1 symbol: simple continuation
        m_contLine = ContLine::SIMPLE;
        str.remove(0, 1);
    }
    while (!str.empty() && (str.at(0) == u'-' || str.at(0) == u'_')) {          // more than 1 symbol: extended continuation
        m_contLine = ContLine::EXTENDED;
        str.remove(0, 1);
    }
    parseParenthesis(str, 4);

    // remove useless parentheses, moving external parentheses toward central digit element
    if (m_prefix == Modifier::NONE && m_parenth[1] == Parenthesis::NONE) {
        m_parenth[1] = m_parenth[0];
        m_parenth[0] = Parenthesis::NONE;
    }
    if (m_digit == FBIDigitNone && m_parenth[2] == Parenthesis::NONE) {
        m_parenth[2] = m_parenth[1];
        m_parenth[1] = Parenthesis::NONE;
    }
    if (m_contLine == ContLine::NONE && m_parenth[3] == Parenthesis::NONE) {
        m_parenth[3] = m_parenth[4];
        m_parenth[4] = Parenthesis::NONE;
    }
    if (m_suffix == Modifier::NONE && m_parenth[2] == Parenthesis::NONE) {
        m_parenth[2] = m_parenth[3];
        m_parenth[3] = Parenthesis::NONE;
    }

    // some checks:
    // if some extra input, str is not conformant
    if (!str.empty()) {
        return false;
    }
    // can't have BOTH prefix and suffix
    // prefix, digit, suffix and cont.line cannot be ALL empty
    // suffix cannot combine with empty digit
    if ((m_prefix != Modifier::NONE && m_suffix != Modifier::NONE)
        || (m_prefix == Modifier::NONE && m_digit == FBIDigitNone && m_suffix == Modifier::NONE && m_contLine == ContLine::NONE)
        || ((m_suffix == Modifier::CROSS || m_suffix == Modifier::BACKSLASH || m_suffix == Modifier::SLASH)
            && m_digit == FBIDigitNone)) {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   FiguredBassItem parsePrefixSuffix()
//
//    scans str to extract prefix or suffix properties. Stops at the first char which cannot fit.
//    Fitting chars are removed from str. DOES NOT generate any display text
//
// returns the number of Chars read from str or -1 if prefix / suffix has an illegal format
// (no prefix / suffix at all IS legal)
//---------------------------------------------------------

int FiguredBassItem::parsePrefixSuffix(String& str, bool bPrefix)
{
    Modifier* dest  = bPrefix ? &m_prefix : &m_suffix;
    bool done  = false;
    size_t size  = str.size();
    str = str.trimmed();

    *dest       = Modifier::NONE;

    while (str.size()) {
        switch (str.at(0).unicode()) {
        case 'b':
            if (*dest != Modifier::NONE) {
                if (*dest == Modifier::FLAT) {          // FLAT may double a previous FLAT
                    *dest = Modifier::DOUBLEFLAT;
                } else {
                    return -1;                        // but no other combination is acceptable
                }
            } else {
                *dest = Modifier::FLAT;
            }
            break;
        case 'h':
            if (*dest != Modifier::NONE) {              // cannot combine with any other accidental
                return -1;
            }
            *dest = Modifier::NATURAL;
            break;
        case '#':
            if (*dest != Modifier::NONE) {
                if (*dest == Modifier::SHARP) {         // SHARP may double a previous SHARP
                    *dest = Modifier::DOUBLESHARP;
                } else {
                    return -1;                        // but no other combination is acceptable
                }
            } else {
                *dest = Modifier::SHARP;
            }
            break;
        case '+':
            // accept '+' as both a prefix and a suffix for harmony notation
            if (*dest != Modifier::NONE) {              // cannot combine with any other accidental
                return -1;
            }
            *dest = Modifier::CROSS;
            break;
        // '\\' and '/' go into the suffix
        case '\\':
            if (m_suffix != Modifier::NONE) {            // cannot combine with any other accidental
                return -1;
            }
            m_suffix = Modifier::BACKSLASH;
            break;
        case '/':
            if (m_suffix != Modifier::NONE) {            // cannot combine with any other accidental
                return -1;
            }
            m_suffix = Modifier::SLASH;
            break;
        default:                                     // any other char: no longer in prefix/suffix
            done = true;
            break;
        }
        if (done) {
            break;
        }
        str.remove(0, 1);                             // 'eat' the char and continue
    }

    return static_cast<int>(size - str.size());                        // return how many chars we had read into prefix/suffix
}

//---------------------------------------------------------
//   FiguredBassItem parseDigit()
//
//    scans str to extract digit properties. Stops at the first char which cannot belong to digit part.
//    Fitting chars are removed from str. DOES NOT generate any display text
//
// returns the number of Chars read from str or -1 if no legal digit can be constructed
// (no digit at all IS legal)
//---------------------------------------------------------

int FiguredBassItem::parseDigit(String& str)
{
    size_t size   = str.size();
    str        = str.trimmed();

    m_digit = FBIDigitNone;

    while (str.size()) {
        // any digit acceptable
        if (str.at(0) >= u'0' && str.at(0) <= u'9') {
            if (m_digit == FBIDigitNone) {
                m_digit = 0;
            }
            m_digit = m_digit * 10 + (str.at(0).unicode() - '0');
            str.remove(0, 1);
        }
        // anything else: no longer in digit part
        else {
            break;
        }
    }

    return static_cast<int>(size - str.size());
}

//---------------------------------------------------------
//   FiguredBassItem parseParenthesis()
//
//    scans str to extract a (possible) parenthesis, stores its code into parenth[parenthIdx]
//    and removes it from str. Only looks at first str char.
//
// returns the number of Chars read from str (actually 0 or 1).
//---------------------------------------------------------

int FiguredBassItem::parseParenthesis(String& str, int parenthIdx)
{
    if (str.empty()) {
        return 0;
    }

    char16_t c = str.at(0).unicode();
    Parenthesis code = Parenthesis::NONE;
    switch (c) {
    case '(':
        code = Parenthesis::ROUNDOPEN;
        break;
    case ')':
        code = Parenthesis::ROUNDCLOSED;
        break;
    case '[':
        code =Parenthesis::SQUAREDOPEN;
        break;
    case ']':
        code = Parenthesis::SQUAREDCLOSED;
        break;
    default:
        break;
    }
    m_parenth[parenthIdx] = code;
    if (code != Parenthesis::NONE) {
        str.remove(0, 1);
        return 1;
    }
    return 0;
}

//---------------------------------------------------------
//   FiguredBassItem normalizedText()
//
// returns a string with the normalized text, i.e. the text displayed while editing;
// this is a standard textual representation of the item properties
//---------------------------------------------------------

String FiguredBassItem::normalizedText() const
{
    String str;
    if (m_parenth[0] != Parenthesis::NONE) {
        str.append(NORM_PARENTH_TO_CHAR[int(m_parenth[0])]);
    }

    if (m_prefix != Modifier::NONE) {
        switch (m_prefix) {
        case Modifier::FLAT:
            str.append(u'b');
            break;
        case Modifier::NATURAL:
            str.append(u'h');
            break;
        case Modifier::SHARP:
            str.append(u'#');
            break;
        case Modifier::CROSS:
            str.append(u'+');
            break;
        case Modifier::DOUBLEFLAT:
            str.append(u"bb");
            break;
        case Modifier::DOUBLESHARP:
            str.append(u"##");
            break;
        default:
            break;
        }
    }

    if (m_parenth[1] != Parenthesis::NONE) {
        str.append(NORM_PARENTH_TO_CHAR[int(m_parenth[1])]);
    }

    // digit
    if (m_digit != FBIDigitNone) {
        str.append(String::number(m_digit));
    }

    if (m_parenth[2] != Parenthesis::NONE) {
        str.append(NORM_PARENTH_TO_CHAR[int(m_parenth[2])]);
    }

    // suffix
    if (m_suffix != Modifier::NONE) {
        switch (m_suffix) {
        case Modifier::FLAT:
            str.append(u'b');
            break;
        case Modifier::NATURAL:
            str.append(u'h');
            break;
        case Modifier::SHARP:
            str.append(u'#');
            break;
        case Modifier::CROSS:
            str.append(u'+');
            break;
        case Modifier::BACKSLASH:
            str.append(u'\\');
            break;
        case Modifier::SLASH:
            str.append(u'/');
            break;
        case Modifier::DOUBLEFLAT:
            str.append(u"bb");
            break;
        case Modifier::DOUBLESHARP:
            str.append(u"##");
            break;
        default:
            break;
        }
    }

    if (m_parenth[3] != Parenthesis::NONE) {
        str.append(NORM_PARENTH_TO_CHAR[int(m_parenth[3])]);
    }
    if (m_contLine > ContLine::NONE) {
        str.append('_');
        if (m_contLine > ContLine::SIMPLE) {
            str.append('_');
        }
    }
    if (m_parenth[4] != Parenthesis::NONE) {
        str.append(NORM_PARENTH_TO_CHAR[int(m_parenth[4])]);
    }

    return str;
}

//---------------------------------------------------------
//   PROPERTY METHODS
//---------------------------------------------------------

PropertyValue FiguredBassItem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FBPREFIX:
        return int(m_prefix);
    case Pid::FBDIGIT:
        return m_digit;
    case Pid::FBSUFFIX:
        return int(m_suffix);
    case Pid::FBCONTINUATIONLINE:
        return int(m_contLine);
    case Pid::FBPARENTHESIS1:
        return int(m_parenth[0]);
    case Pid::FBPARENTHESIS2:
        return int(m_parenth[1]);
    case Pid::FBPARENTHESIS3:
        return int(m_parenth[2]);
    case Pid::FBPARENTHESIS4:
        return int(m_parenth[3]);
    case Pid::FBPARENTHESIS5:
        return int(m_parenth[4]);
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

bool FiguredBassItem::setProperty(Pid propertyId, const PropertyValue& v)
{
    score()->addRefresh(canvasBoundingRect());
    int val = v.toInt();
    switch (propertyId) {
    case Pid::FBPREFIX:
        if (val < int(Modifier::NONE) || val >= int(Modifier::NUMOF)) {
            return false;
        }
        m_prefix = (Modifier)val;
        break;
    case Pid::FBDIGIT:
        if (val < 1 || val > 9) {
            return false;
        }
        m_digit = val;
        break;
    case Pid::FBSUFFIX:
        if (val < int(Modifier::NONE) || val >= int(Modifier::NUMOF)) {
            return false;
        }
        m_suffix = (Modifier)val;
        break;
    case Pid::FBCONTINUATIONLINE:
        m_contLine = (ContLine)val;
        break;
    case Pid::FBPARENTHESIS1:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        m_parenth[0] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS2:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        m_parenth[1] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS3:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        m_parenth[2] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS4:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        m_parenth[3] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS5:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        m_parenth[4] = (Parenthesis)val;
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue FiguredBassItem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::FBPREFIX:
    case Pid::FBSUFFIX:
        return int(Modifier::NONE);
    case Pid::FBDIGIT:
        return FBIDigitNone;
    case Pid::FBCONTINUATIONLINE:
        return false;
    default:
        return EngravingItem::propertyDefault(id);
    }
}

void FiguredBassItem::regenerateDisplayText()
{
    // re-generate displayText
    renderer()->layoutItem(this);
}

//---------------------------------------------------------
//   UNDOABLE PROPERTY SETTERS
//---------------------------------------------------------

void FiguredBassItem::undoSetPrefix(Modifier pref)
{
    if (pref <= Modifier::CROSS) {
        undoChangeProperty(Pid::FBPREFIX, (int)pref);
        // if setting some prefix and there is a suffix already, clear suffix
        if (pref != Modifier::NONE && m_suffix != Modifier::NONE) {
            undoChangeProperty(Pid::FBSUFFIX, int(Modifier::NONE));
        }
        regenerateDisplayText();
    }
}

void FiguredBassItem::undoSetDigit(int digit)
{
    if (digit >= 0 && digit <= 9) {
        undoChangeProperty(Pid::FBDIGIT, digit);
        regenerateDisplayText();
    }
}

void FiguredBassItem::undoSetSuffix(Modifier suff)
{
    undoChangeProperty(Pid::FBSUFFIX, int(suff));
    // if setting some suffix and there is a prefix already, clear prefix
    if (suff != Modifier::NONE && m_prefix != Modifier::NONE) {
        undoChangeProperty(Pid::FBPREFIX, int(Modifier::NONE));
    }
    regenerateDisplayText();
}

void FiguredBassItem::undoSetContLine(ContLine val)
{
    undoChangeProperty(Pid::FBCONTINUATIONLINE, int(val));
    regenerateDisplayText();
}

void FiguredBassItem::undoSetParenth1(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS1, int(par));
    regenerateDisplayText();
}

void FiguredBassItem::undoSetParenth2(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS2, int(par));
    regenerateDisplayText();
}

void FiguredBassItem::undoSetParenth3(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS3, int(par));
    regenerateDisplayText();
}

void FiguredBassItem::undoSetParenth4(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS4, int(par));
    regenerateDisplayText();
}

void FiguredBassItem::undoSetParenth5(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS5, int(par));
    regenerateDisplayText();
}

//---------------------------------------------------------
//   startsWithParenthesis
//---------------------------------------------------------

bool FiguredBassItem::startsWithParenthesis() const
{
    if (m_prefix != Modifier::NONE) {
        return m_parenth[0] != Parenthesis::NONE;
    }
    if (m_digit != FBIDigitNone) {
        return m_parenth[1] != Parenthesis::NONE;
    }
    if (m_suffix != Modifier::NONE) {
        return m_parenth[2] != Parenthesis::NONE;
    }
    return false;
}

//---------------------------------------------------------
//   F I G U R E D   B A S S
//---------------------------------------------------------

FiguredBass::FiguredBass(Segment* parent)
    : TextBase(ElementType::FIGURED_BASS, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&figuredBassStyle);
    // figured bass inherits from TextBase for layout purposes
    // but there is no specific text style to use as a basis for styled properties
    // (this is true for historical reasons due to the special layout of figured bass)
    // override the styled property definitions
    for (const auto& p : *textStyle(textStyleType())) {
        setPropertyFlags(p.pid, PropertyFlags::NOSTYLE);
    }
    for (const StyledProperty& p : figuredBassTextStyle) {
        setPropertyFlags(p.pid, PropertyFlags::STYLED);
        setProperty(p.pid, styleValue(p.pid, p.sid));
    }
    setOnNote(true);
    setTicks(Fraction(0, 1));
    muse::DeleteAll(m_items);
    m_items.clear();
}

FiguredBass::FiguredBass(const FiguredBass& fb)
    : TextBase(fb)
{
    setOnNote(fb.onNote());
    setTicks(fb.ticks());
    for (auto i : fb.m_items) {       // deep copy is needed
        FiguredBassItem* fbi = new FiguredBassItem(*i);
        fbi->setParent(this);
        m_items.push_back(fbi);
    }
//      items = fb.items;
}

FiguredBass::~FiguredBass()
{
    for (FiguredBassItem* item : m_items) {
        delete item;
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid FiguredBass::getPropertyStyle(Pid id) const
{
    // do not use TextBase::getPropertyStyle
    // as most text style properties do not apply
    for (const StyledProperty& p : figuredBassTextStyle) {
        if (p.pid == id) {
            return p.sid;
        }
    }
    return EngravingItem::getPropertyStyle(id);
}

void FiguredBass::startEdit(EditData& ed)
{
    clearItems();
    renderer()->layoutText1(this);   // re-layout without F.B.-specific formatting.
    TextBase::startEdit(ed);
}

bool FiguredBass::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    if ((ed.key == Key_Left || ed.key == Key_Right) && (ed.modifiers & ControlModifier)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

void FiguredBass::endEdit(EditData& ed)
{
    TextBase::endEdit(ed);
    regenerateText();
}

void FiguredBass::regenerateText()
{
    // as the standard text editor keeps inserting spurious HTML formatting and styles
    // retrieve and work only on the plain text
    const String txt = plainText();
    if (txt.isEmpty()) { // if no text, nothing to do
        return;
    }

    // split text into lines and create an item for each line
    StringList list = txt.split(u'\n', muse::SkipEmptyParts);
    clearItems();
    String normalizedText;
    int idx = 0;
    for (String str : list) {
        FiguredBassItem* pItem = new FiguredBassItem(this, idx++);
        if (!pItem->parse(str)) {               // if any item fails parsing
            clearItems();
            // TODO: this `startCmd` call is possibly invalid
            score()->startCmd(TranslatableString("undoableAction", "Regenerate figured bass text"));
            triggerLayout();
            score()->endCmd();
            delete pItem;
            return;
        }
        addItemToLinked(pItem);

        // add item normalized text
        if (!normalizedText.isEmpty()) {
            normalizedText.append('\n');
        }
        normalizedText.append(pItem->normalizedText());
    }
    // if all items parsed and text is styled, replaced entered text with normalized text
    if (m_items.size()) {
        undoChangeProperty(Pid::TEXT, normalizedText);
    }

    // TODO: this `startCmd` call is possibly invalid
    score()->startCmd(TranslatableString("undoableAction", "Regenerate figured bass text"));
    triggerLayout();
    score()->endCmd();
}

//---------------------------------------------------------
//   setSelected /setVisible
//
//    forward flags to items
//---------------------------------------------------------

void FiguredBass::setSelected(bool flag)
{
    EngravingItem::setSelected(flag);
    for (FiguredBassItem* item : m_items) {
        item->setSelected(flag);
    }
}

void FiguredBass::setVisible(bool flag)
{
    EngravingItem::setVisible(flag);
    for (FiguredBassItem* item : m_items) {
        item->setVisible(flag);
    }
}

//---------------------------------------------------------
//   nextFiguredBass
//
//    returns the next *contiguous* FiguredBass element if it exists,
//    i.e. the FiguredBass element which starts where 'this' ends
//    returns 0 if none
//---------------------------------------------------------

FiguredBass* FiguredBass::nextFiguredBass() const
{
    if (m_ticks <= Fraction(0, 1)) {                                      // if _ticks unset, no clear idea of when 'this' ends
        return 0;
    }
    Segment* nextSegm;                                   // the Segment beyond this' segment
    Fraction nextTick = segment()->tick() + m_ticks;      // the tick beyond this' duration

    // locate the ChordRest segment right after this' end
    nextSegm = score()->tick2segment(nextTick, true, SegmentType::ChordRest);
    if (nextSegm == 0) {
        return 0;
    }

    // scan segment annotations for an existing FB element in the this' staff
    for (EngravingItem* e : nextSegm->annotations()) {
        if (e->type() == ElementType::FIGURED_BASS && e->track() == track()) {
            return toFiguredBass(e);
        }
    }

    return 0;
}

//---------------------------------------------------------
//   additionalContLineX
//
//    if there is a continuation line, without other text elements, at pagePosY, returns its X coord (in page coords)
//    returns 0 if no cont.line there or if there are text elements before the cont.line
//
//    In practice, returns the X coord of a cont. line which can be the continuation of a previous cont. line
//
//    Note: pagePosY is the Y coord of the FiguredBassItem containing the line, not of the line itself,
//    as line position might depend on styles.
//---------------------------------------------------------

double FiguredBass::additionalContLineX(double pagePosY) const
{
    PointF pgPos = pagePos();
    for (FiguredBassItem* fbi : m_items) {
        // if item has cont.line but nothing before it
        // and item Y coord near enough to pagePosY
        if (fbi->contLine() != FiguredBassItem::ContLine::NONE
            && fbi->digit() == FBIDigitNone
            && fbi->prefix() == FiguredBassItem::Modifier::NONE
            && fbi->suffix() == FiguredBassItem::Modifier::NONE
            && fbi->parenth4() == FiguredBassItem::Parenthesis::NONE
            && std::abs(pgPos.y() + fbi->ldata()->pos().y() - pagePosY) < 0.05) {
            return pgPos.x() + fbi->ldata()->pos().x();
        }
    }

    return 0.0;                                 // no suitable line
}

//---------------------------------------------------------
//   PROPERTY METHODS
//---------------------------------------------------------

PropertyValue FiguredBass::getProperty(Pid propertyId) const
{
    return TextBase::getProperty(propertyId);
}

bool FiguredBass::setProperty(Pid propertyId, const PropertyValue& v)
{
    score()->addRefresh(canvasBoundingRect());
    if (propertyId == Pid::TEXT_LINKED_TO_MASTER) {
        if (TextBase::setProperty(propertyId, v)) {
            regenerateText();
            return true;
        }
        return false;
    }
    return TextBase::setProperty(propertyId, v);
}

PropertyValue FiguredBass::propertyDefault(Pid id) const
{
    return TextBase::propertyDefault(id);
}

void FiguredBass::clearItems()
{
    const std::list<EngravingObject*> links = linkList();
    for (EngravingObject* linkedObject : links) {
        if (!linkedObject || !linkedObject->isFiguredBass()) {
            continue;
        }
        if (linkedObject == this) {
            muse::DeleteAll(m_items);
            m_items.clear();
        } else {
            bool isThisTextLinked = getProperty(Pid::TEXT_LINKED_TO_MASTER).toBool();
            bool isOtherTextLinked = linkedObject->getProperty(Pid::TEXT_LINKED_TO_MASTER).toBool();
            if ((score()->isMaster() && !isOtherTextLinked)
                || (!score()->isMaster() && !isThisTextLinked)) {
                continue;
            }
            FiguredBass* linkedFb = toFiguredBass(linkedObject);
            muse::DeleteAll(linkedFb->m_items);
            linkedFb->m_items.clear();
        }
    }
}

void FiguredBass::addItemToLinked(FiguredBassItem* item)
{
    const std::list<EngravingObject*> links = linkList();
    for (EngravingObject* linkedObject : links) {
        if (!linkedObject || !linkedObject->isFiguredBass()) {
            continue;
        }
        Score* linkedScore = linkedObject->score();
        if (linkedObject == this) {
            item->setTrack(track());
            item->setParent(this);
            m_items.push_back(item);
            score()->doUndoAddElement(item);
        } else {
            bool isThisTextLinked = getProperty(Pid::TEXT_LINKED_TO_MASTER).toBool();
            bool isOtherTextLinked = linkedObject->getProperty(Pid::TEXT_LINKED_TO_MASTER).toBool();
            if ((score()->isMaster() && !isOtherTextLinked)
                || (!score()->isMaster() && !isThisTextLinked)) {
                continue;
            }
            FiguredBass* linkedFb = toFiguredBass(linkedObject);
            FiguredBassItem* itemClone = item->clone();

            itemClone->linkTo(item);
            itemClone->setTrack(linkedFb->track());
            itemClone->setParent(linkedFb);
            itemClone->setScore(linkedFb->score());
            linkedFb->appendItem(itemClone);
            linkedScore->doUndoAddElement(itemClone);
        }
    }
}

//---------------------------------------------------------
//   STATIC FUNCTION
//    adding a new FiguredBass to a Segment;
//    the main purpose of this function is to ensure that ONLY ONE F.b. element exists for each Segment/staff;
//    it either re-uses an existing FiguredBass or creates a new one if none is found;
//    returns the FiguredBass and sets pNew to true if it has been newly created.
//
//    Sets an initial duration of the element up to the next ChordRest of the same staff.
//
//    As the F.b. very concept requires the base chord to have ONLY ONE note,
//    FiguredBass elements are created and looked for only in the first track of the staff.
//---------------------------------------------------------

FiguredBass* FiguredBass::addFiguredBassToSegment(Segment* seg, track_idx_t track, const Fraction& extTicks, bool* pNew)
{
    Fraction endTick;                        // where this FB is initially assumed to end
    staff_idx_t staff = track / VOICES;          // convert track to staff
    track = staff * VOICES;                     // first track for this staff

    // scan segment annotations for an existing FB element in the same staff
    FiguredBass* fb = 0;
    for (EngravingItem* e : seg->annotations()) {
        if (e->type() == ElementType::FIGURED_BASS && (e->track() / VOICES) == staff) {
            // an FB already exists in segment: re-use it
            fb = toFiguredBass(e);
            *pNew = false;
            endTick = seg->tick() + fb->ticks();
            break;
        }
    }
    if (fb == 0) {                            // no FB at segment: create new
        fb = Factory::createFiguredBass(seg);
        fb->setTrack(track);
        fb->setParent(seg);

        // locate next SegChordRest in the same staff to estimate presumed duration of element
        endTick = Fraction::max();
        Segment* nextSegm;
        for (voice_idx_t iVoice = 0; iVoice < VOICES; iVoice++) {
            nextSegm = seg->nextCR(track + iVoice);
            if (nextSegm && nextSegm->tick() < endTick) {
                endTick = nextSegm->tick();
            }
        }
        if (endTick == Fraction::max()) {               // no next segment: set up to score end
            Measure* meas = seg->score()->lastMeasure();
            endTick = meas->tick() + meas->ticks();
        }
        fb->setTicks(endTick - seg->tick());

        // set onNote status
        fb->setOnNote(false);                   // assume not onNote
        for (track_idx_t i = track; i < track + VOICES; i++) {           // if segment has chord in staff, set onNote
            if (seg->element(i) && seg->element(i)->type() == ElementType::CHORD) {
                fb->setOnNote(true);
                break;
            }
        }
        *pNew = true;
    }

    // if we are extending a previous FB
    if (extTicks > Fraction(0, 1)) {
        // locate previous FB for same staff
        Segment* prevSegm;
        FiguredBass* prevFB = 0;
        for (prevSegm = seg->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE); prevSegm;
             prevSegm = prevSegm->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
            for (EngravingItem* e : prevSegm->annotations()) {
                if (e->type() == ElementType::FIGURED_BASS && (e->track()) == track) {
                    prevFB = toFiguredBass(e);             // previous FB found
                    break;
                }
            }
            if (prevFB) {
                // if previous FB did not stop more than extTicks before this FB...
                Fraction delta = seg->tick() - prevFB->segment()->tick();
                if (prevFB->ticks() + extTicks >= delta) {
                    prevFB->setTicks(delta);                // update prev FB ticks to last up to this FB
                }
                break;
            }
        }
    }
    return fb;
}

//---------------------------------------------------------
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

bool FiguredBassFont::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "family") {
            family = e.readText();
        } else if (tag == "displayName") {
            displayName = e.readText();
        } else if (tag == "defaultPitch") {
            defPitch = e.readDouble();
        } else if (tag == "defaultLineHeight") {
            defLineHeight = e.readDouble();
        } else if (tag == "parenthesisRoundOpen") {
            displayParenthesis[1] = e.readText().at(0);
        } else if (tag == "parenthesisRoundClosed") {
            displayParenthesis[2] = e.readText().at(0);
        } else if (tag == "parenthesisSquareOpen") {
            displayParenthesis[3] = e.readText().at(0);
        } else if (tag == "parenthesisSquareClosed") {
            displayParenthesis[4] = e.readText().at(0);
        } else if (tag == "doubleflat") {
            displayAccidental[int(FiguredBassItem::Modifier::DOUBLEFLAT)]= e.readText().at(0);
        } else if (tag == "flat") {
            displayAccidental[int(FiguredBassItem::Modifier::FLAT)]      = e.readText().at(0);
        } else if (tag == "natural") {
            displayAccidental[int(FiguredBassItem::Modifier::NATURAL)]   = e.readText().at(0);
        } else if (tag == "sharp") {
            displayAccidental[int(FiguredBassItem::Modifier::SHARP)]     = e.readText().at(0);
        } else if (tag == "doublesharp") {
            displayAccidental[int(FiguredBassItem::Modifier::DOUBLESHARP)]= e.readText().at(0);
        } else if (tag == "cross") {
            displayAccidental[int(FiguredBassItem::Modifier::CROSS)]     = e.readText().at(0);
        } else if (tag == "backslash") {
            displayAccidental[int(FiguredBassItem::Modifier::BACKSLASH)] = e.readText().at(0);
        } else if (tag == "slash") {
            displayAccidental[int(FiguredBassItem::Modifier::SLASH)]     = e.readText().at(0);
        } else if (tag == "digit") {
            int digit = e.intAttribute("value");
            if (digit < 0 || digit > 9) {
                return false;
            }
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "simple") {
                    displayDigit[int(FiguredBassItem::Style::MODERN)]  [digit][int(FiguredBassItem::Combination::SIMPLE)]
                        = e.readText().at(0);
                } else if (t == "crossed") {
                    displayDigit[int(FiguredBassItem::Style::MODERN)]  [digit][int(FiguredBassItem::Combination::CROSSED)]
                        = e.readText().at(0);
                } else if (t == "backslashed") {
                    displayDigit[int(FiguredBassItem::Style::MODERN)]  [digit][int(FiguredBassItem::Combination::BACKSLASHED)]
                        = e.readText().at(0);
                } else if (t == "slashed") {
                    displayDigit[int(FiguredBassItem::Style::MODERN)]  [digit][int(FiguredBassItem::Combination::SLASHED)]
                        = e.readText().at(0);
                } else if (t == "simpleHistoric") {
                    displayDigit[int(FiguredBassItem::Style::HISTORIC)][digit][int(FiguredBassItem::Combination::SIMPLE)]
                        = e.readText().at(0);
                } else if (t == "crossedHistoric") {
                    displayDigit[int(FiguredBassItem::Style::HISTORIC)][digit][int(FiguredBassItem::Combination::CROSSED)]
                        = e.readText().at(0);
                } else if (t == "backslashedHistoric") {
                    displayDigit[int(FiguredBassItem::Style::HISTORIC)][digit][int(FiguredBassItem::Combination::BACKSLASHED)]
                        = e.readText().at(0);
                } else if (t == "slashedHistoric") {
                    displayDigit[int(FiguredBassItem::Style::HISTORIC)][digit][int(FiguredBassItem::Combination::SLASHED)]
                        = e.readText().at(0);
                } else {
                    e.unknown();
                    return false;
                }
            }
        } else {
            e.unknown();
            return false;
        }
    }
    displayParenthesis[0] = displayAccidental[int(FiguredBassItem::Modifier::NONE)] = ' ';
    return true;
}

//---------------------------------------------------------
//   Read Configuration File
//
//    reads a configuration and appends read data to g_FBFonts
//    resets everything and reads the built-in config file if fileName is null or empty
//---------------------------------------------------------

bool FiguredBass::readConfigFile(const String& fileName)
{
    String path;

    if (fileName.isEmpty()) {         // defaults to built-in xml
        path = u":/fonts/fonts_figuredbass.xml";
        g_FBFonts.clear();
    } else {
        path = fileName;
    }

    File fi(path);
    if (!fi.open(IODevice::ReadOnly)) {
        LOGE() << "Cannot open figured bass description: " << fi.filePath();
        return false;
    }
    XmlReader e(&fi);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            // String version = e.attribute(String("version"));
            // StringList sl = version.split('.');
            // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();

            while (e.readNextStartElement()) {
                if (e.name() == "font") {
                    FiguredBassFont f;
                    if (f.read(e)) {
                        g_FBFonts.push_back(f);
                    } else {
                        return false;
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
//   Get Font Names
//
//    returns a list of display names for the fonts  configured to work with Figured Bass;
//    the index of a name in the list can be used to retrieve the font data with fontData()
//---------------------------------------------------------

std::list<String> FiguredBass::fontNames()
{
    std::list<String> names;
    for (const FiguredBassFont& f : g_FBFonts) {
        names.push_back(f.displayName);
    }
    return names;
}

//---------------------------------------------------------
//   Get Font Data
//
//    retrieves data about a Figured Bass font.
//    returns: true if idx is valid | false if it is not
// any of the pointer parameter can be null, if that datum is not needed
//---------------------------------------------------------

bool FiguredBass::fontData(int nIdx, String* pFamily, String* pDisplayName,
                           double* pSize, double* pLineHeight)
{
    if (nIdx >= 0 && nIdx < static_cast<int>(g_FBFonts.size())) {
        FiguredBassFont f = g_FBFonts.at(nIdx);
        if (pFamily) {
            *pFamily          = f.family;
        }
        if (pDisplayName) {
            *pDisplayName     = f.displayName;
        }
        if (pSize) {
            *pSize            = f.defPitch;
        }
        if (pLineHeight) {
            *pLineHeight      = f.defLineHeight;
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   hasParentheses
//
//   return true if any FiguredBassItem starts with a parenthesis
//---------------------------------------------------------

bool FiguredBass::hasParentheses() const
{
    for (FiguredBassItem* item : m_items) {
        if (item->startsWithParenthesis()) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//
// METHODS BELONGING TO OTHER CLASSES
//
//    Work In Progress: kept here until the FiguredBass framework is reasonably set up;
//    To be finally moved to their respective class implementation files.
//
//---------------------------------------------------------

//---------------------------------------------------------
//   Score::addFiguredBass
//    called from Keyboard Accelerator & menus
//---------------------------------------------------------

FiguredBass* Score::addFiguredBass()
{
    EngravingItem* el = selection().element();
    if (!el || (!(el->isNote()) && !(el->isRest()) && !(el->isFiguredBass()))) {
        MScore::setError(MsError::NO_NOTE_FIGUREDBASS_SELECTED);
        return 0;
    }

    FiguredBass* fb;
    bool bNew = true;
    if (el->isNote()) {
        ChordRest* cr = toNote(el)->chord();
        fb = FiguredBass::addFiguredBassToSegment(cr->segment(), cr->staffIdx() * VOICES, Fraction(0, 1), &bNew);
    } else if (el->isRest()) {
        ChordRest* cr = toRest(el);
        fb = FiguredBass::addFiguredBassToSegment(cr->segment(), cr->staffIdx() * VOICES, Fraction(0, 1), &bNew);
    } else if (el->isFiguredBass()) {
        fb = toFiguredBass(el);
        bNew = false;
    } else {
        return 0;
    }

    if (fb == 0) {
        return 0;
    }

    if (bNew) {
        undoAddElement(fb);
    }
    select(fb, SelectType::SINGLE, 0);
    return fb;
}
}
