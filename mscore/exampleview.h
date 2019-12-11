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
class Chord;
class Icon;
enum class Grip : int;

//---------------------------------------------------------
//   ExampleView
//---------------------------------------------------------

class ExampleView : public QFrame, public MuseScoreView {
      Q_OBJECT

      QTransform _matrix, imatrix;
      QColor _fgColor;
      QPixmap* _fgPixmap;
      Element* dragElement = 0;
      const Element* dropTarget = 0;      ///< current drop target during dragMove
      QRectF dropRectangle;               ///< current drop rectangle during dragMove
      QLineF dropAnchor;                  ///< line to current anchor point during dragMove

      QStateMachine* sm;
      QPointF startMove;

      void drawElements(QPainter& painter, const QList<Element*>& el);
      void setDropTarget(const Element* el);

      virtual void paintEvent(QPaintEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      void constraintCanvas(int *dxx);
      virtual QSize sizeHint() const;

   signals:
      void noteClicked(Note*);
      void beamPropertyDropped(Chord*, Icon*);

   public:
      ExampleView(QWidget* parent = 0);
      ~ExampleView();
      void resetMatrix();
      virtual void layoutChanged();
      virtual void dataChanged(const QRectF&);
      virtual void updateAll();
      virtual void adjustCanvasPosition(const Element* el, bool playBack);
      virtual void setScore(Score*);
      virtual void removeScore();

      virtual void changeEditElement(Element*);
      virtual QCursor cursor() const;
      virtual void setCursor(const QCursor&);
      virtual void setDropRectangle(const QRectF&);
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote);
      virtual Element* elementNear(QPointF);
      virtual void drawBackground(QPainter*, const QRectF&) const;
      void dragExampleView(QMouseEvent* ev);
      virtual const QRect geometry() const override { return QFrame::geometry(); }
      };

//---------------------------------------------------------
//   DragTransitionExampleView
//---------------------------------------------------------

class DragTransitionExampleView : public QEventTransition
      {
      ExampleView* canvas;

   protected:
      virtual void onTransition(QEvent* e);

   public:
      DragTransitionExampleView(ExampleView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

} // namespace Ms
#endif

