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

#ifndef __SIMPLETEXT_H__
#define __SIMPLETEXT_H__

#include "element.h"
#include "style.h"
#include "elementlayout.h"

namespace Ms {

class MuseScoreView;
struct SymCode;

enum class CharFormatType : char { TEXT, SYMBOL };
enum class VerticalAlignment : char { AlignNormal, AlignSuperScript, AlignSubScript };
enum class FormatId : char { Bold, Italic, Underline, Valign, FontSize, FontFamily };

//---------------------------------------------------------
//   CharFormat
//---------------------------------------------------------

class CharFormat {
      CharFormatType _type      { CharFormatType::TEXT };
      bool _bold                { false };
      bool _italic              { false };
      bool _underline           { false };
      VerticalAlignment _valign { VerticalAlignment::AlignNormal };
      qreal _fontSize           { 12.0  };
      QString _fontFamily;

   public:
      CharFormat() {}
      bool operator==(const CharFormat&) const;
      CharFormatType type() const            { return _type;        }
      bool bold() const                      { return _bold;        }
      bool italic() const                    { return _italic;      }
      bool underline() const                 { return _underline;   }
      VerticalAlignment valign() const       { return _valign;   }
      qreal fontSize() const                 { return _fontSize;    }
      QString fontFamily() const             { return _fontFamily;  }

      void setType(CharFormatType val)       { _type        = val;  }
      void setBold(bool val)                 { _bold        = val;  }
      void setItalic(bool val)               { _italic      = val;  }
      void setUnderline(bool val)            { _underline   = val;  }
      void setValign(VerticalAlignment val)  { _valign      = val;  }
      void setFontSize(qreal val)            { _fontSize    = val;  }
      void setFontFamily(const QString& val) { _fontFamily  = val;  }

      void setFormat(FormatId, QVariant);
      };

//---------------------------------------------------------
//   TextCursor
//    Contains current position and start of selection
//    during editing.
//---------------------------------------------------------

class TextCursor {
      CharFormat _format;
      int _line          { 0 };
      int _column        { 0 };
      int _selectLine    { 0 };         // start of selection
      int _selectColumn  { 0 };

   public:
      TextCursor() {}

      bool hasSelection() const { return (_selectLine != _line) || (_selectColumn != _column); }
      void clearSelection();

      CharFormat* format()                { return &_format;  }
      void setFormat(const CharFormat& f) { _format = f;      }

      int line() const              { return _line; }
      int column() const            { return _column; }
      int selectLine() const        { return _selectLine; }
      int selectColumn() const      { return _selectColumn; }
      void setLine(int val)         { _line = val; }
      void setColumn(int val)       { _column = val; }
      void setSelectLine(int val)   { _selectLine = val; }
      void setSelectColumn(int val) { _selectColumn = val; }
      void initFromStyle(const TextStyle& s);
      };

class Text;

//---------------------------------------------------------
//   TextFragment
//    contains a styled text of symbol with name "text"
//---------------------------------------------------------

class TextFragment {

   public:
      mutable CharFormat format;
      QPointF pos;                  // y is relativ to TextBlock->y()

      mutable QString text;
      QList<SymId> ids;

      bool operator ==(const TextFragment& f) const;

      TextFragment();
      TextFragment(const QString& s);
      TextFragment(TextCursor*, SymId);
      TextFragment(TextCursor*, const QString&);
      TextFragment split(int column);
      void draw(QPainter*, const Text*) const;
      QFont font(const Text*) const;
      int columns() const;
      void changeFormat(FormatId id, QVariant data);
      };

//---------------------------------------------------------
//   TextBlock
//    represents a block of formatted text
//---------------------------------------------------------

class TextBlock {
      QList<TextFragment> _text;
      qreal  _y = 0;
      qreal _lineSpacing;
      QRectF _bbox;
      bool _eol = false;

      void simplify();

   public:
      TextBlock() {}
      bool operator ==(const TextBlock& x)         { return _text == x._text; }
      void draw(QPainter*, const Text*) const;
      void layout(Text*);
      const QList<TextFragment>& fragments() const { return _text; }
      QList<TextFragment>& fragments()             { return _text; }
      const QRectF& boundingRect() const           { return _bbox; }
      QRectF boundingRect(int col1, int col2, const Text*) const;
      int columns() const;
      void insert(TextCursor*, const QString&);
      void insert(TextCursor*, SymId);
      void remove(int column);
      void remove(int start, int n);
      int column(qreal x, Text*) const;
      TextBlock split(int column);
      qreal xpos(int col, const Text*) const;
      const CharFormat* formatAt(int) const;
      const TextFragment* fragment(int col) const;
      QList<TextFragment>::iterator fragment(int col, int* rcol);
      qreal y() const      { return _y; }
      void setY(qreal val) { _y = val; }
      qreal lineSpacing() const { return _lineSpacing; }
      QString text(int, int) const;
      bool eol() const      { return _eol; }
      void setEol(bool val) { _eol = val; }
      void changeFormat(FormatId, QVariant val, int start, int n);
      };

//---------------------------------------------------------
//   @@ Text
///    Graphic representation of a text.
//
//   @P text  Qstring  the raw text
//---------------------------------------------------------

class Text : public Element {
      Q_OBJECT

