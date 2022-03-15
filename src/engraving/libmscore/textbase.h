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

#include "infrastructure/draw/fontmetrics.h"

#include "infrastructure/draw/color.h"
#include "engravingitem.h"
#include "property.h"
#include "style/style.h"

namespace Ms {
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

enum class VerticalAlignment : char {
    AlignUndefined = -1, AlignNormal, AlignSuperScript, AlignSubScript
};

//---------------------------------------------------------
//   FormatId
//---------------------------------------------------------

enum class FormatId : char {
    Bold, Italic, Underline, Strike, Valign, FontSize, FontFamily
};

//---------------------------------------------------------
//   MultiClick
//---------------------------------------------------------

enum class MultiClick : char {
    Double, Triple
};

//---------------------------------------------------------
//   CharFormat
//---------------------------------------------------------

class CharFormat
{
    FontStyle _style          { FontStyle::Normal };
    VerticalAlignment _valign { VerticalAlignment::AlignNormal };
    qreal _fontSize           { 12.0 };
    qreal _textLineSpacing    { 1.0 };
    QString _fontFamily;

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
    qreal fontSize() const { return _fontSize; }
    QString fontFamily() const { return _fontFamily; }
    void setValign(VerticalAlignment val) { _valign = val; }
    void setFontSize(qreal val) { _fontSize = val; }
    void setFontFamily(const QString& val) { _fontFamily = val; }
    void setTextLineSpacing(qreal val) { _textLineSpacing = val; }

    QVariant formatValue(FormatId) const;
    void setFormatValue(FormatId, QVariant);
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
    int _row           { 0 };
    int _column        { 0 };
    int _selectLine    { 0 };           // start of selection
    int _selectColumn  { 0 };
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

    int row() const { return _row; }
    int column() const { return _column; }
    int selectLine() const { return _selectLine; }
    int selectColumn() const { return _selectColumn; }
    void setRow(int val) { _row = val; }
    void setColumn(int val) { _column = val; }
    void setSelectLine(int val) { _selectLine = val; }
    void setSelectColumn(int val) { _selectColumn = val; }
    int columns() const;
    void init();

    struct Range {
        int startPosition = 0;
        int endPosition = 0;
        QString text;
    };

    std::pair<int, int> positionToLocalCoord(int position) const;

    int currentPosition() const;
    Range selectionRange() const;

    TextBlock& curLine() const;
    mu::RectF cursorRect() const;
    bool movePosition(TextCursor::MoveOperation op, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor, int count = 1);
    void doubleClickSelect();
    void moveCursorToEnd() { movePosition(TextCursor::MoveOperation::End); }
    void moveCursorToStart() { movePosition(TextCursor::MoveOperation::Start); }
    QChar currentCharacter() const;
    bool set(const mu::PointF& p, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor);
    QString selectedText(bool withFormat = false) const;
    QString extractText(int r1, int c1, int r2, int c2, bool withFormat = false) const;
    void updateCursorFormat();
    void setFormat(FormatId, QVariant);
    void changeSelectionFormat(FormatId id, QVariant val);
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
    mutable QString text;

    bool operator ==(const TextFragment& f) const;

    TextFragment();
    TextFragment(const QString& s);
    TextFragment(TextCursor*, const QString&);
    TextFragment split(int column);
    void draw(mu::draw::Painter*, const TextBase*) const;
    mu::draw::Font font(const TextBase*) const;
    int columns() const;
    void changeFormat(FormatId id, QVariant data);
};

//---------------------------------------------------------
//   TextBlock
//    represents a block of formatted text
//---------------------------------------------------------

class TextBlock
{
    QList<TextFragment> _fragments;
    qreal _y = 0;
    qreal _lineSpacing = 0.0;
    mu::RectF _bbox;
    bool _eol = false;

