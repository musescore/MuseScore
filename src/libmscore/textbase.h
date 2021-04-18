//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTBASE_H__
#define __TEXTBASE_H__

#include <QTextCursor>

#include "element.h"
#include "property.h"
#include "style.h"

namespace Ms {
class MuseScoreView;
struct SymCode;
class TextBase;
class TextBlock;
class ChangeText;

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
    Bold, Italic, Underline, Valign, FontSize, FontFamily
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
    bool _preedit             { false };
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
    void setBold(bool val) { _style = val ? _style + FontStyle::Bold : _style - FontStyle::Bold; }
    void setItalic(bool val) { _style = val ? _style + FontStyle::Italic : _style - FontStyle::Italic; }
    void setUnderline(bool val) { _style = val ? _style + FontStyle::Underline : _style - FontStyle::Underline; }

    bool preedit() const { return _preedit; }
    VerticalAlignment valign() const { return _valign; }
    qreal fontSize() const { return _fontSize; }
    QString fontFamily() const { return _fontFamily; }
    void setPreedit(bool val) { _preedit = val; }
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

public:
    TextCursor(TextBase* t)
        : _text(t) {}

    TextBase* text() const { return _text; }
    bool hasSelection() const { return (_selectLine != _row) || (_selectColumn != _column); }
    void clearSelection();

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

    TextBlock& curLine() const;
    QRectF cursorRect() const;
    bool movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor, int count = 1);
    void doubleClickSelect();
    void moveCursorToEnd() { movePosition(QTextCursor::End); }
    void moveCursorToStart() { movePosition(QTextCursor::Start); }
    QChar currentCharacter() const;
    QString currentWord() const;
    QString currentLine() const;
    bool set(const QPointF& p, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    QString selectedText() const;
    QString extractText(int r1, int c1, int r2, int c2) const;
    void updateCursorFormat();
    void setFormat(FormatId, QVariant);
    void changeSelectionFormat(FormatId id, QVariant val);
    const CharFormat selectedFragmentsFormat() const;

private:
    QString accessibleCurrentCharacter() const;
    void accessibileMessage(QString& accMsg, int oldRow, int oldCol, QString oldSelection, QTextCursor::MoveMode mode) const;
};

//---------------------------------------------------------
//   TextFragment
//    contains a styled text
//---------------------------------------------------------

class TextFragment
{
public:
    mutable CharFormat format;
    QPointF pos;                    // y is relative to TextBlock->y()
    mutable QString text;

    bool operator ==(const TextFragment& f) const;

    TextFragment();
    TextFragment(const QString& s);
    TextFragment(TextCursor*, const QString&);
    TextFragment split(int column);
    void draw(mu::draw::Painter*, const TextBase*) const;
    QFont font(const TextBase*) const;
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
    QRectF _bbox;
    bool _eol = false;

