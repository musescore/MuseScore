//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

class MuseScoreView;
struct SymCode;

//---------------------------------------------------------
//   TCursor
//---------------------------------------------------------

class TCursor {
   public:
      int selectLine;         // start of selection
      int selectColumn;

      int line;
      int column;

      bool hasSelection() const { return (selectLine != line) || (selectColumn != column); }
      };

//---------------------------------------------------------
//   TLine
//---------------------------------------------------------

struct TLine {
      QString text;
      QPointF pos;

      TLine() {}
      TLine(const QString& s) { text = s; }
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
      QString& curLine();
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
      const QString& firstLine() const;

      void startEdit(MuseScoreView*, const QPointF&);
      void endEdit();
      bool edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers, const QString&);
      void addChar(int code);
      bool deletePreviousChar();
      bool deleteChar();
      bool movePosition(QTextCursor::MoveOperation op,
         QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
      bool setCursor(const QPointF& p,
         QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
      void moveCursorToEnd()   { movePosition(QTextCursor::Start); };
      void moveCursorToStart() { movePosition(QTextCursor::End); };

      QString selectedText() const;

      void insertText(const QString&);
      };

#endif
