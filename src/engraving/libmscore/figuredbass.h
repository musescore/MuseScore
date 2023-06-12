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

    void draw(mu::draw::Painter* painter) const override;

    bool startsWithParenthesis() const;

    // specific API
    const FiguredBass* figuredBass() const { return (FiguredBass*)(explicitParent()); }
    bool parse(String& text);

    // getters / setters
    int ord() const { return m_ord; }

    Modifier prefix() const { return m_prefix; }
    void setPrefix(const Modifier& v) { m_prefix = v; }
    void undoSetPrefix(Modifier pref);

    int digit() const { return m_digit; }
    void setDigit(int val) { m_digit = val; }
    void undoSetDigit(int digit);

    Modifier suffix() const { return m_suffix; }
    void setSuffix(const Modifier& v) { m_suffix = v; }
    void undoSetSuffix(Modifier suff);

    ContLine contLine() const { return m_contLine; }
    void setContLine(const ContLine& v) { m_contLine = v; }
    void undoSetContLine(ContLine val);

    double textWidth() const { return m_textWidth; }
    void setTextWidth(double w) { m_textWidth = w; }

    Parenthesis parenth1() const { return m_parenth[0]; }
    Parenthesis parenth2() const { return m_parenth[1]; }
    Parenthesis parenth3() const { return m_parenth[2]; }
    Parenthesis parenth4() const { return m_parenth[3]; }
    Parenthesis parenth5() const { return m_parenth[4]; }

    void setParenth1(Parenthesis v) { m_parenth[0] = v; }
    void setParenth2(Parenthesis v) { m_parenth[1] = v; }
    void setParenth3(Parenthesis v) { m_parenth[2] = v; }
    void setParenth4(Parenthesis v) { m_parenth[3] = v; }
    void setParenth5(Parenthesis v) { m_parenth[4] = v; }

    void undoSetParenth1(Parenthesis par);
    void undoSetParenth2(Parenthesis par);
    void undoSetParenth3(Parenthesis par);
    void undoSetParenth4(Parenthesis par);
    void undoSetParenth5(Parenthesis par);
    String normalizedText() const;

    String displayText() const { return m_displayText; }
    void setDisplayText(const String& s) { m_displayText = s; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

private:

    friend class FiguredBass;

    FiguredBassItem(FiguredBass* parent = 0, int line = 0);
    FiguredBassItem(const FiguredBassItem&);

    // part parsing
    int parseDigit(String& str);
    int parseParenthesis(String& str, int parenthIdx);
    int parsePrefixSuffix(String& str, bool bPrefix);

    void regenerateDisplayText();

    static const Char NORM_PARENTH_TO_CHAR[int(Parenthesis::NUMOF)];

    String m_displayText;                       // the constructed display text (read-only)
    int m_ord = 0;                              // the line ordinal of this element in the FB stack
                                                // the parts making a FiguredBassItem up
    Modifier m_prefix = Modifier::NONE;         // the accidental coming before the body
    int m_digit = 0;                            // the main digit (if present)
    Modifier m_suffix = Modifier::NONE;         // the accidental coming after the body
    ContLine m_contLine = ContLine::NONE;       // whether the item has continuation line or not
    Parenthesis m_parenth[5];                   // each of the parenthesis: before, between and after parts
    double m_textWidth = 0.0;                   // the text width (in raster units), set during layout()
                                                //    used by draw()
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

public:

    ~FiguredBass();

    // a convenience static function to create/retrieve a new FiguredBass into/from its intended parent
    static FiguredBass* addFiguredBassToSegment(Segment* seg, track_idx_t track, const Fraction& extTicks, bool* pNew);

    // static functions for font config files
    static bool readConfigFile(const String& fileName);
    static std::list<String> fontNames();
    static bool fontData(int nIdx, String* pFamily, String* pDisplayName, double* pSize, double* pLineHeight);

    // standard re-implemented virtual functions
    FiguredBass* clone() const override { return new FiguredBass(*this); }

    FiguredBassItem* createItem(int line) { return new FiguredBassItem(this, line); }

    void draw(mu::draw::Painter* painter) const override;

    void setSelected(bool f) override;
    void setVisible(bool f) override;
    void startEdit(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    void endEdit(EditData&) override;

    void setLineLengths(const std::vector<double>& ll) { m_lineLengths = ll; }
    const std::vector<double>& lineLengths() const { return m_lineLengths; }
    double lineLength(size_t idx) const
    {
        if (idx < m_lineLengths.size()) {
            return m_lineLengths.at(idx);
        }
        return 0;
    }

    void setPrintedLineLength(double l) { m_printedLineLength = l; }
    double printedLineLength() const { return m_printedLineLength; }

    bool onNote() const { return m_onNote; }
    void setOnNote(bool val) { m_onNote = val; }
    Segment* segment() const { return (Segment*)(explicitParent()); }
    const Fraction& ticks() const { return m_ticks; }
    void setTicks(const Fraction& v) { m_ticks = v; }

    double additionalContLineX(double pagePosY) const;  // returns the X coord (in page coord) of cont. line at pagePosY, if any
    FiguredBass* nextFiguredBass() const;                         // returns next *adjacent* f.b. item, if any

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    size_t itemsCount() const { return m_items.size(); }
    void appendItem(FiguredBassItem* item) { m_items.push_back(item); }
    const std::vector<FiguredBassItem*>& items() const { return m_items; }

    // the array of configured fonts
    static const std::vector<FiguredBassFont>& FBFonts();

    bool hasParentheses() const;       // read / write MusicXML support

private:

    friend class Factory;
    FiguredBass(Segment* parent = 0);
    FiguredBass(const FiguredBass&);

    Sid getPropertyStyle(Pid) const override;

    std::vector<FiguredBassItem*> m_items;       // the individual lines of the F.B.
    std::vector<double> m_lineLengths;           // lengths of duration indicator lines (in raster units)
    bool m_onNote = true;                        // true if this element is on a staff note | false if it is between notes
    Fraction m_ticks;                            // the duration (used for cont. lines and for multiple F.B.
                                                 // under the same note)
    double m_printedLineLength = 0.0;            // the length of lines actually printed (i.e. continuation lines)
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::FiguredBassItem::Modifier);
Q_DECLARE_METATYPE(mu::engraving::FiguredBassItem::Parenthesis);
Q_DECLARE_METATYPE(mu::engraving::FiguredBassItem::ContLine);
#endif

#endif
