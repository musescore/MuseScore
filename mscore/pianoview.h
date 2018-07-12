//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PIANOVIEW_H__
#define __PIANOVIEW_H__

#include "libmscore/pos.h"

namespace Ms {

class Score;
class Staff;
class Chord;
class ChordRest;
class Note;
class NoteEvent;
class PianoView;

enum class NoteSelectType {
      REPLACE = 0,
      XOR,
      ADD,
      SUBTRACT,
      FIRST
      };

enum class DragStyle {
    NONE = 0,
    SELECTION_RECT,
    MOVE_NOTES
};
      
//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

class PianoItem {
      Note* _note;
      PianoView* _pianoView;
      
      void paintNoteBlock(QPainter* painter, NoteEvent* evt);
      QRect boundingRectTicks(NoteEvent* evt);
      QRect boundingRectPixels(NoteEvent* evt);
      bool intersectsBlock(int startTick, int endTick, int highPitch, int lowPitch, NoteEvent* evt);
      
   public:
      PianoItem(Note*, PianoView*);
      ~PianoItem() {}
      Note* note() { return _note; }
      void paint(QPainter* painter);
      bool intersects(int startTick, int endTick, int highPitch, int lowPitch);
      
      QRect boundingRect();
      
      NoteEvent* getTweakNoteEvent();
      };

//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

class PianoView : public QGraphicsView {
      Q_OBJECT

      Staff* _staff;
      Chord* chord;
      
      Pos trackingPos;  //Track mouse position
      Pos* _locator;
      int ticks;
      TType _timeType;
      int _noteHeight;
      qreal _xZoom;
      
      bool _playEventsView;
      bool mouseDown;
      bool dragStarted;
      QPointF mouseDownPos;
      QPointF lastMousePos;
      DragStyle dragStyle;
      int lastDragPitch;
      bool inProgressUndoEvent;
      
      QList<PianoItem*> noteList;

      virtual void drawBackground(QPainter* painter, const QRectF& rect);

      void addChord(Chord* chord, int voice);
      void updateBoundingSize();
      void clearNoteData();
      void selectNotes(int startTick, int endTick, int lowPitch, int highPitch, NoteSelectType selType);

      QAction* getAction(const char* id);
//      void splitCR(Score* score, ChordRest* chordHead, Fraction frac);

   protected:
      virtual void wheelEvent(QWheelEvent* event);
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

   signals:
      void xZoomChanged(qreal);
      void noteHeightChanged(int);
      void pitchChanged(int);
      void trackingPosChanged(const Pos&);
      void selectionChanged();

   public slots:
      void moveLocator(int);
      void updateNotes();

   public:
      PianoView();
      ~PianoView();
      Staff* staff() { return _staff; }
      void setStaff(Staff*, Pos* locator);
      void ensureVisible(int tick);
      int noteHeight() { return _noteHeight; }
      qreal xZoom() { return _xZoom; }
      QList<QGraphicsItem*> items() { return scene()->selectedItems(); }

      int pixelXToTick(int pixX);
      int tickToPixelX(int tick);
      int pixelYToPitch(int pixY) { return (int)floor(128 - pixY / (qreal)_noteHeight); }
      
      PianoItem* pickNote(int tick, int pitch);

      QList<PianoItem*> getSelectedItems();
      QList<PianoItem*> getItems();
      
      bool playEventsView() { return _playEventsView; }

      };


} // namespace Ms
#endif

