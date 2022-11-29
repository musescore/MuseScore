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

#ifndef __TEXTBASE_H__
#define __TEXTBASE_H__

#include <variant>

#include "engravingitem.h"
#include "property.h"
#include "types.h"

#include "draw/fontmetrics.h"
#include "draw/types/color.h"

#include "style/style.h"

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
    FontStyle _style          { FontStyle::Normal };
    VerticalAlignment _valign { VerticalAlignment::AlignNormal };
    double _fontSize           { 12.0 };
    double _textLineSpacing    { 1.0 };
    String _fontFamily;

public:
    CharFormat() {}
    bool operator==(const CharFormat&) const;

    FontStyle style() const { return _style; }
    void setStyle(FontStyle s) { _style = s; }
    bool bold() const { return _style & FontStyle::Bold; }
    bool italic() const { return _style & FontStyle::Italic; }
    bool underline() const { return _style & FontStyle::Underline; }
    bool strike() const { return _style & FontStyle::Strike; }
    void setBold(bool val) { _style = val ? _style + FontStyle::Bold : _style - FontStyle::Bold; }
    void setItalic(bool val) { _style = val ? _style + FontStyle::Italic : _style - FontStyle::Italic; }
    void setUnderline(bool val) { _style = val ? _style + FontStyle::Underline : _style - FontStyle::Underline; }
    void setStrike(bool val) { _style = val ? _style + FontStyle::Strike : _style - FontStyle::Strike; }

    VerticalAlignment valign() const { return _valign; }
    double fontSize() const { return _fontSize; }
    String fontFamily() const { return _fontFamily; }
    void setValign(VerticalAlignment val) { _valign = val; }
    void setFontSize(double val) { _fontSize = val; }
    void setFontFamily(const String& val) { _fontFamily = val; }
    void setTextLineSpacing(double val) { _textLineSpacing = val; }

    FormatValue formatValue(FormatId) const;
    void setFormatValue(FormatId, const FormatValue& val);
};

//---------------------------------------------------------
//   TextCursor
//    Contains current position and start of selection
//    during editing.
//---------------------------------------------------------

class TextCursor
{
    TextBase* _text;
    CharFormat _format;
    size_t _row           { 0 };
    size_t _column        { 0 };
    size_t _selectLine    { 0 };           // start of selection
    size_t _selectColumn  { 0 };
    bool _editing { false };

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
        : _text(t) {}

    TextBase* text() const { return _text; }
    bool hasSelection() const { return (_selectLine != _row) || (_selectColumn != _column); }
    void clearSelection();
    void endEdit();
    void startEdit();
    bool editing() const { return _editing; }

    CharFormat* format() { return &_format; }
    const CharFormat* format() const { return &_format; }
    void setFormat(const CharFormat& f) { _format = f; }

    size_t row() const { return _row; }
    size_t column() const { return _column; }
    size_t selectLine() const { return _selectLine; }
    size_t selectColumn() const { return _selectColumn; }
    void setRow(size_t val) { _row = val; }
    void setColumn(size_t val) { _column = val; }
    void setSelectLine(size_t val) { _selectLine = val; }
    void setSelectColumn(size_t val) { _selectColumn = val; }
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

    TextBlock& curLine() const;
    mu::RectF cursorRect() const;
    bool movePosition(TextCursor::MoveOperation op, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor, int count = 1);
    void selectWord();
    void moveCursorToEnd() { movePosition(TextCursor::MoveOperation::End); }
    void moveCursorToStart() { movePosition(TextCursor::MoveOperation::Start); }
    Char currentCharacter() const;
    bool set(const mu::PointF& p, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor);
    String selectedText(bool withFormat = false) const;
    String extractText(int r1, int c1, int r2, int c2, bool withFormat = false) const;
    void updateCursorFormat();
    void setFormat(FormatId, FormatValue val);
    void changeSelectionFormat(FormatId id, const FormatValue& val);
    const CharFormat selectedFragmentsFormat() const;

private:
    Range range(int start, int end) const;
    int position(int row, int column) const;
};

//---------------------------------------------------------
//   TextFragment
//    contains a styled text
//---------------------------------------------------------

