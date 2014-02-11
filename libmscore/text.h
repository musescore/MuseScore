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

enum class CharFormatType { TEXT, SYMBOL };
enum class VerticalAlignment { AlignNormal, AlignSuperScript, AlignSubScript };

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
      };

class Text;

//---------------------------------------------------------
//   TextFragment
//    contains a styled text of symbol with name "text"
//---------------------------------------------------------

class TextFragment {
   public:
      mutable CharFormat format;
      QPointF pos;

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
      };

//---------------------------------------------------------
//   TextBlock
//    represents a block of formatted text
//---------------------------------------------------------

class TextBlock {
      QList<TextFragment> _text;
      QRectF _bbox;

      void simplify();

   public:
      TextBlock() {}
      bool operator ==(const TextBlock& x)         { return _text == x._text; }
      QString text(TextCursor*) const;
      void addText(TextCursor*, const QString&);
      void draw(QPainter*, const Text*) const;
      void layout(qreal y, Text*);
      const QList<TextFragment>& fragments() const { return _text; }
      QList<TextFragment>& fragments()             { return _text; }
      QRectF boundingRect() const                  { return _bbox; }
      QRectF boundingRect(int col1, int col2, const Text*) const;
      void moveX(qreal);
      int columns() const;
      void insert(TextCursor*, const QString&);
      void insert(TextCursor*, SymId);
      void remove(int column);
      void remove(int start, int n);
      int column(qreal x, Text*) const;
      TextBlock split(int column);
      qreal xpos(int col, const Text*) const;
      qreal y() const         { return _text.isEmpty() ? 0 : _text.front().pos.y(); }
      const CharFormat& formatAt(int) const;
      const TextFragment& fragment(int col) const;
      };

//---------------------------------------------------------
//   @@ Text
//---------------------------------------------------------

class Text : public Element {
      Q_OBJECT

      QString _text;
      QList<TextBlock> _layout;
      QRectF frame;           // calculated in layout()
      int _styleIndex;        // set to TEXT_STYLE_UNTEXT if not styled

      bool _layoutToParentWidth;
      bool _editMode;

      static TextCursor _cursor;       // used during editing

      QRectF cursorRect() const;
      const TextBlock& curLine() const;
      TextBlock& curLine();
      void drawSelection(QPainter*, const QRectF&) const;
      void createLayout();
      void insert(TextCursor*, QChar);
      void insert(TextCursor*, SymId);
      void updateCursorFormat(TextCursor*);

   protected:
      TextStyle _textStyle;

      void drawFrame(QPainter* painter) const;
      QColor textColor() const;
      void layoutFrame();
      void layoutEdit();

   public:
      Text(Score* = 0);
      Text(const Text&);
      ~Text() {}

      virtual Text* clone() const         { return new Text(*this); }
      virtual ElementType type() const    { return TEXT; }
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);

      Text &operator=(const Text&);

      virtual void draw(QPainter*) const;

      void setTextStyle(const TextStyle& st)  { _textStyle = st;   }
      const TextStyle& textStyle() const      { return _textStyle; }
      TextStyle& textStyle()                  { return _textStyle; }

      void setText(const QString& s)      { _text = s;    }
      QString text() const                { return _text; }
      void insertText(const QString&);

      bool editMode() const               { return _editMode; }
      void setEditMode(bool val)          { _editMode = val;  }

      virtual void layout();
      qreal lineSpacing() const;
      qreal lineHeight() const;
      virtual qreal baseLine() const;

      bool isEmpty() const                { return _text.isEmpty(); }
      void clear()                        { _text.clear();          }

      bool layoutToParentWidth() const    { return _layoutToParentWidth; }
      void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v;   }

      void startEdit(MuseScoreView*, const QPointF&);
      void endEdit();
      bool edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers, const QString&);

      bool deletePreviousChar();
      bool deleteChar();
      void deleteSelectedText();

      bool movePosition(QTextCursor::MoveOperation op,
         QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
      bool setCursor(const QPointF& p,
         QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
      void moveCursorToEnd()   { movePosition(QTextCursor::Start); }
      void moveCursorToStart() { movePosition(QTextCursor::End); }

      QString selectedText() const;

      void insertSym(SymId);
      void selectAll();

      void setUnstyled()                  { _styleIndex = TEXT_STYLE_UNSTYLED; }
      bool styled() const                 { return _styleIndex != TEXT_STYLE_UNSTYLED; }
      int textStyleType() const           { return _styleIndex; }

      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      void writeProperties(Xml&, bool = true) const;
      bool readProperties(XmlReader&);

      void setTextStyleType(int);

      void spellCheckUnderline(bool) {}
      void layout1();
      virtual void textStyleChanged();

      virtual void paste();

      QRectF pageRectangle() const;

      TextCursor* cursor() { return &_cursor; }

      void setCurHalign(int);

      void setAbove(bool val) {  textStyle().setYoff(val ? -2.0 : 7.0); }
      void dragTo(const QPointF&);

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);

      friend class TextBlock;
      friend class TextFragment;
      };


}     // namespace Ms
#endif
