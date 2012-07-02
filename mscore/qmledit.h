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

//---------------------------------------------------------
//   QmlEdit
//---------------------------------------------------------

class QmlEdit : public QPlainTextEdit {
      Q_OBJECT

      QWidget* lineNumberArea;

   private slots:
      void updateLineNumberAreaWidth(int);
      void highlightCurrentLine();
      void updateLineNumberArea(const QRect&, int);

   protected:
      void resizeEvent(QResizeEvent*);

   public:
      QmlEdit(QWidget* parent = 0);
      void lineNumberAreaPaintEvent(QPaintEvent*);
      int lineNumberAreaWidth();
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

#endif