    void simplify();

public:
    TextBlock() {}
    bool operator ==(const TextBlock& x) const { return _fragments == x._fragments; }
    bool operator !=(const TextBlock& x) const { return _fragments != x._fragments; }
    void draw(mu::draw::Painter*, const TextBase*) const;
    void layout(TextBase*);
    const QList<TextFragment>& fragments() const { return _fragments; }
    QList<TextFragment>& fragments() { return _fragments; }
    QList<TextFragment>* fragmentsWithoutEmpty();
    const mu::RectF& boundingRect() const { return _bbox; }
    mu::RectF boundingRect(int col1, int col2, const TextBase*) const;
    int columns() const;
    void insert(TextCursor*, const QString&);
    void insertEmptyFragmentIfNeeded(TextCursor*);
    void removeEmptyFragment();
    QString remove(int column, TextCursor*);
    QString remove(int start, int n, TextCursor*);
    int column(qreal x, TextBase*) const;
    TextBlock split(int column, TextCursor* cursor);
    qreal xpos(int col, const TextBase*) const;
    const CharFormat* formatAt(int) const;
    const TextFragment* fragment(int col) const;
    QList<TextFragment>::iterator fragment(int column, int* rcol, int* ridx);
    qreal y() const { return _y; }
    void setY(qreal val) { _y = val; }
    qreal lineSpacing() const { return _lineSpacing; }
    QString text(int, int, bool = false) const;
    bool eol() const { return _eol; }
    void setEol(bool val) { _eol = val; }
    void changeFormat(FormatId, QVariant val, int start, int n);
};

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase : public EngravingItem
{
    // sorted by size to allow for most compact memory layout
    M_PROPERTY(FrameType,  frameType,              setFrameType)
    M_PROPERTY(qreal,      textLineSpacing,        setTextLineSpacing)
    M_PROPERTY(mu::draw::Color,      bgColor,                setBgColor)
    M_PROPERTY(mu::draw::Color,      frameColor,             setFrameColor)
    M_PROPERTY(Spatium,    frameWidth,             setFrameWidth)
    M_PROPERTY(Spatium,    paddingWidth,           setPaddingWidth)
    M_PROPERTY(int,        frameRound,             setFrameRound)

    Align _align;

    // there are two representations of text; only one
    // might be valid and the other can be constructed from it

    mutable QString _text;                          // cached
    mutable bool textInvalid      { true };

    QList<TextBlock> _layout;
    bool layoutInvalid            { true };
    TextStyleType _textStyleType;           // text style id

    bool _layoutToParentWidth     { false };

    int hexState                 { -1 };
    bool _primed                  { 0 };

    TextCursor* _cursor           { nullptr };

    void drawSelection(mu::draw::Painter*, const mu::RectF&) const;
    void insert(TextCursor*, uint code);
    void genText() const;
    virtual int getPropertyFlagsIdx(Pid id) const override;
    QString stripText(bool, bool, bool) const;
    Sid offsetSid() const;

    static QString getHtmlStartTag(qreal, qreal&, const QString&, QString&, Ms::FontStyle, Ms::VerticalAlignment);
    static QString getHtmlEndTag(Ms::FontStyle, Ms::VerticalAlignment);

    mu::engraving::AccessibleItem* createAccessible() override;

    void notifyAboutTextCursorChanged();
    void notifyAboutTextInserted(int startPosition, int endPosition, const QString& text);
    void notifyAboutTextRemoved(int startPosition, int endPosition, const QString& text);

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
    void prepareFormat(const QString& token, Ms::TextCursor& cursor);
    bool prepareFormat(const QString& token, Ms::CharFormat& format);

    virtual void commitText();

public:

    ~TextBase();

    virtual bool mousePress(EditData&) override;

    Text& operator=(const Text&) = delete;

    virtual void draw(mu::draw::Painter*) const override;
    virtual void drawEditMode(mu::draw::Painter* p, EditData& ed, qreal currentViewScaling) override;
    static void drawTextWorkaround(mu::draw::Painter* p, mu::draw::Font& f, const mu::PointF& pos, const QString& text);

    Align align() const { return _align; }
    void setAlign(Align a) { _align = a; }

    static QString plainToXmlText(const QString& s) { return s.toHtmlEscaped(); }
    void setPlainText(const QString& t) { setXmlText(plainToXmlText(t)); }
    virtual void setXmlText(const QString&);
    QString xmlText() const;
    QString plainText() const;
    void resetFormatting();

    void insertText(EditData&, const QString&);

    virtual void layout() override;
    virtual void layout1();
    qreal lineSpacing() const;
    qreal lineHeight() const;
    virtual qreal baseLine() const override;

    bool empty() const { return xmlText().isEmpty(); }
    void clear() { setXmlText(QString()); }

    FontStyle fontStyle() const;
    QString family() const;
    qreal size() const;

    void setFontStyle(const FontStyle& val);
    void setFamily(const QString& val);
    void setSize(const qreal& val);

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
    void multiClickSelect(EditData&, MultiClick);
    bool isPrimed() const { return _primed; }
    void setPrimed(bool primed) { _primed = primed; }

    virtual void write(XmlWriter& xml) const override;
    virtual void read(XmlReader&) override;
    virtual void writeProperties(XmlWriter& xml) const override { writeProperties(xml, true, true); }
    void writeProperties(XmlWriter& xml, bool writeText) const { writeProperties(xml, writeText, true); }
    void writeProperties(XmlWriter&, bool, bool) const;
    bool readProperties(XmlReader&) override;

    virtual void paste(EditData& ed, const QString& txt);

    mu::RectF pageRectangle() const;

    void dragTo(EditData&);

    QVector<mu::LineF> dragAnchorLines() const override;

    virtual bool acceptDrop(EditData&) const override;
    virtual EngravingItem* drop(EditData&) override;

    friend class TextBlock;
    friend class TextFragment;

    static QString unEscape(QString s);
    static QString escape(QString s);

    virtual QString accessibleInfo() const override;
    virtual QString screenReaderInfo() const override;

    virtual int subtype() const override;
    virtual QString subtypeName() const override;

    QList<TextFragment> fragmentList() const;   // for MusicXML formatted export

    static bool validateText(QString& s);
    bool inHexState() const { return hexState >= 0; }
    void endHexState(EditData&);

    mu::draw::Font font() const;
    mu::draw::FontMetrics fontMetrics() const;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue& v) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;
    void undoChangeProperty(Pid id, const mu::engraving::PropertyValue& v, PropertyFlags ps) override;
    Pid propertyId(const QStringRef& xmlName) const override;
    Sid getPropertyStyle(Pid) const override;
    void styleChanged() override;
    void editInsertText(TextCursor*, const QString&);

    TextCursor* cursorFromEditData(const EditData&);
    TextCursor* cursor() const { return _cursor; }
    const TextBlock& textBlock(int line) const { return _layout[line]; }
    TextBlock& textBlock(int line) { return _layout[line]; }
    QList<TextBlock>& textBlockList() { return _layout; }
    int rows() const { return _layout.size(); }

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

    static const QString UNDEFINED_FONT_FAMILY;
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
inline bool isTextNavigationKey(int key, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier) {
        static const std::set<int> standardTextOperationsKeys {
            Qt::Key_Space, // Ctrl + Space inserts the space symbol
            Qt::Key_A // select all
        };

        return standardTextOperationsKeys.find(key) == standardTextOperationsKeys.end();
    }

    static const std::set<int> navigationKeys {
        Qt::Key_Space,
        Qt::Key_Tab
    };

    return navigationKeys.find(key) != navigationKeys.end();
}
}     // namespace Ms

#endif
