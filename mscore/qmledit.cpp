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

#include "qmledit.h"


//---------------------------------------------------------
//   QmlEdit
//---------------------------------------------------------

QmlEdit::QmlEdit(QWidget* parent)
   : QPlainTextEdit(parent)
      {
      lineNumberArea = new LineNumberArea(this);

      connect(this, SIGNAL(blockCountChanged(int)),   SLOT(updateLineNumberAreaWidth(int)));
      connect(this, SIGNAL(updateRequest(QRect,int)), SLOT(updateLineNumberArea(QRect,int)));
      connect(this, SIGNAL(cursorPositionChanged()),  SLOT(highlightCurrentLine()));

      updateLineNumberAreaWidth(0);
      highlightCurrentLine();
      }

//---------------------------------------------------------
//   lineNumberAreaWidth
//---------------------------------------------------------

int QmlEdit::lineNumberAreaWidth()
      {
      int digits = 1;
      int max = qMax(1, blockCount());
      while (max >= 10) {
            max /= 10;
            ++digits;
            }
      int space = 6 + fontMetrics().width(QLatin1Char('9')) * digits;
      return space;
      }

//---------------------------------------------------------
//   updateLineNumberAreaWidth
//---------------------------------------------------------

void QmlEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
      {
      setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
      }

//---------------------------------------------------------
//   updateLineNumberArea
//---------------------------------------------------------

void QmlEdit::updateLineNumberArea(const QRect& rect, int dy)
      {
      if (dy)
            lineNumberArea->scroll(0, dy);
      else
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

      if (rect.contains(viewport()->rect()))
            updateLineNumberAreaWidth(0);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void QmlEdit::resizeEvent(QResizeEvent *e)
      {
      QPlainTextEdit::resizeEvent(e);

      QRect cr = contentsRect();
      lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
      }

//---------------------------------------------------------
//   highlightCurrentLine
//---------------------------------------------------------

void QmlEdit::highlightCurrentLine()
      {
      QList<QTextEdit::ExtraSelection> extraSelections;

      if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = QColor(Qt::darkGray);

            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
            }

      setExtraSelections(extraSelections);
      }

//---------------------------------------------------------
//   lineNumberAreaPaintEvent
//---------------------------------------------------------

void QmlEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
      {
      QPainter painter(lineNumberArea);
      painter.fillRect(event->rect(), Qt::lightGray);

      QTextBlock block = firstVisibleBlock();
      int blockNumber = block.blockNumber();
      int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
      int bottom = top + (int) blockBoundingRect(block).height();

      int w = lineNumberArea->width();
      int h = fontMetrics().height();
      while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                  QString number = QString::number(blockNumber + 1);
                  painter.setPen(Qt::black);
                  painter.drawText(0, top, w-3, h, Qt::AlignRight, number);
                  }

            block = block.next();
            top = bottom;
            bottom = top + (int) blockBoundingRect(block).height();
            ++blockNumber;
            }
       }
