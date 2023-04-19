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

#include "figuredbass.h"

#include "io/file.h"

#include "rw/xmlreader.h"

#include "draw/fontmetrics.h"
#include "draw/types/pen.h"

#include "style/textstyle.h"

#include "chord.h"
#include "factory.h"
#include "measure.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

namespace mu::engraving {
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

static constexpr double FB_CONTLINE_HEIGHT            = 0.875;     // the % of font EM to raise the cont. line at
                                                                   // (0 = top of font; 1 = bottom of font)
static constexpr double FB_CONTLINE_LEFT_PADDING      = 0.1875;    // (3/16sp) the blank space at the left of a cont. line (in sp)
static constexpr double FB_CONTLINE_OVERLAP           = 0.125;     // (1/8sp)  the overlap of an extended cont. line (in sp)
static constexpr double FB_CONTLINE_THICKNESS         = 0.09375;   // (3/32sp) the thickness of a cont. line (in sp)

// the array of configured fonts
static std::vector<FiguredBassFont> g_FBFonts;

//---------------------------------------------------------
//   F I G U R E D   B A S S   I T E M
//---------------------------------------------------------

// used for indexed access to parenthesis chars
// (these is no normAccidToChar[], as accidentals may use mult. chars in normalized display):
const Char FiguredBassItem::normParenthToChar[int(FiguredBassItem::Parenthesis::NUMOF)] =
{ 0, '(', ')', '[', ']' };

FiguredBassItem::FiguredBassItem(FiguredBass* parent, int l)
    : EngravingItem(ElementType::INVALID, parent), ord(l)
{
    _prefix     = _suffix = Modifier::NONE;
    _digit      = FBIDigitNone;
    parenth[0]  = parenth[1] = parenth[2] = parenth[3] = parenth[4] = Parenthesis::NONE;
    _contLine   = ContLine::NONE;
    textWidth   = 0;
}

FiguredBassItem::FiguredBassItem(const FiguredBassItem& item)
    : EngravingItem(item)
{
    ord         = item.ord;
    _prefix     = item._prefix;
    _digit      = item._digit;
    _suffix     = item._suffix;
    parenth[0]  = item.parenth[0];
    parenth[1]  = item.parenth[1];
    parenth[2]  = item.parenth[2];
    parenth[3]  = item.parenth[3];
    parenth[4]  = item.parenth[4];
    _contLine   = item._contLine;
    textWidth   = item.textWidth;
    _displayText= item._displayText;
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
    _contLine = ContLine::NONE;                         // contLine
    if (str.at(0) == u'-' || str.at(0) == u'_') {             // 1 symbol: simple continuation
        _contLine = ContLine::SIMPLE;
        str.remove(0, 1);
    }
    while (str.at(0) == u'-' || str.at(0) == u'_') {          // more than 1 symbol: extended continuation
        _contLine = ContLine::EXTENDED;
        str.remove(0, 1);
    }
    parseParenthesis(str, 4);

    // remove useless parentheses, moving external parentheses toward central digit element
    if (_prefix == Modifier::NONE && parenth[1] == Parenthesis::NONE) {
        parenth[1] = parenth[0];
        parenth[0] = Parenthesis::NONE;
    }
    if (_digit == FBIDigitNone && parenth[2] == Parenthesis::NONE) {
        parenth[2] = parenth[1];
        parenth[1] = Parenthesis::NONE;
    }
    if (_contLine == ContLine::NONE && parenth[3] == Parenthesis::NONE) {
        parenth[3] = parenth[4];
        parenth[4] = Parenthesis::NONE;
    }
    if (_suffix == Modifier::NONE && parenth[2] == Parenthesis::NONE) {
        parenth[2] = parenth[3];
        parenth[3] = Parenthesis::NONE;
    }

    // some checks:
    // if some extra input, str is not conformant
    if (str.size()) {
        return false;
    }
    // can't have BOTH prefix and suffix
    // prefix, digit, suffix and cont.line cannot be ALL empty
    // suffix cannot combine with empty digit
    if ((_prefix != Modifier::NONE && _suffix != Modifier::NONE)
        || (_prefix == Modifier::NONE && _digit == FBIDigitNone && _suffix == Modifier::NONE && _contLine == ContLine::NONE)
        || ((_suffix == Modifier::CROSS || _suffix == Modifier::BACKSLASH || _suffix == Modifier::SLASH)
            && _digit == FBIDigitNone)) {
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
    Modifier* dest  = bPrefix ? &_prefix : &_suffix;
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
            if (_suffix != Modifier::NONE) {            // cannot combine with any other accidental
                return -1;
            }
            _suffix = Modifier::BACKSLASH;
            break;
        case '/':
            if (_suffix != Modifier::NONE) {            // cannot combine with any other accidental
                return -1;
            }
            _suffix = Modifier::SLASH;
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

    _digit = FBIDigitNone;

    while (str.size()) {
        // any digit acceptable
        if (str.at(0) >= u'0' && str.at(0) <= u'9') {
            if (_digit == FBIDigitNone) {
                _digit = 0;
            }
            _digit = _digit * 10 + (str.at(0).unicode() - '0');
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
    parenth[parenthIdx] = code;
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
    if (parenth[0] != Parenthesis::NONE) {
        str.append(normParenthToChar[int(parenth[0])]);
    }

    if (_prefix != Modifier::NONE) {
        switch (_prefix) {
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

    if (parenth[1] != Parenthesis::NONE) {
        str.append(normParenthToChar[int(parenth[1])]);
    }

    // digit
    if (_digit != FBIDigitNone) {
        str.append(String::number(_digit));
    }

    if (parenth[2] != Parenthesis::NONE) {
        str.append(normParenthToChar[int(parenth[2])]);
    }

    // suffix
    if (_suffix != Modifier::NONE) {
        switch (_suffix) {
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

    if (parenth[3] != Parenthesis::NONE) {
        str.append(normParenthToChar[int(parenth[3])]);
    }
    if (_contLine > ContLine::NONE) {
        str.append('_');
        if (_contLine > ContLine::SIMPLE) {
            str.append('_');
        }
    }
    if (parenth[4] != Parenthesis::NONE) {
        str.append(normParenthToChar[int(parenth[4])]);
    }

    return str;
}

//---------------------------------------------------------
//   FiguredBassItem layout()
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------

void FiguredBassItem::layout()
{
    double h, w, x, x1, x2, y;

    // construct font metrics
    int fontIdx = 0;
    mu::draw::Font f(g_FBFonts.at(fontIdx).family, draw::Font::Type::Tablature);

    // font size in pixels, scaled according to spatium()
    // (use the same font selection as used in draw() below)
    double m = score()->styleD(Sid::figuredBassFontSize) * spatium() / SPATIUM20;
    f.setPointSizeF(m);
    mu::draw::FontMetrics fm(f);

    String str;
    x  = symWidth(SymId::noteheadBlack) * .5;
    x1 = x2 = 0.0;

    // create display text
    int font = 0;
    int style = score()->styleI(Sid::figuredBassStyle);

    if (parenth[0] != Parenthesis::NONE) {
        str.append(g_FBFonts.at(font).displayParenthesis[int(parenth[0])]);
    }

    // prefix
    if (_prefix != Modifier::NONE) {
        // if no digit, the string created so far 'hangs' to the left of the note
        if (_digit == FBIDigitNone) {
            x1 = fm.width(str);
        }
        str.append(g_FBFonts.at(font).displayAccidental[int(_prefix)]);
        // if no digit, the string from here onward 'hangs' to the right of the note
        if (_digit == FBIDigitNone) {
            x2 = fm.width(str);
        }
    }

    if (parenth[1] != Parenthesis::NONE) {
        str.append(g_FBFonts.at(font).displayParenthesis[int(parenth[1])]);
    }

    // digit
    if (_digit != FBIDigitNone) {
        // if some digit, the string created so far 'hangs' to the left of the note
        x1 = fm.width(str);
        // if suffix is a combining shape, combine it with digit (multi-digit numbers cannot be combined)
        // unless there is a parenthesis in between
        if ((_digit < 10)
            && (_suffix == Modifier::CROSS || _suffix == Modifier::BACKSLASH || _suffix == Modifier::SLASH)
            && parenth[2] == Parenthesis::NONE) {
            str.append(g_FBFonts.at(font).displayDigit[style][_digit][int(_suffix) - (int(Modifier::CROSS) - 1)]);
        }
        // if several digits or no shape combination, convert _digit to font styled chars
        else {
            String digits;
            int digit         = _digit;
            while (true) {
                digits.prepend(g_FBFonts.at(font).displayDigit[style][(digit % 10)][0]);
                digit /= 10;
                if (digit == 0) {
                    break;
                }
            }
            str.append(digits);
        }
        // if some digit, the string from here onward 'hangs' to the right of the note
        x2 = fm.width(str);
    }

    if (parenth[2] != Parenthesis::NONE) {
        str.append(g_FBFonts.at(font).displayParenthesis[int(parenth[2])]);
    }

    // suffix
    // append only if non-combining shape or cannot combine (no digit or parenthesis in between)
    if (_suffix != Modifier::NONE
        && ((_suffix != Modifier::CROSS && _suffix != Modifier::BACKSLASH && _suffix != Modifier::SLASH)
            || _digit == FBIDigitNone
            || parenth[2] != Parenthesis::NONE)) {
        str.append(g_FBFonts.at(font).displayAccidental[int(_suffix)]);
    }

    if (parenth[3] != Parenthesis::NONE) {
        str.append(g_FBFonts.at(font).displayParenthesis[int(parenth[3])]);
    }

    setDisplayText(str);                  // this text will be displayed

    if (str.size()) {                     // if some text
        x = x - (x1 + x2) * 0.5;          // position the text so that [x1<-->x2] is centered below the note
    } else {                              // if no text (but possibly a line)
        x = 0;                            // start at note left margin
    }
    // vertical position
    h = fm.lineSpacing();
    h *= score()->styleD(Sid::figuredBassLineHeight);
    if (score()->styleI(Sid::figuredBassAlignment) == 0) {          // top alignment: stack down from first item
        y = h * ord;
    } else {                                                      // bottom alignment: stack up from last item
        y = -h * (figuredBass()->numOfItems() - ord);
    }
    setPos(x, y);
    // determine bbox from text width
//      w = fm.width(str);
    w = fm.width(str);
    textWidth = w;
    // if there is a cont.line, extend width to cover the whole FB element duration line
    int lineLen;
    if (_contLine != ContLine::NONE && (lineLen=figuredBass()->lineLength(0)) > w) {
        w = lineLen;
    }
    bbox().setRect(0, 0, w, h);
}

//---------------------------------------------------------
//   FiguredBassItem draw()
//---------------------------------------------------------

void FiguredBassItem::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    int font = 0;
    double _spatium = spatium();
    // set font from general style
    mu::draw::Font f(g_FBFonts.at(font).family, draw::Font::Type::Tablature);

    // (use the same font selection as used in layout() above)
    double m = score()->styleD(Sid::figuredBassFontSize) * spatium() / SPATIUM20;
    f.setPointSizeF(m * MScore::pixelRatio);

    painter->setFont(f);
    painter->setBrush(BrushStyle::NoBrush);
    Pen pen(figuredBass()->curColor(), FB_CONTLINE_THICKNESS * _spatium, PenStyle::SolidLine, PenCapStyle::RoundCap);
    painter->setPen(pen);
    painter->drawText(bbox(), draw::TextDontClip | draw::AlignLeft | draw::AlignTop, displayText());

    // continuation line
    double lineEndX = 0.0;
    if (_contLine != ContLine::NONE) {
        double lineStartX  = textWidth;                           // by default, line starts right after text
        if (lineStartX > 0.0) {
            lineStartX += _spatium * FB_CONTLINE_LEFT_PADDING;          // if some text, give some room after it
        }
        lineEndX = figuredBass()->printedLineLength();            // by default, line ends with item duration
        if (lineEndX - lineStartX < 1.0) {                         // if line length < 1 sp, ignore it
            lineEndX = 0.0;
        }

        // if extended cont.line and no closing parenthesis: look at next FB element
        if (_contLine > ContLine::SIMPLE && parenth[4] == Parenthesis::NONE) {
            FiguredBass* nextFB;
            // if there is a contiguous FB element
            if ((nextFB=figuredBass()->nextFiguredBass()) != 0) {
                // retrieve the X position (in page coords) of a possible cont. line of nextFB
                // on the same line of 'this'
                PointF pgPos = pagePos();
                double nextContPageX = nextFB->additionalContLineX(pgPos.y());
                // if an additional cont. line has been found, extend up to its initial X coord
                if (nextContPageX > 0) {
                    lineEndX = nextContPageX - pgPos.x() + _spatium * FB_CONTLINE_OVERLAP;
                }
                // with a little bit of overlap
                else {
                    lineEndX = figuredBass()->lineLength(0);                  // if none found, draw to the duration end
                }
            }
        }
        // if some line, draw it
        if (lineEndX > 0.0) {
            double h = bbox().height() * FB_CONTLINE_HEIGHT;
            painter->drawLine(lineStartX, h, lineEndX - ipos().x(), h);
        }
    }

    // closing cont.line parenthesis
    if (parenth[4] != Parenthesis::NONE) {
        int x = lineEndX > 0.0 ? lineEndX : textWidth;
        painter->drawText(RectF(x, 0, bbox().width(), bbox().height()), draw::AlignLeft | draw::AlignTop,
                          Char(g_FBFonts.at(font).displayParenthesis[int(parenth[4])].unicode()));
    }
}

//---------------------------------------------------------
//   PROPERTY METHODS
//---------------------------------------------------------

PropertyValue FiguredBassItem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FBPREFIX:
        return int(_prefix);
    case Pid::FBDIGIT:
        return _digit;
    case Pid::FBSUFFIX:
        return int(_suffix);
    case Pid::FBCONTINUATIONLINE:
        return int(_contLine);
    case Pid::FBPARENTHESIS1:
        return int(parenth[0]);
    case Pid::FBPARENTHESIS2:
        return int(parenth[1]);
    case Pid::FBPARENTHESIS3:
        return int(parenth[2]);
    case Pid::FBPARENTHESIS4:
        return int(parenth[3]);
    case Pid::FBPARENTHESIS5:
        return int(parenth[4]);
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
        _prefix = (Modifier)val;
        break;
    case Pid::FBDIGIT:
        if (val < 1 || val > 9) {
            return false;
        }
        _digit = val;
        break;
    case Pid::FBSUFFIX:
        if (val < int(Modifier::NONE) || val >= int(Modifier::NUMOF)) {
            return false;
        }
        _suffix = (Modifier)val;
        break;
    case Pid::FBCONTINUATIONLINE:
        _contLine = (ContLine)val;
        break;
    case Pid::FBPARENTHESIS1:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        parenth[0] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS2:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        parenth[1] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS3:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        parenth[2] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS4:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        parenth[3] = (Parenthesis)val;
        break;
    case Pid::FBPARENTHESIS5:
        if (val < int(Parenthesis::NONE) || val >= int(Parenthesis::NUMOF)) {
            return false;
        }
        parenth[4] = (Parenthesis)val;
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayoutAll();
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

//---------------------------------------------------------
//   UNDOABLE PROPERTY SETTERS
//---------------------------------------------------------

void FiguredBassItem::undoSetPrefix(Modifier pref)
{
    if (pref <= Modifier::CROSS) {
        undoChangeProperty(Pid::FBPREFIX, (int)pref);
        // if setting some prefix and there is a suffix already, clear suffix
        if (pref != Modifier::NONE && _suffix != Modifier::NONE) {
            undoChangeProperty(Pid::FBSUFFIX, int(Modifier::NONE));
        }
        layout();                         // re-generate displayText
    }
}

void FiguredBassItem::undoSetDigit(int digit)
{
    if (digit >= 0 && digit <= 9) {
        undoChangeProperty(Pid::FBDIGIT, digit);
        layout();                         // re-generate displayText
    }
}

void FiguredBassItem::undoSetSuffix(Modifier suff)
{
    undoChangeProperty(Pid::FBSUFFIX, int(suff));
    // if setting some suffix and there is a prefix already, clear prefix
    if (suff != Modifier::NONE && _prefix != Modifier::NONE) {
        undoChangeProperty(Pid::FBPREFIX, int(Modifier::NONE));
    }
    layout();                       // re-generate displayText
}

void FiguredBassItem::undoSetContLine(ContLine val)
{
    undoChangeProperty(Pid::FBCONTINUATIONLINE, int(val));
    layout();                       // re-generate displayText
}

void FiguredBassItem::undoSetParenth1(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS1, int(par));
    layout();                       // re-generate displayText
}

void FiguredBassItem::undoSetParenth2(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS2, int(par));
    layout();                       // re-generate displayText
}

void FiguredBassItem::undoSetParenth3(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS3, int(par));
    layout();                       // re-generate displayText
}

void FiguredBassItem::undoSetParenth4(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS4, int(par));
    layout();                       // re-generate displayText
}

void FiguredBassItem::undoSetParenth5(Parenthesis par)
{
    undoChangeProperty(Pid::FBPARENTHESIS5, int(par));
    layout();                       // re-generate displayText
}

//---------------------------------------------------------
//   startsWithParenthesis
//---------------------------------------------------------

bool FiguredBassItem::startsWithParenthesis() const
{
    if (_prefix != Modifier::NONE) {
        return parenth[0] != Parenthesis::NONE;
    }
    if (_digit != FBIDigitNone) {
        return parenth[1] != Parenthesis::NONE;
    }
    if (_suffix != Modifier::NONE) {
        return parenth[2] != Parenthesis::NONE;
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
    DeleteAll(m_items);
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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FiguredBass::layout()
{
    // VERTICAL POSITION:
    const double y = score()->styleD(Sid::figuredBassYOffset) * spatium();
    setPos(PointF(0.0, y));

    // BOUNDING BOX and individual item layout (if required)
    TextBase::layout1();   // prepare structs and data expected by Text methods
    // if element could be parsed into items, layout each element
    // Items list will be empty in edit mode (see FiguredBass::startEdit).
    // TODO: consider disabling specific layout in case text style is changed (tid() != TextStyleName::FIGURED_BASS).
    if (m_items.size() > 0) {
        layoutLines();
        bbox().setRect(0, 0, _lineLengths.at(0), 0);
        // layout each item and enlarge bbox to include items bboxes
        for (FiguredBassItem* item : m_items) {
            item->layout();
            addbbox(item->bbox().translated(item->pos()));
        }
    }
}

//---------------------------------------------------------
//   layoutLines
//
//    lays out the duration indicator line(s), filling the _lineLengths array
//    and the length of printed lines (used by continuation lines)
//---------------------------------------------------------

void FiguredBass::layoutLines()
{
    if (_ticks <= Fraction(0, 1) || !segment()) {
        _lineLengths.resize(1);                             // be sure to always have
        _lineLengths[0] = 0;                                // at least 1 item in array
        return;
    }

    ChordRest* lastCR  = nullptr;                         // the last ChordRest of this
    Segment* nextSegm = nullptr;                          // the Segment beyond this' segment
    Fraction nextTick = segment()->tick() + _ticks;       // the tick beyond this' duration

    // locate the measure containing the last tick of this; it is either:
    // the same measure containing nextTick, if nextTick is not the first tick of a measure
    //    (and line should stop right before it)
    // or the previous measure, if nextTick is the first tick of a measure
    //    (and line should stop before any measure terminal segment (bar, clef, ...) )

    Measure* m = score()->tick2measure(nextTick - Fraction::fromTicks(1));
    if (m) {
        // locate the first segment (of ANY type) right after this' last tick
        for (nextSegm = m->first(SegmentType::All); nextSegm; nextSegm = nextSegm->next()) {
            if (nextSegm->tick() >= nextTick) {
                break;
            }
        }
        // locate the last ChordRest of this
        if (nextSegm) {
            track_idx_t startTrack = trackZeroVoice(track());
            track_idx_t endTrack = startTrack + VOICES;
            for (const Segment* seg = nextSegm->prev1(); seg; seg = seg->prev1()) {
                for (track_idx_t t = startTrack; t < endTrack; ++t) {
                    EngravingItem* el = seg->element(t);
                    if (el && el->isChordRest()) {
                        lastCR = toChordRest(el);
                        break;
                    }
                }
                if (lastCR) {
                    break;
                }
            }
        }
    }
    if (!m || !nextSegm) {
        LOGD("FiguredBass layout: no segment found for tick %d", nextTick.ticks());
        _lineLengths.resize(1);                             // be sure to always have
        _lineLengths[0] = 0;                                // at least 1 item in array
        return;
    }

    // get length of printed lines from horiz. page position of lastCR
    // (enter a bit 'into' the ChordRest for clarity)
    _printedLineLength = lastCR ? lastCR->pageX() - pageX() + 1.5 * spatium() : 3 * spatium();

    // get duration indicator line(s) from page position of nextSegm
    const std::vector<System*>& systems = score()->systems();
    System* s1  = segment()->measure()->system();
    System* s2  = nextSegm->measure()->system();
    system_idx_t sysIdx1 = mu::indexOf(systems, s1);
    system_idx_t sysIdx2 = mu::indexOf(systems, s2);

    if (sysIdx2 == mu::nidx || sysIdx2 < sysIdx1) {
        sysIdx2 = sysIdx1;
        nextSegm = segment()->next1();
        // TODO
        // During layout of figured bass next systems' numbers may be still
        // undefined (then sysIdx2 == mu::nidx) or change in the future.
        // A layoutSystem() approach similar to that for spanners should
        // probably be implemented.
    }

    system_idx_t i;
    int len;
    size_t segIdx = 0;
    for (i = sysIdx1, segIdx = 0; i <= sysIdx2; ++i, ++segIdx) {
        len = 0;
        if (sysIdx1 == sysIdx2 || i == sysIdx1) {
            // single line
            len = nextSegm->pageX() - pageX() - 4;               // stop 4 raster units before next segm
        } else if (i == sysIdx1) {
            // initial line
            double w   = s1->staff(staffIdx())->bbox().right();
            double x   = s1->pageX() + w;
            len = x - pageX();
        } else if (i > 0 && i != sysIdx2) {
            // middle line
            LOGD("FiguredBass: duration indicator middle line not implemented");
        } else if (i == sysIdx2) {
            // end line
            LOGD("FiguredBass: duration indicator end line not implemented");
        }
        // store length item, reusing array items if already present
        if (_lineLengths.size() <= segIdx) {
            _lineLengths.push_back(len);
        } else {
            _lineLengths[segIdx] = len;
        }
    }
    // if more array items than needed, truncate array
    if (_lineLengths.size() > segIdx) {
        _lineLengths.resize(segIdx);
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FiguredBass::draw(mu::draw::Painter* painter) const
{
    using namespace mu::draw;
    // if not printing, draw duration line(s)
    if (!score()->printing() && score()->showUnprintable()) {
        for (double len : _lineLengths) {
            if (len > 0) {
                painter->setPen(Pen(engravingConfiguration()->formattingMarksColor(), 3));
                painter->drawLine(0.0, -2, len, -2);              // -2: 2 rast. un. above digits
            }
        }
    }
    // if in edit mode or with custom style, use standard text drawing
//      if (editMode() || subStyle() != ElementStyle::FIGURED_BASS)
//      if (tid() != TextStyleName::FIGURED_BASS)
//            TextBase::draw(painter);
//      else
    {                                                        // not edit mode:
        if (m_items.size() < 1) {                             // if not parseable into f.b. items
            TextBase::draw(painter);                            // draw as standard text
        } else {
            for (FiguredBassItem* item : m_items) {           // if parseable into f.b. items
                painter->translate(item->pos());            // draw each item in its proper position
                item->draw(painter);
                painter->translate(-item->pos());
            }
        }
    }
}

//---------------------------------------------------------
//   startEdit / edit / endEdit
//---------------------------------------------------------

void FiguredBass::startEdit(EditData& ed)
{
    DeleteAll(m_items);
    m_items.clear();
    layout1();   // re-layout without F.B.-specific formatting.
    TextBase::startEdit(ed);
}

bool FiguredBass::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

void FiguredBass::endEdit(EditData& ed)
{
    TextBase::endEdit(ed);
    // as the standard text editor keeps inserting spurious HTML formatting and styles
    // retrieve and work only on the plain text
    const String txt = plainText();
    if (txt.isEmpty()) { // if no text, nothing to do
        return;
    }

    // split text into lines and create an item for each line
    StringList list = txt.split(u'\n', mu::SkipEmptyParts);
    DeleteAll(m_items);
    m_items.clear();
    String normalizedText;
    int idx = 0;
    for (String str : list) {
        FiguredBassItem* pItem = new FiguredBassItem(this, idx++);
        if (!pItem->parse(str)) {               // if any item fails parsing
            DeleteAll(m_items);
            m_items.clear();                      // clear item list
            score()->startCmd();
            triggerLayout();
            score()->endCmd();
            delete pItem;
            return;
        }
        pItem->setTrack(track());
        pItem->setParent(this);
        m_items.push_back(pItem);

        // add item normalized text
        if (!normalizedText.isEmpty()) {
            normalizedText.append('\n');
        }
        normalizedText.append(pItem->normalizedText());
    }
    // if all items parsed and text is styled, replaced entered text with normalized text
    if (m_items.size()) {
        setXmlText(normalizedText);
    }

    score()->startCmd();
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
    if (_ticks <= Fraction(0, 1)) {                                      // if _ticks unset, no clear idea of when 'this' ends
        return 0;
    }
    Segment* nextSegm;                                   // the Segment beyond this' segment
    Fraction nextTick = segment()->tick() + _ticks;      // the tick beyond this' duration

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
            && std::abs(pgPos.y() + fbi->ipos().y() - pagePosY) < 0.05) {
            return pgPos.x() + fbi->ipos().x();
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
    return TextBase::setProperty(propertyId, v);
}

PropertyValue FiguredBass::propertyDefault(Pid id) const
{
    return TextBase::propertyDefault(id);
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
        for (prevSegm = seg->prev1(SegmentType::ChordRest); prevSegm; prevSegm = prevSegm->prev1(SegmentType::ChordRest)) {
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
    bool bNew;
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
