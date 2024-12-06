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

#ifndef MU_ENGRAVING_TEXTBASE_H
#define MU_ENGRAVING_TEXTBASE_H

#include <variant>

#include "draw/fontmetrics.h"

#include "modularity/ioc.h"
#include "../iengravingfontsprovider.h"

#include "engravingitem.h"
#include "property.h"
#include "types.h"

namespace mu::engraving {
class TextBase;
class TextBlock;

//---------------------------------------------------------
//   FrameType
//---------------------------------------------------------

enum class FrameType : char {
    NO_FRAME, SQUARE, CIRCLE
};

//---------------------------------------------------------
//   VerticalAlignment
//---------------------------------------------------------

enum class VerticalAlignment : signed char {
    AlignUndefined = -1, AlignNormal, AlignSuperScript, AlignSubScript
};

//---------------------------------------------------------
//   FormatId
//---------------------------------------------------------

enum class FormatId : char {
    Bold, Italic, Underline, Strike, Valign, FontSize, FontFamily
};

using FormatValue = std::variant<std::monostate, bool, int, double, String>;

//---------------------------------------------------------
//   MultiClick
//---------------------------------------------------------

enum class SelectTextType : char {
    Word, All
};

//---------------------------------------------------------
//   CharFormat
//---------------------------------------------------------

class CharFormat
{
public:
    CharFormat() {}
    CharFormat(const CharFormat& cf) { *this = cf; }
    bool operator==(const CharFormat& cf) const;
    CharFormat& operator=(const CharFormat& cf);

    FontStyle style() const { return m_style; }
    void setStyle(FontStyle s) { m_style = s; }
    bool bold() const { return m_style & FontStyle::Bold; }
    bool italic() const { return m_style & FontStyle::Italic; }
    bool underline() const { return m_style & FontStyle::Underline; }
    bool strike() const { return m_style & FontStyle::Strike; }
    void setBold(bool val) { m_style = val ? m_style + FontStyle::Bold : m_style - FontStyle::Bold; }
    void setItalic(bool val) { m_style = val ? m_style + FontStyle::Italic : m_style - FontStyle::Italic; }
    void setUnderline(bool val) { m_style = val ? m_style + FontStyle::Underline : m_style - FontStyle::Underline; }
    void setStrike(bool val) { m_style = val ? m_style + FontStyle::Strike : m_style - FontStyle::Strike; }

    VerticalAlignment valign() const { return m_valign; }
    double fontSize() const { return m_fontSize; }
    String fontFamily() const { return m_fontFamily; }
    void setValign(VerticalAlignment val) { m_valign = val; }
    void setFontSize(double val) { m_fontSize = val; }
    void setFontFamily(const String& val) { m_fontFamily = val; }

    FormatValue formatValue(FormatId) const;
    void setFormatValue(FormatId, const FormatValue& val);

private:

    FontStyle m_style = FontStyle::Normal;
    VerticalAlignment m_valign = VerticalAlignment::AlignNormal;
    double m_fontSize = 12.0;
    String m_fontFamily;
};

//---------------------------------------------------------
//   TextCursor
//    Contains current position and start of selection
//    during editing.
//---------------------------------------------------------

class TextCursor
{
public:
    enum class MoveOperation {
        Start,
        Up,
        StartOfLine,
        Left,
        WordLeft,
        End,
        Down,
        EndOfLine,
        NextWord,
        Right
    };

    enum class MoveMode {
        MoveAnchor,
        KeepAnchor
    };

    TextCursor(TextBase* t)
        : m_text(t) {}

    TextBase* text() const { return m_text; }
    bool hasSelection() const { return (m_selectLine != m_row) || (m_selectColumn != m_column); }
    void clearSelection();
    void endEdit();
    void startEdit();
    bool editing() const { return m_editing; }

    CharFormat* format() { return &m_format; }
    const CharFormat* format() const { return &m_format; }
    void setFormat(const CharFormat& f) { m_format = f; }

