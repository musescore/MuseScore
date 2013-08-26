//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __EXAMPLEVIEW_H__
#define __EXAMPLEVIEW_H__

#include "libmscore/mscoreview.h"

namespace Ms {

class Element;
class Score;
class Note;

//---------------------------------------------------------
//   ExampleView
//---------------------------------------------------------

class ExampleView : public QFrame, public MuseScoreView {
      Q_OBJECT

      QTransform _matrix, imatrix;
      QColor _bgColor;
      QColor _fgColor;
      QPixmap* bgPixmap;
      QPixmap* fgPixmap;
      Element* dragElement = 0;
      const Element* dropTarget = 0;      ///< current drop target during dragMove
      QRectF dropRectangle;               ///< current drop rectangle during dragMove
      QLineF dropAnchor;                  ///< line to current anchor point during dragMove

      void drawElements(QPainter& painter, const QList<Element*>& el);
      void setDropTarget(const Element* el);

      virtual void paintEvent(QPaintEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual QSize sizeHint() const { return QSize(1000, 80); }
      virtual void mousePressEvent(QMouseEvent*);

   signals:
      void noteClicked(Note*);

   public:
      ExampleView(QWidget* parent = 0);
      virtual void layoutChanged();
      virtual void dataChanged(const QRectF&);
      virtual void updateLoopCursors();
      virtual void showLoopCursors() {}
      virtual void hideLoopCursors() {}
      virtual void updateAll();
      virtual void moveCursor();
      virtual void adjustCanvasPosition(const Element* el, bool playBack);
      virtual void setScore(Score*);
      virtual void removeScore();

      virtual void changeEditElement(Element*);
      virtual QCursor cursor() const;
      virtual void setCursor(const QCursor&);
      virtual int gripCount() const;
      virtual const QRectF& getGrip(int) const;
      virtual void setDropRectangle(const QRectF&);
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote);
      virtual void startEdit();
      virtual void startEdit(Element*, int startGrip);
      virtual Element* elementNear(QPointF);
      virtual void drawBackground(QPainter*, const QRectF&) const;
      };


} // namespace Ms
#endif