      Q_PROPERTY(QString text READ text WRITE undoSetText)

      QString _text;
      QList<TextBlock> _layout;
      TextStyleType _styleIndex;

      bool _layoutToParentWidth;
      bool _editMode;
      TextStyle _textStyle;

      static TextCursor _cursor;       // used during editing

      QRectF cursorRect() const;
      const TextBlock& curLine() const;
      TextBlock& curLine();
      void drawSelection(QPainter*, const QRectF&) const;

      void insert(TextCursor*, QChar);
      void insert(TextCursor*, SymId);
      void updateCursorFormat(TextCursor*);
      void genText();
      void changeSelectionFormat(FormatId id, QVariant val);

   protected:
      QColor textColor() const;
      QRectF frame;           // calculated in layout()
      void layoutFrame();
      void layoutEdit();
      void createLayout();

   public:
      Text(Score* = 0);
      Text(const Text&);
      ~Text() {}

      virtual Text* clone() const override         { return new Text(*this); }
      virtual Element::Type type() const override  { return Element::Type::TEXT; }
      virtual bool mousePress(const QPointF&, QMouseEvent* ev) override;

      Text &operator=(const Text&) = delete;

      virtual void draw(QPainter*) const override;

      bool editMode() const                   { return _editMode; }
      void setEditMode(bool val)              { _editMode = val;  }

      virtual void setTextStyle(const TextStyle& st);
      const TextStyle& textStyle() const      { return _textStyle; }
      TextStyle& textStyle()                  { return _textStyle; }
      TextStyleType textStyleType() const     { return _styleIndex; }
      void setTextStyleType(TextStyleType);
      void restyle(TextStyleType);

      void setPlainText(const QString&);
      void setText(const QString&);
      QString text() const                    { return _text; }
      QString plainText(bool noSym = false) const;
      void insertText(const QString&);

      virtual void layout() override;
      virtual void layout1();
      void sameLayout();
      qreal lineSpacing() const;
      qreal lineHeight() const;
      virtual qreal baseLine() const override;

      bool isEmpty() const                { return _text.isEmpty(); }
      void clear()                        { _text.clear();          }

      bool layoutToParentWidth() const    { return _layoutToParentWidth; }
      void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v;   }

      void startEdit(MuseScoreView*, const QPointF&);
      void endEdit();
      bool edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers, const QString&);

      void setFormat(FormatId, QVariant);

      bool deletePreviousChar();
      bool deleteChar();
      void deleteSelectedText();

      bool movePosition(QTextCursor::MoveOperation op,
         QTextCursor::MoveMode mode = QTextCursor::MoveAnchor, int count = 1);
      bool setCursor(const QPointF& p,
         QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
      void moveCursorToEnd()   { movePosition(QTextCursor::End); }
      void moveCursorToStart() { movePosition(QTextCursor::Start); }

      QString selectedText() const;

      void insertSym(SymId);
      void selectAll();

      virtual bool systemFlag() const     { return textStyle().systemFlag(); }

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      void writeProperties(Xml&, bool = true, bool = true) const;
      bool readProperties(XmlReader&);

      void spellCheckUnderline(bool) {}
      virtual void textStyleChanged();

      virtual void paste();

      QRectF pageRectangle() const;

      TextCursor* cursor() { return &_cursor; }

      void setAbove(bool val) {  textStyle().setYoff(val ? -2.0 : 7.0); }
      void dragTo(const QPointF&);

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);
      virtual QVariant propertyDefault(P_ID id) const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const override;
      virtual Element* drop(const DropData&) override;

      friend class TextBlock;
      friend class TextFragment;
      virtual void textChanged() {}
      QString convertFromHtml(const QString& ss) const;
      static QString convertToHtml(const QString&, const TextStyle&);

      void undoSetText(const QString& s) { undoChangeProperty(P_ID::TEXT, s); }
      };


}     // namespace Ms
#endif