    size_t row() const { return m_row; }
    size_t column() const { return m_column; }
    size_t selectLine() const { return m_selectLine; }
    size_t selectColumn() const { return m_selectColumn; }
    void setRow(size_t val) { m_row = val; }
    void setColumn(size_t val) { m_column = val; }
    void setSelectLine(size_t val) { m_selectLine = val; }
    void setSelectColumn(size_t val) { m_selectColumn = val; }
    size_t columns() const;
    void init();

    struct Range {
        int startPosition = 0;
        int endPosition = 0;
        String text;
    };

    std::pair<size_t, size_t> positionToLocalCoord(int position) const;

    int currentPosition() const;
    Range selectionRange() const;

    const TextBlock& curLine() const;
    TextBlock& curLine();

    RectF cursorRect() const;
    bool movePosition(TextCursor::MoveOperation op, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor, int count = 1);
    void selectWord();
    void moveCursorToEnd() { movePosition(TextCursor::MoveOperation::End); }
    void moveCursorToStart() { movePosition(TextCursor::MoveOperation::Start); }
    Char currentCharacter() const;
    bool set(const PointF& p, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor);
    String selectedText(bool withFormat = false) const;
    String extractText(int r1, int c1, int r2, int c2, bool withFormat = false) const;
    void updateCursorFormat();
    void setFormat(FormatId, FormatValue val);
    void changeSelectionFormat(FormatId id, const FormatValue& val);
    const CharFormat selectedFragmentsFormat() const;

private:
    Range range(int start, int end) const;
    int position(int row, int column) const;

    TextBase* m_text = nullptr;
    CharFormat m_format;
    size_t m_row = 0;
    size_t m_column = 0;
    size_t m_selectLine = 0;           // start of selection
    size_t m_selectColumn = 0;
    bool m_editing = false;
};

//---------------------------------------------------------
//   TextFragment
//    contains a styled text
//---------------------------------------------------------

class TextFragment
{
public:
    muse::GlobalInject<IEngravingFontsProvider> engravingFonts;

public:
    mutable CharFormat format;
    PointF pos;                    // y is relative to TextBlock->y()
    mutable String text;

    TextFragment() = default;
    TextFragment(const String& s);
    TextFragment(TextCursor*, const String&);
    TextFragment(const TextFragment& f);

    TextFragment& operator =(const TextFragment& f);

    bool operator ==(const TextFragment& f) const;

    TextFragment split(int column);
    void draw(muse::draw::Painter*, const TextBase*) const;
    muse::draw::Font font(const TextBase*) const;
    int columns() const;
    void changeFormat(FormatId id, const FormatValue& data);
};

//---------------------------------------------------------
//   TextBlock
//    represents a block of formatted text
//---------------------------------------------------------

class TextBlock
{
public:
    TextBlock() = default;

    bool operator ==(const TextBlock& x) const { return m_fragments == x.m_fragments; }
    bool operator !=(const TextBlock& x) const { return m_fragments != x.m_fragments; }
    void draw(muse::draw::Painter*, const TextBase*) const;
    void layout(const TextBase*);
    const std::list<TextFragment>& fragments() const { return m_fragments; }
    std::list<TextFragment>& fragments() { return m_fragments; }
    std::list<TextFragment> fragmentsWithoutEmpty();
    const Shape& shape() const { return m_shape; }
    const RectF& boundingRect() const { return m_shape.bbox(); }
    RectF boundingRect(int col1, int col2, const TextBase*) const;
    size_t columns() const;
    void insert(TextCursor*, const String&);
    void insertEmptyFragmentIfNeeded(TextCursor*);
    void removeEmptyFragment();
    String remove(int column, TextCursor*);
    String remove(int start, int n, TextCursor*);
    int column(double x, TextBase*) const;
    TextBlock split(int column, TextCursor* cursor);
    double xpos(size_t col, const TextBase*) const;
    const CharFormat* formatAt(int) const;
    const TextFragment* fragment(int col) const;
    std::list<TextFragment>::iterator fragment(int column, int* rcol, int* ridx);
    double y() const { return m_y; }
    void setY(double val) { m_y = val; }
    double lineSpacing() const { return m_lineSpacing; }
    String text(int, int, bool = false) const;
    bool eol() const { return m_eol; }
    void setEol(bool val) { m_eol = val; }
    void changeFormat(FormatId, const FormatValue& val, int start, int n);

private:
    void simplify();

