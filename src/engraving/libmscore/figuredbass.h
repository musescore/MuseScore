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

#ifndef __FIGUREDBASS_H__
#define __FIGUREDBASS_H__

#include "textbase.h"

namespace mu::engraving {
class Factory;
class Segment;

/*---------------------------------------------------------
NOTE ON ARCHITECTURE

FiguredBass elements are stored in the annotations of a Segment (like for instance Harmony)

FiguredBass is rather simple: it contains only _ticks, telling the duration of the element,
and a list of FiguredBassItem elements which do most of the job. It also maintains a text with the
normalized (made uniform) version of the text, which is used during editing.

Normally, a FiguredBass element is assumed to be styled with an internally maintained text style
(based on the parameters of the general style "Figured Bass") FIGURED_BASS style and it is set
in this way upon creation and upon layout().
- - - -
FiguredBassItem contains the actually f.b. info; it is made of 4 parts (in this order):
1) prefix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp, cross]
2) digit: one digit from 1 to 9
3) suffix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp, cross, backslash, slash]
4) contLine: true if the item has a continuation line (whose length is determined by parent's _ticks)
and 5 parenthesis flags, one for each position before, between and after the four parts above:
each of them may contain one of [nothing, roundOpen, roundClosed, squaredOpen, squaredClosed].

There is a number of restrictions, implemented at the end of FiguredBassItem::parse().
Currently, no attempt is made to ensure that, if multiple parentheses are present, they are consistent
(matching open and closed parentheses is left to the user).

If an item cannot be parsed, the whole FiguredBass element is kept as entered, possibly un-styled.
If all items can be parsed, each item generates a display text from its properties,
lays it out so that it properly aligns under the chord, draws it at its proper location
and provides its FiguredBass parent with a normalized text for future editing.

FiguredBassItem has not use for formats (italics, bold, ...) and it is never edited directly;
more generally, it is never accessed directly, only via its FiguredBass parent;
so it is directly derived from EngravingItem and returns INVALID as type.

FiguredBass might require formatting (discouraged, but might be necessary for very uncommon cases)
and it is edited (via the normalized text); so it is derived from Text.
---------------------------------------------------------*/

constexpr int FBIDigitNone = -1;

//---------------------------------------------------------
//   @@ FiguredBassItem
///   One line of a figured bass indication
//
//   @P continuationLine   enum (FiguredBassItem.NONE, .SIMPLE, .EXTENDED)  whether item has continuation line or not, and of which type
//   @P digit              int                              main digit(s) (0 - 9)
//   @P displayText        string                           text displayed (depends on configured fonts) (read only)
//   @P normalizedText     string                           conventional textual representation of item properties (= text used during input) (read only)
//   @P parenthesis1       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis before the prefix
//   @P parenthesis2       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the prefix / before the digit
//   @P parenthesis3       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the digit / before the suffix
//   @P parenthesis4       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the suffix / before the cont. line
//   @P parenthesis5       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the cont. line
//   @P prefix             enum (FiguredBassItem.NONE, .DOUBLEFLAT, .FLAT, .NATURAL, .SHARP, .DOUBLESHARP, .PLUS, .BACKSLASH, .SLASH)  accidental before the digit
//   @P suffix             enum (FiguredBassItem.NONE, .DOUBLEFLAT, .FLAT, .NATURAL, .SHARP, .DOUBLESHARP, .PLUS, .BACKSLASH, .SLASH)  accidental/diacritic after the digit
//---------------------------------------------------------

class FiguredBass;

class FiguredBassItem final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, FiguredBassItem)