    void simplify();

public:
    TextBlock() {}
    bool operator ==(const TextBlock& x) { return _fragments == x._fragments; }
    bool operator !=(const TextBlock& x) { return _fragments != x._fragments; }
    void draw(mu::draw::Painter*, const TextBase*) const;
    void layout(TextBase*);
    const QList<TextFragment>& fragments() const { return _fragments; }
    QList<TextFragment>& fragments() { return _fragments; }
    QList<TextFragment>* fragmentsWithoutEmpty();
    const QRectF& boundingRect() const { return _bbox; }
    QRectF boundingRect(int col1, int col2, const TextBase*) const;
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
    QString text(int, int) const;
    bool eol() const { return _eol; }
    void setEol(bool val) { _eol = val; }
    void changeFormat(FormatId, QVariant val, int start, int n);
};

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase : public Element
{
    // sorted by size to allow for most compact memory layout
    M_PROPERTY(Align,      align,                  setAlign)
    M_PROPERTY(FrameType,  frameType,              setFrameType)
    M_PROPERTY(qreal,      textLineSpacing,        setTextLineSpacing)
    M_PROPERTY(QColor,     bgColor,                setBgColor)
    M_PROPERTY(QColor,     frameColor,             setFrameColor)
    M_PROPERTY(Spatium,    frameWidth,             setFrameWidth)
    M_PROPERTY(Spatium,    paddingWidth,           setPaddingWidth)
    M_PROPERTY(int,        frameRound,             setFrameRound)

    // there are two representations of text; only one
    // might be valid and the other can be constructed from it

    mutable QString _text;                          // cached
    mutable bool textInvalid      { true };

    QList<TextBlock> _layout;
    bool layoutInvalid            { true };
    Tid _tid;           // text style id

    QString preEdit;                // move to EditData?
    bool _layoutToParentWidth     { false };

    int hexState                 { -1 };
    bool _primed                  { 0 };

    TextCursor* _cursor           { nullptr };

    void drawSelection(mu::draw::Painter*, const QRectF&) const;
    void insert(TextCursor*, uint code);
    void genText() const;
    virtual int getPropertyFlagsIdx(Pid id) const override;
    QString stripText(bool, bool, bool) const;
    Sid offsetSid() const;

protected:
    QColor textColor() const;
    QRectF frame;             // calculated in layout()
    void layoutFrame();
    void layoutEdit();
    void createLayout();
    void insertSym(EditData& ed, SymId id);

public:
    TextBase(Score* = 0, Tid tid = Tid::DEFAULT, ElementFlags = ElementFlag::NOTHING);
    TextBase(Score*, ElementFlags);
    TextBase(const TextBase&);

    virtual bool mousePress(EditData&) override;

    Text& operator=(const Text&) = delete;

    virtual void draw(mu::draw::Painter*) const override;
    virtual void drawEditMode(mu::draw::Painter* p, EditData& ed) override;
    static void drawTextWorkaround(mu::draw::Painter* p, QFont& f, const QPointF pos, const QString text);

    static QString plainToXmlText(const QString& s) { return s.toHtmlEscaped(); }
    void setPlainText(const QString& t) { setXmlText(plainToXmlText(t)); }
    virtual void setXmlText(const QString&);
    QString xmlText() const;
    QString plainText() const;

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
    virtual bool edit(EditData&) override;
    virtual void editCut(EditData&) override;
    virtual void editCopy(EditData&) override;
    virtual void endEdit(EditData&) override;
    void movePosition(EditData&, QTextCursor::MoveOperation);

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

    virtual void paste(EditData&);

    QRectF pageRectangle() const;

    void dragTo(EditData&);

    QVector<QLineF> dragAnchorLines() const override;

    virtual bool acceptDrop(EditData&) const override;
    virtual Element* drop(EditData&) override;

    friend class TextBlock;
    friend class TextFragment;
    QString convertFromHtml(const QString& ss) const;
    static QString convertToHtml(const QString&, const TextStyle&);
    static QString tagEscape(QString s);
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
    void inputTransition(EditData&, QInputMethodEvent*);

    QFont font() const;
    QFontMetricsF fontMetrics() const;

    virtual QVariant getProperty(Pid propertyId) const override;
    virtual bool setProperty(Pid propertyId, const QVariant& v) override;
    virtual QVariant propertyDefault(Pid id) const override;
    virtual void undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps) override;
    virtual Pid propertyId(const QStringRef& xmlName) const override;
    virtual Sid getPropertyStyle(Pid) const override;
    virtual void styleChanged() override;
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

    Tid tid() const { return _tid; }
    void setTid(Tid id) { _tid = id; }
    void initTid(Tid id);
    void initTid(Tid id, bool preserveDifferent);
    virtual void initElementStyle(const ElementStyle*) override;

    static const QString UNDEFINED_FONT_FAMILY;
    static const int UNDEFINED_FONT_SIZE;

    bool bold() const { return fontStyle() & FontStyle::Bold; }
    bool italic() const { return fontStyle() & FontStyle::Italic; }
    bool underline() const { return fontStyle() & FontStyle::Underline; }
    void setBold(bool val) { setFontStyle(val ? fontStyle() + FontStyle::Bold : fontStyle() - FontStyle::Bold); }
    void setItalic(bool val) { setFontStyle(val ? fontStyle() + FontStyle::Italic : fontStyle() - FontStyle::Italic); }
    void setUnderline(bool val)
    {
        setFontStyle(val ? fontStyle() + FontStyle::Underline : fontStyle() - FontStyle::Underline);
    }

    bool hasCustomFormatting() const;

    friend class TextCursor;
    using ScoreElement::undoChangeProperty;
};
}     // namespace Ms

#endif