class TextFragment
{
public:
    mutable CharFormat format;
    mu::PointF pos;                    // y is relative to TextBlock->y()
    mutable String text;

    bool operator ==(const TextFragment& f) const;

    TextFragment();
    TextFragment(const String& s);
    TextFragment(TextCursor*, const String&);
    TextFragment split(int column);
    void draw(mu::draw::Painter*, const TextBase*) const;
    mu::draw::Font font(const TextBase*) const;
    int columns() const;
    void changeFormat(FormatId id, const FormatValue& data);
};

//---------------------------------------------------------
//   TextBlock
//    represents a block of formatted text
//---------------------------------------------------------

class TextBlock
{
    std::list<TextFragment> _fragments;
    double _y = 0;
    double _lineSpacing = 0.0;
    mu::RectF _bbox;
    bool _eol = false;

    void simplify();

public:
    TextBlock() {}
    bool operator ==(const TextBlock& x) const { return _fragments == x._fragments; }
    bool operator !=(const TextBlock& x) const { return _fragments != x._fragments; }
    void draw(mu::draw::Painter*, const TextBase*) const;
    void layout(TextBase*);
    const std::list<TextFragment>& fragments() const { return _fragments; }
    std::list<TextFragment>& fragments() { return _fragments; }
    std::list<TextFragment> fragmentsWithoutEmpty();
    const mu::RectF& boundingRect() const { return _bbox; }
    mu::RectF boundingRect(int col1, int col2, const TextBase*) const;
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
    double y() const { return _y; }
    void setY(double val) { _y = val; }
    double lineSpacing() const { return _lineSpacing; }
    String text(int, int, bool = false) const;
    bool eol() const { return _eol; }
    void setEol(bool val) { _eol = val; }
    void changeFormat(FormatId, const FormatValue& val, int start, int n);
};

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TextBase)

    // sorted by size to allow for most compact memory layout
    M_PROPERTY(FrameType,  frameType,              setFrameType)
    M_PROPERTY(double,      textLineSpacing,        setTextLineSpacing)
    M_PROPERTY(mu::draw::Color,      bgColor,                setBgColor)
    M_PROPERTY(mu::draw::Color,      frameColor,             setFrameColor)
    M_PROPERTY(Spatium,    frameWidth,             setFrameWidth)
    M_PROPERTY(Spatium,    paddingWidth,           setPaddingWidth)
    M_PROPERTY(int,        frameRound,             setFrameRound)

    Align _align;

    // there are two representations of text; only one
    // might be valid and the other can be constructed from it

    mutable String _text;                          // cached
    mutable bool textInvalid      { true };

    std::vector<TextBlock> _layout;
    bool layoutInvalid            { true };
    TextStyleType _textStyleType;           // text style id

    bool _layoutToParentWidth     { false };

    int hexState                 { -1 };
    bool _primed                  { 0 };

    TextCursor* _cursor           { nullptr };

    void drawSelection(mu::draw::Painter*, const mu::RectF&) const;
    void insert(TextCursor*, char32_t code);
    void genText() const;
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

    virtual bool alwaysKernable() const override { return true; }

protected:
    TextBase(const ElementType& type, EngravingItem* parent = 0, TextStyleType tid = TextStyleType::DEFAULT,
             ElementFlags = ElementFlag::NOTHING);
    TextBase(const ElementType& type, EngravingItem* parent, ElementFlags);
    TextBase(const TextBase&);

    mu::draw::Color textColor() const;
    mu::RectF frame;             // calculated in layout()
    void layoutFrame();
    void layoutEdit();
    void createLayout();
    void insertSym(EditData& ed, SymId id);
    void prepareFormat(const String& token, TextCursor& cursor);
    bool prepareFormat(const String& token, CharFormat& format);

    virtual void commitText();

