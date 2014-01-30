//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
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

//---------------------------------------------------------
//   TCursor
//    Contains current position and start of selection
//    during editing.
//---------------------------------------------------------

class TCursor {
   public:
      int selectLine;         // start of selection
      int selectColumn;

      int line;
      int column;

      bool hasSelection() const { return (selectLine != line) || (selectColumn != column); }
      };

enum class CharFormat { STYLED, SYMBOL };
class SimpleText;

//---------------------------------------------------------
//   TFragment
//    contains a styled text of symbol with name "text"
//---------------------------------------------------------

class TFragment {
   public:
      CharFormat cf;          // STYLED or SYMBOL
      QPointF pos;

      QString text;
      QList<SymId> ids;

      bool operator ==(const TFragment& f) const;

      TFragment() {}
      TFragment(const QString& s) {
            cf = CharFormat::STYLED;
            text = s;
            }
      TFragment(SymId id) {
            cf = CharFormat::SYMBOL;
            ids.append(id);
            }
      TFragment split(int column);
      void draw(QPainter* p, const SimpleText* t) const;
      };


//---------------------------------------------------------
//   TLine
//    represents a line of formatted text
//---------------------------------------------------------

class TLine {
      QList<TFragment> _text;
      QRectF _bbox;

   public:
      TLine() {}
      TLine(const QString&);
      bool operator ==(const TLine& x)          { return _text == x._text; }
      QString text() const;
      void setText(const QString&);
      void draw(QPainter*, const SimpleText*) const;
      void layout(qreal y, SimpleText*);
      const QList<TFragment>& fragments() const { return _text; }
      QList<TFragment>& fragments()             { return _text; }
      QRectF boundingRect() const               { return _bbox; }
      QRectF boundingRect(int col1, int col2, const SimpleText*) const;
      void moveX(qreal);
      int columns() const;
      void insert(int column, const QString&);
      void insert(int column, SymId);
      void remove(int column);
      void remove(int start, int n);
      int column(qreal x, SimpleText*) const;
      TLine split(int column);
      qreal xpos(int col, const SimpleText*) const;
      qreal y() const         { return _text.isEmpty() ? 0 : _text.front().pos.y(); }
      };

//---------------------------------------------------------
//   @@ SimpleText
//---------------------------------------------------------

class SimpleText : public Element {
      Q_OBJECT

      QString _text;
      QList<TLine> _layout;
      QRectF frame;           // calculated in layout()

      bool _layoutToParentWidth;
      bool _editMode;

      static TCursor _cursor;       // used during editing

      QRectF cursorRect() const;
      const TLine& curLine() const;
      TLine& curLine();
      void drawSelection(QPainter*, const QRectF&) const;

   protected:
      TextStyle _textStyle;

      void drawFrame(QPainter* painter) const;
      QColor textColor() const;
      void layoutFrame();
      virtual void draw(QPainter*) const;

   public:
      SimpleText(Score*);
      SimpleText(const SimpleText&);
      ~SimpleText() {}

      SimpleText &operator=(const SimpleText&);

      void setTextStyle(const TextStyle& st)  { _textStyle = st;   }
      const TextStyle& textStyle() const      { return _textStyle; }
      TextStyle& textStyle()                  { return _textStyle; }

      void setText(const QString& s)      { _text = s;    }
      QString text() const                { return _text; }

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

      Spatium frameWidth() const     { return textStyle().frameWidth(); }
      QColor backgroundColor() const { return textStyle().backgroundColor(); }
      bool hasFrame() const          { return textStyle().hasFrame(); }
      Spatium paddingWidth() const   { return textStyle().paddingWidth(); }
      QColor frameColor() const      { return textStyle().frameColor(); }
      int frameRound() const         { return textStyle().frameRound(); }
      bool circle() const            { return textStyle().circle(); }
      Align align() const            { return textStyle().align(); }

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

      void insertText(const QString&);
      void insertSym(SymId);
      void selectAll();
      };


}     // namespace Ms
#endif
