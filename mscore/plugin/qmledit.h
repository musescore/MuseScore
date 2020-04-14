//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __QML_EDIT_H__
#define __QML_EDIT_H__

#include "globals.h"

namespace Ms {

class JSHighlighter;

//---------------------------------------------------------
//   QmlEdit
//---------------------------------------------------------

class QmlEdit : public QPlainTextEdit {
      Q_OBJECT

      QWidget* lineNumberArea;
      JSHighlighter* hl;
      ScoreState mscoreState;
      QString pickBuffer;

      virtual void focusInEvent(QFocusEvent*);
      virtual void focusOutEvent(QFocusEvent*);
      void move(QTextCursor::MoveOperation);
      virtual void keyPressEvent(QKeyEvent*);
      void tab();
      void autoIndent();

   private slots:
      void updateLineNumberAreaWidth(int);
      void highlightCurrentLine();
      void updateLineNumberArea(const QRect&, int);
      void startOfLine() { move(QTextCursor::StartOfLine); }
      void endOfLine()   { move(QTextCursor::EndOfLine); }
      void upLine()      { move(QTextCursor::Up); }
      void downLine();
      void right()       { move(QTextCursor::Right); }
      void left()        { move(QTextCursor::Left);  }
      void rightWord()   { move(QTextCursor::NextWord); }
      void start()       { move(QTextCursor::Start); }
      void end()         { move(QTextCursor::End);   }
      void leftWord();
      void pick();
      void put();
      void delLine();
      void delWord();

   protected:
      void resizeEvent(QResizeEvent*);

   public:
      QmlEdit(QWidget* parent = 0);
      ~QmlEdit();
      void lineNumberAreaPaintEvent(QPaintEvent*);
      int lineNumberAreaWidth();
      enum ColorComponent { Normal, Comment, Number, String, Operator, Identifier,
         Keyword, BuiltIn, Marker };
      };

//---------------------------------------------------------
//   LineNumberArea
//---------------------------------------------------------

class LineNumberArea : public QWidget {
      Q_OBJECT
      QmlEdit* editor;

      QSize sizeHint() const {
            return QSize(editor->lineNumberAreaWidth(), 0);
            }
      void paintEvent(QPaintEvent* event) {
            editor->lineNumberAreaPaintEvent(event);
            }

   public:
      LineNumberArea(QmlEdit* parent) : QWidget(parent) { editor = parent; }
      };

//---------------------------------------------------------
//   JSBlockData
//---------------------------------------------------------

class JSBlockData : public QTextBlockUserData {
   public:
      QList<int> bracketPositions;
      };

//---------------------------------------------------------
//   JSHighlighter
//---------------------------------------------------------

class JSHighlighter : public QSyntaxHighlighter {
      QSet<QString> m_keywords;
      QSet<QString> m_knownIds;
      QHash<QmlEdit::ColorComponent, QColor> m_colors;
      QString m_markString;
      Qt::CaseSensitivity m_markCaseSensitivity;

   protected:
      void highlightBlock(const QString &text);

   public:
      JSHighlighter(QTextDocument *parent = 0);
      void setColor(QmlEdit::ColorComponent component, const QColor &color);
      void mark(const QString &str, Qt::CaseSensitivity caseSensitivity);
      QStringList keywords() const;
      void setKeywords(const QStringList &keywords);
      };


} // namespace Ms
#endif