    std::list<TextFragment> m_fragments;
    double m_y = 0.0;
    double m_lineSpacing = 0.0;
    Shape m_shape;
    bool m_eol = false;
};

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TextBase)

    M_PROPERTY2(bool, isTextLinkedToMaster, setTextLinkedToMaster, true)

public:

    ~TextBase();

    virtual bool mousePress(EditData&) override;

    Text& operator=(const Text&) = delete;

    virtual void drawEditMode(muse::draw::Painter* p, EditData& ed, double currentViewScaling) override;
    static void drawTextWorkaround(muse::draw::Painter* p, muse::draw::Font& f, const PointF& pos, const String& text);

    Align align() const { return m_align; }
    void setAlign(Align a) { m_align = a; }

    static String plainToXmlText(const String& s) { return s.toXmlEscaped(); }
    void setPlainText(const String& t) { setXmlText(plainToXmlText(t)); }
    virtual void setXmlText(const String&);
    void setXmlText(const char* str) { setXmlText(String::fromUtf8(str)); }
    void checkCustomFormatting(const String&);
    String xmlText() const;
    String plainText() const;
    void resetFormatting();

    void insertText(EditData&, const String&);

    double lineSpacing() const;
    double lineHeight() const;
    virtual double baseLine() const override;

    bool empty() const { return xmlText().isEmpty(); }
    void clear() { setXmlText(String()); }

    FontStyle fontStyle() const;
    String family() const;
    double size() const;

    void setFontStyle(const FontStyle& val);
    void setFamily(const String& val);
    void setSize(const double& val);

    bool anchorToEndOfPrevious() const { return m_anchorToEndOfPrevious; }
    void setAnchorToEndOfPrevious(bool v) { m_anchorToEndOfPrevious = v; }

    bool hasParentSegment() const { return explicitParent() && parent()->isSegment(); }
    virtual bool needStartEditingAfterSelecting() const override { return hasParentSegment(); }
    virtual bool allowTimeAnchor() const override { return hasParentSegment(); }
    virtual void startEdit(EditData&) override;
    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    virtual void editCut(EditData&) override;
    virtual void editCopy(EditData&) override;
    virtual void endEdit(EditData&) override;
    virtual void editDrag(EditData&) override;
    void movePosition(EditData&, TextCursor::MoveOperation);

    virtual void undoMoveSegment(Segment* newSeg, Fraction tickDiff);
    void checkMeasureBoundariesAndMoveIfNeed();

    bool deleteSelectedText(EditData&);

    void selectAll(TextCursor*);
    void select(EditData&, SelectTextType);
    bool isPrimed() const { return m_primed; }
    void setPrimed(bool primed) { m_primed = primed; }

    virtual void paste(EditData& ed, const String& txt);

    RectF pageRectangle() const;

    const Shape& highResShape() const { return ldata()->highResShape.value(); }
    void computeHighResShape(const muse::draw::FontMetrics& fontMetrics);

    void dragTo(EditData&);

    std::vector<LineF> dragAnchorLines() const override;

    virtual bool acceptDrop(EditData&) const override;
    virtual EngravingItem* drop(EditData&) override;

    friend class TextBlock;
    friend class TextFragment;

    static String unEscape(String s);
    static String escape(String s);

    String accessibleInfo() const override;
    String screenReaderInfo() const override;

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    std::list<TextFragment> fragmentList() const;   // for MusicXML formatted export

    static bool validateText(String& s);
    bool inHexState() const { return m_hexState >= 0; }
    void endHexState(EditData&);

    muse::draw::Font font() const;
    muse::draw::FontMetrics fontMetrics() const;

    bool isPropertyLinkedToMaster(Pid id) const override;
    bool isUnlinkedFromMaster() const override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;
    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;
    Sid getPropertyStyle(Pid) const override;
    void styleChanged() override;
    void editInsertText(TextCursor*, const String&);

    TextCursor* cursorFromEditData(const EditData&);
    TextCursor* cursor() const { return m_cursor; }

    void setTextInvalid() { m_textInvalid = true; }
    bool isTextInvalid() const { return m_textInvalid; }

    // helper functions
    bool hasFrame() const { return m_frameType != FrameType::NO_FRAME; }
    bool circle() const { return m_frameType == FrameType::CIRCLE; }
    bool square() const { return m_frameType == FrameType::SQUARE; }

    TextStyleType textStyleType() const { return m_textStyleType; }
    void setTextStyleType(TextStyleType id) { m_textStyleType = id; }
    void initTextStyleType(TextStyleType id);
    void initTextStyleType(TextStyleType id, bool preserveDifferent);
    virtual void initElementStyle(const ElementStyle*) override;

    static const String UNDEFINED_FONT_FAMILY;
    static const double UNDEFINED_FONT_SIZE;

    bool bold() const { return fontStyle() & FontStyle::Bold; }
    bool italic() const { return fontStyle() & FontStyle::Italic; }
    bool underline() const { return fontStyle() & FontStyle::Underline; }
    bool strike() const { return fontStyle() & FontStyle::Strike; }
    void setBold(bool val) { setFontStyle(val ? fontStyle() + FontStyle::Bold : fontStyle() - FontStyle::Bold); }
    void setItalic(bool val) { setFontStyle(val ? fontStyle() + FontStyle::Italic : fontStyle() - FontStyle::Italic); }
    void setUnderline(bool val)
    {
        setFontStyle(val ? fontStyle() + FontStyle::Underline : fontStyle() - FontStyle::Underline);
    }

    void setStrike(bool val)
    {
        setFontStyle(val ? fontStyle() + FontStyle::Strike : fontStyle() - FontStyle::Strike);
    }

    bool hasCustomFormatting() const;

    friend class TextCursor;
    using EngravingObject::undoChangeProperty;

    Color textColor() const;
    FrameType frameType() const { return m_frameType; }
    void setFrameType(FrameType val) { m_frameType = val; }
    double textLineSpacing() const { return m_textLineSpacing; }
    void setTextLineSpacing(double val) { m_textLineSpacing = val; }
    Color bgColor() const { return m_bgColor; }
    void setBgColor(const Color& val) { m_bgColor = val; }
    Color frameColor() const { return m_frameColor; }
    void setFrameColor(const Color& val) { m_frameColor = val; }
    Spatium frameWidth() const { return m_frameWidth; }
    void setFrameWidth(Spatium val) { m_frameWidth = val; }
    Spatium paddingWidth() const { return m_paddingWidth; }
    void setPaddingWidth(Spatium val) { m_paddingWidth = val; }
    int frameRound() const { return m_frameRound; }
    void setFrameRound(int val) { m_frameRound = val; }

    struct LayoutData : public EngravingItem::LayoutData {
        std::vector<TextBlock> blocks;
        bool layoutInvalid = true;

        RectF frame;

        size_t rows() const { return blocks.size(); }
        const TextBlock& textBlock(size_t i) const { return blocks.at(i); }
        TextBlock& textBlock(size_t i) { return blocks[i]; }

        ld_field<Shape> highResShape = { "[TextBase] highResShape", Shape() };
    };
    DECLARE_LAYOUTDATA_METHODS(TextBase)

    void createBlocks();
    void createBlocks(LayoutData* ldata) const;
    void layoutFrame();
    void layoutFrame(LayoutData* ldata) const;

    //! NOTE It can only be set for some types of text, see who has the setter.
    //! At the moment it's: Text, Jump, Marker
    bool layoutToParentWidth() const { return m_layoutToParentWidth; }

    void setVoiceAssignment(VoiceAssignment v) { m_voiceAssignment = v; }
    VoiceAssignment voiceAssignment() const { return m_voiceAssignment; }
    void setDirection(DirectionV v) { m_direction = v; }
    DirectionV direction() const { return m_direction; }
    void setCenterBetweenStaves(AutoOnOff v) { m_centerBetweenStaves = v; }
    AutoOnOff centerBetweenStaves() const { return m_centerBetweenStaves; }
    void genText();

