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

namespace Ms {

class Element;
class Score;
class Note;
class Page;
enum class Grip : char;

//---------------------------------------------------------
//   MuseScoreView
//---------------------------------------------------------

class MuseScoreView {

   protected:
      Score* _score;

   public:
      MuseScoreView() {}
      Page* point2page(const QPointF&);
      Element* elementAt(const QPointF& p);
      const QList<Element*> elementsAt(const QPointF&);
      virtual Element* elementNear(QPointF) = 0;

      virtual void layoutChanged() {}
      virtual void dataChanged(const QRectF&) = 0;
      virtual void updateAll() = 0;

      virtual void moveCursor()          {}
      virtual void showLoopCursors(bool) {}

      virtual void adjustCanvasPosition(const Element* el, bool playBack) = 0;
      virtual void setScore(Score* s) { _score = s; }
      Score* score() const            { return _score; }
      virtual void removeScore() = 0;

      virtual void changeEditElement(Element*) = 0;
      virtual QCursor cursor() const = 0;
      virtual void setCursor(const QCursor&) = 0;
      virtual int gripCount() const = 0;
      virtual const QRectF& getGrip(Grip) const = 0;
      virtual void setDropRectangle(const QRectF&) = 0;
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote) = 0;
      virtual void cmdAddHairpin(bool) = 0;
      virtual void startEdit() = 0;
      virtual void startEdit(Element*, Grip startGrip) = 0;
      virtual void drawBackground(QPainter*, const QRectF&) const = 0;
      virtual void setDropTarget(const Element*) {}

      virtual void lyricsTab(bool /*back*/, bool /*end*/, bool /*moveOnly*/) {}
      virtual void lyricsReturn() {}
      virtual void lyricsEndEdit() {}
      virtual void lyricsUpDown(bool /*up*/, bool /*end*/)  {}
      virtual void lyricsMinus()  {}
      virtual void lyricsUnderscore()  {}
      };


}     // namespace Ms

#endif

