//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __MSCOREVIEW_H__
#define __MSCOREVIEW_H__

class Element;
class Score;
class Note;

//---------------------------------------------------------
//   MuseScoreView
//---------------------------------------------------------

class MuseScoreView {
   public:
      MuseScoreView() {}
      virtual void layoutChanged() {}
      virtual void dataChanged(const QRectF&) = 0;
      virtual void updateAll() = 0;
      virtual void moveCursor() = 0;
      virtual void adjustCanvasPosition(const Element* el, bool playBack) = 0;
      virtual void setScore(Score*) = 0;
      virtual void removeScore() = 0;

      virtual void changeEditElement(Element*) = 0;
      virtual QCursor cursor() const = 0;
      virtual void setCursor(const QCursor&) = 0;
      virtual int gripCount() const = 0;
      virtual const QRectF& getGrip(int) const = 0;
      virtual void setDropRectangle(const QRectF&) = 0;
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote) = 0;
      virtual void startEdit() = 0;
      virtual void startEdit(Element*, int startGrip) = 0;
      virtual Element* elementNear(QPointF) = 0;
      virtual void drawBackground(QPainter*, const QRectF&) const = 0;
      };

#endif