protected:
    TextBase(const ElementType& type, EngravingItem* parent = 0, TextStyleType tid = TextStyleType::DEFAULT,
             ElementFlags = ElementFlag::NOTHING);
    TextBase(const ElementType& type, EngravingItem* parent, ElementFlags);
    TextBase(const TextBase&);

    virtual void startEditTextual(EditData&);
    virtual void startEditNonTextual(EditData&);
    virtual bool editTextual(EditData&);
    virtual bool editNonTextual(EditData&);
    virtual void endEditNonTextual(EditData&);
    virtual void endEditTextual(EditData&);
    virtual bool isNonTextualEditAllowed(EditData&) const;
    virtual bool isTextualEditAllowed(EditData&) const;
    bool nudge(const EditData& ed);

    bool moveSegment(const EditData&);
    void moveSnappedItems(Segment* newSeg, Fraction tickDiff) const;

    void insertSym(EditData& ed, SymId id);
    void prepareFormat(const String& token, TextCursor& cursor);
    bool prepareFormat(const String& token, CharFormat& format);

    virtual void commitText();

    bool m_layoutToParentWidth = false;

private:

    void drawSelection(muse::draw::Painter*, const RectF&) const;
    void insert(TextCursor*, char32_t code, LayoutData* ldata) const;
    String genText(const LayoutData* ldata) const;

    virtual int getPropertyFlagsIdx(Pid id) const override;
    String stripText(bool, bool, bool) const;
    Sid offsetSid() const;

    static String getHtmlStartTag(double, double&, const String&, String&, FontStyle, VerticalAlignment);
    static String getHtmlEndTag(FontStyle, VerticalAlignment);

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    void notifyAboutTextCursorChanged();
    void notifyAboutTextInserted(int startPosition, int endPosition, const String& text);
    void notifyAboutTextRemoved(int startPosition, int endPosition, const String& text);

    void shiftInitOffset(EditData& ed, const PointF& offsetShift);

    Align m_align;

    FrameType m_frameType = FrameType::NO_FRAME;
    double m_textLineSpacing = 1.0;
    Color m_bgColor;
    Color m_frameColor;
    Spatium m_frameWidth;
    Spatium m_paddingWidth;
    int m_frameRound = 0;

    // there are two representations of text; only one
    // might be valid and the other can be constructed from it

    String m_text;                          // cached
    bool m_textInvalid = true;

    TextStyleType m_textStyleType = TextStyleType::DEFAULT;           // text style id

    int m_hexState = -1;
    bool m_primed = 0;

    TextCursor* m_cursor = nullptr;

    VoiceAssignment m_voiceAssignment = VoiceAssignment::ALL_VOICE_IN_INSTRUMENT;
    DirectionV m_direction = DirectionV::AUTO;
    AutoOnOff m_centerBetweenStaves = AutoOnOff::AUTO;
    bool m_anchorToEndOfPrevious = false;
};

inline bool isTextNavigationKey(int key, KeyboardModifiers modifiers)
{
    // space + TextEditingControlModifier = insert nonbreaking space, so that's *not* a navigation key
    return (key == Key_Space && modifiers != TextEditingControlModifier) || key == Key_Tab;
}
} // namespace mu::engraving

#endif