public:

    ~TextBase();

    virtual bool mousePress(EditData&) override;

    Text& operator=(const Text&) = delete;

    virtual void draw(mu::draw::Painter*) const override;
    virtual void drawEditMode(mu::draw::Painter* p, EditData& ed, double currentViewScaling) override;
    static void drawTextWorkaround(mu::draw::Painter* p, mu::draw::Font& f, const mu::PointF& pos, const String& text);

    Align align() const { return _align; }
    void setAlign(Align a) { _align = a; }

    static String plainToXmlText(const String& s) { return s.toXmlEscaped(); }
    void setPlainText(const String& t) { setXmlText(plainToXmlText(t)); }
    virtual void setXmlText(const String&);
    void setXmlText(const char* str) { setXmlText(String::fromUtf8(str)); }
    String xmlText() const;
    String plainText() const;
    void resetFormatting();

    void insertText(EditData&, const String&);

    virtual void layout() override;
    virtual void layout1();
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

    bool layoutToParentWidth() const { return _layoutToParentWidth; }
    void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v; }

    virtual void startEdit(EditData&) override;
    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    virtual void editCut(EditData&) override;
    virtual void editCopy(EditData&) override;
    virtual void endEdit(EditData&) override;
    void movePosition(EditData&, TextCursor::MoveOperation);

    bool deleteSelectedText(EditData&);

    void selectAll(TextCursor*);
    void select(EditData&, SelectTextType);
    bool isPrimed() const { return _primed; }
    void setPrimed(bool primed) { _primed = primed; }

    virtual void write(XmlWriter& xml) const override;
    virtual void read(XmlReader&) override;
    virtual void writeProperties(XmlWriter& xml) const override { writeProperties(xml, true, true); }
    void writeProperties(XmlWriter& xml, bool writeText) const { writeProperties(xml, writeText, true); }
    void writeProperties(XmlWriter&, bool, bool) const;
    bool readProperties(XmlReader&) override;

    virtual void paste(EditData& ed, const String& txt);

    mu::RectF pageRectangle() const;

    void dragTo(EditData&);

    std::vector<mu::LineF> dragAnchorLines() const override;

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
    bool inHexState() const { return hexState >= 0; }
    void endHexState(EditData&);

    mu::draw::Font font() const;
    mu::draw::FontMetrics fontMetrics() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;
    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;
    Sid getPropertyStyle(Pid) const override;
    void styleChanged() override;
    void editInsertText(TextCursor*, const String&);

    TextCursor* cursorFromEditData(const EditData&);
    TextCursor* cursor() const { return _cursor; }
    const TextBlock& textBlock(int line) const { return _layout[line]; }
    TextBlock& textBlock(int line) { return _layout[line]; }
    std::vector<TextBlock>& textBlockList() { return _layout; }
    size_t rows() const { return _layout.size(); }

    void setTextInvalid() { textInvalid = true; }
    bool isTextInvalid() const { return textInvalid; }
    void setLayoutInvalid() { layoutInvalid = true; }
    bool isLayoutInvalid() const { return layoutInvalid; }

    // helper functions
    bool hasFrame() const { return _frameType != FrameType::NO_FRAME; }
    bool circle() const { return _frameType == FrameType::CIRCLE; }
    bool square() const { return _frameType == FrameType::SQUARE; }

    TextStyleType textStyleType() const { return _textStyleType; }
    void setTextStyleType(TextStyleType id) { _textStyleType = id; }
    void initTextStyleType(TextStyleType id);
    void initTextStyleType(TextStyleType id, bool preserveDifferent);
    virtual void initElementStyle(const ElementStyle*) override;

    static const String UNDEFINED_FONT_FAMILY;
    static const int UNDEFINED_FONT_SIZE;

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
};

// allow shortcut key controller to handle
inline bool isTextNavigationKey(int key, KeyboardModifiers modifiers)
{
    if (modifiers & TextEditingControlModifier) {
        static const std::set<int> controlNavigationKeys {
            Key_Left,
            Key_Right,
            Key_Up,
            Key_Down,
            Key_Home,
            Key_End
        };

        return controlNavigationKeys.find(key) != controlNavigationKeys.end();
    }

    static const std::set<int> navigationKeys {
        Key_Space,
        Key_Tab
    };

    return navigationKeys.find(key) != navigationKeys.end();
}
} // namespace mu::engraving

#endif