public:
    enum class Modifier : char {
        NONE = 0,
        DOUBLEFLAT,
        FLAT,
        NATURAL,
        SHARP,
        DOUBLESHARP,
        CROSS,
        BACKSLASH,
        SLASH,
        NUMOF
    };
    enum class Parenthesis : char {
        NONE = 0,
        ROUNDOPEN,
        ROUNDCLOSED,
        SQUAREDOPEN,
        SQUAREDCLOSED,
        NUMOF
    };
    enum class ContLine : char {
        NONE = 0,
        SIMPLE,                         // cont. line stops at f.b. element end
        EXTENDED                        // cont. line joins with next element, if possible
    };

    enum class Style : char {
        MODERN = 0,
        HISTORIC,
        NUMOF
    };
    enum class Combination : char {
        SIMPLE = 0,
        CROSSED,
        BACKSLASHED,
        SLASHED,
        NUMOF
    };

    ~FiguredBassItem();

    FiguredBassItem& operator=(const FiguredBassItem&) = delete;

    // standard re-implemented virtual functions
    FiguredBassItem* clone() const override { return new FiguredBassItem(*this); }

    void              draw(mu::draw::Painter* painter) const override;
    void              layout() override;

    bool              startsWithParenthesis() const;

    // specific API
    const FiguredBass* figuredBass() const { return (FiguredBass*)(explicitParent()); }
    bool              parse(String& text);

    // getters / setters
    Modifier          prefix() const { return _prefix; }
    void              setPrefix(const Modifier& v) { _prefix = v; }
    void              undoSetPrefix(Modifier pref);
    int               digit() const { return _digit; }
    void              setDigit(int val) { _digit = val; }
    void              undoSetDigit(int digit);
    Modifier          suffix() const { return _suffix; }
    void              setSuffix(const Modifier& v) { _suffix = v; }
    void              undoSetSuffix(Modifier suff);
    ContLine          contLine() const { return _contLine; }
    void              setContLine(const ContLine& v) { _contLine = v; }
    void              undoSetContLine(ContLine val);
    Parenthesis       parenth1() const { return m_parenth[0]; }
    Parenthesis       parenth2() const { return m_parenth[1]; }
    Parenthesis       parenth3() const { return m_parenth[2]; }
    Parenthesis       parenth4() const { return m_parenth[3]; }
    Parenthesis       parenth5() const { return m_parenth[4]; }

    void              setParenth1(Parenthesis v) { m_parenth[0] = v; }
    void              setParenth2(Parenthesis v) { m_parenth[1] = v; }
    void              setParenth3(Parenthesis v) { m_parenth[2] = v; }
    void              setParenth4(Parenthesis v) { m_parenth[3] = v; }
    void              setParenth5(Parenthesis v) { m_parenth[4] = v; }

    void              undoSetParenth1(Parenthesis par);
    void              undoSetParenth2(Parenthesis par);
    void              undoSetParenth3(Parenthesis par);
    void              undoSetParenth4(Parenthesis par);
    void              undoSetParenth5(Parenthesis par);
    String            normalizedText() const;
    String            displayText() const { return _displayText; }

    PropertyValue  getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue  propertyDefault(Pid) const override;

private:

    friend class FiguredBass;
    friend class v0::TLayout;

    static const Char normParenthToChar[int(Parenthesis::NUMOF)];

    String _displayText;                        // the constructed display text (read-only)
    int m_ord;                                  // the line ordinal of this element in the FB stack
    // the parts making a FiguredBassItem up
    Modifier _prefix;                           // the accidental coming before the body
    int _digit;                                 // the main digit (if present)
    Modifier _suffix;                           // the accidental coming after the body
    ContLine _contLine;                         // whether the item has continuation line or not
    Parenthesis m_parenth[5];                   // each of the parenthesis: before, between and after parts
    double m_textWidth;                         // the text width (in raster units), set during layout()
                                                //    used by draw()

    FiguredBassItem(FiguredBass* parent = 0, int line = 0);
    FiguredBassItem(const FiguredBassItem&);

    // part parsing
    int               parseDigit(String& str);
    int               parseParenthesis(String& str, int parenthIdx);
    int               parsePrefixSuffix(String& str, bool bPrefix);

    void              setDisplayText(const String& s) { _displayText = s; }
};

