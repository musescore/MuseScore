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
class Slur;
class Note;
class Page;
class ChordRest;

enum class Grip : int;
enum class HairpinType : signed char;

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
      virtual Element* elementNear(QPointF) { return 0; }

      virtual void layoutChanged() {}
      virtual void dataChanged(const QRectF&) = 0;
      virtual void updateAll() = 0;

      virtual void moveCursor()          {}
      virtual void showLoopCursors(bool) {}

      virtual void adjustCanvasPosition(const Element*, bool /*playBack*/, int /*staffIdx*/ = -1) {};
      virtual void setScore(Score* s) { _score = s; }
      Score* score() const            { return _score; }
      virtual void removeScore() {};

      virtual void changeEditElement(Element*) {};
      virtual QCursor cursor() const { return QCursor(); }
      virtual void setCursor(const QCursor&) {};
      virtual void setDropRectangle(const QRectF&) {};
      virtual void cmdAddSlur(ChordRest*, ChordRest*, const Slur* /* slurTemplate */) {};
      virtual void startEdit(Element*, Grip /*startGrip*/) {};
      virtual void startNoteEntryMode() {};
      virtual void drawBackground(QPainter*, const QRectF&) const = 0;
      virtual void setDropTarget(const Element*) {}

      virtual void textTab(bool /*back*/) {}
      virtual void lyricsTab(bool /*back*/, bool /*end*/, bool /*moveOnly*/) {}
      virtual void lyricsReturn() {}
      virtual void lyricsEndEdit() {}
      virtual void lyricsUpDown(bool /*up*/, bool /*end*/)  {}
      virtual void lyricsMinus()  {}
      virtual void lyricsUnderscore()  {}

      virtual void onElementDestruction(Element*) {}

      virtual const QRect geometry() const = 0;
      };


}     // namespace Ms

#endif