//---------------------------------------------------------
//   FiguredBassFont
//---------------------------------------------------------

struct FiguredBassFont {
    String family;
    String displayName;
    double defPitch;
    double defLineHeight;
    Char displayAccidental[int(FiguredBassItem::Modifier::NUMOF)];
    Char displayParenthesis[int(FiguredBassItem::Parenthesis::NUMOF)];
    Char displayDigit[int(FiguredBassItem::Style::NUMOF)][10][int(FiguredBassItem::Combination::NUMOF)];

    bool read(XmlReader&);
};

// the array of configured fonts
static std::vector<FiguredBassFont> g_FBFonts;

//---------------------------------------------------------
//   @@ FiguredBass
///    A complete figured bass indication
//
//   @P onNote  bool  whether it is placed on a note beginning or between notes (read only)
//   @P ticks   int   duration in ticks
//---------------------------------------------------------

class FiguredBass final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, FiguredBass)
    DECLARE_CLASSOF(ElementType::FIGURED_BASS)

    std::vector<FiguredBassItem*> m_items;        // the individual lines of the F.B.
    std::vector<double> _lineLengths;                // lengths of duration indicator lines (in raster units)
    bool _onNote;                               // true if this element is on a staff note | false if it is between notes
    Fraction _ticks;                            // the duration (used for cont. lines and for multiple F.B.
                                                // under the same note)
    double _printedLineLength;                   // the length of lines actually printed (i.e. continuation lines)

    friend class Factory;
    FiguredBass(Segment* parent = 0);
    FiguredBass(const FiguredBass&);

    void              layoutLines();
    bool              hasParentheses() const;   // read / write MusicXML support

    Sid getPropertyStyle(Pid) const override;

public:

    ~FiguredBass();

    // a convenience static function to create/retrieve a new FiguredBass into/from its intended parent
    static FiguredBass* addFiguredBassToSegment(Segment* seg, track_idx_t track, const Fraction& extTicks, bool* pNew);

    // static functions for font config files
    static bool       readConfigFile(const String& fileName);
    static std::list<String> fontNames();
    static bool       fontData(int nIdx, String* pFamily, String* pDisplayName, double* pSize, double* pLineHeight);

    // standard re-implemented virtual functions
    FiguredBass* clone() const override { return new FiguredBass(*this); }

    FiguredBassItem* createItem(int line) { return new FiguredBassItem(this, line); }

    void draw(mu::draw::Painter* painter) const override;
    void layout() override;
    void setSelected(bool f) override;
    void setVisible(bool f) override;
    void startEdit(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    void endEdit(EditData&) override;

    double lineLength(size_t idx) const
    {
        if (idx < _lineLengths.size()) {
            return _lineLengths.at(idx);
        }
        return 0;
    }

    double             printedLineLength() const { return _printedLineLength; }
    bool              onNote() const { return _onNote; }
    size_t            numOfItems() const { return m_items.size(); }
    void              setOnNote(bool val) { _onNote = val; }
    Segment* segment() const { return (Segment*)(explicitParent()); }
    Fraction          ticks() const { return _ticks; }
    void              setTicks(const Fraction& v) { _ticks = v; }

    double             additionalContLineX(double pagePosY) const;  // returns the X coord (in page coord) of cont. line at pagePosY, if any
    FiguredBass* nextFiguredBass() const;                         // returns next *adjacent* f.b. item, if any

    PropertyValue  getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue  propertyDefault(Pid) const override;

    void appendItem(FiguredBassItem* item) { m_items.push_back(item); }

    const std::vector<FiguredBassItem*>& items() const { return m_items; }
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::FiguredBassItem::Modifier);
Q_DECLARE_METATYPE(mu::engraving::FiguredBassItem::Parenthesis);
Q_DECLARE_METATYPE(mu::engraving::FiguredBassItem::ContLine);
#endif

#endif
