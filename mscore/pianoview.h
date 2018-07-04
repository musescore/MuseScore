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

class Staff;
class Chord;
class Note;
class NoteEvent;
class PianoView;

enum class NoteSelectType {
      REPLACE = 0,
      XOR,
      ADD,
      SUBTRACT
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
      Note*      _note;
      NoteEvent* _event;
      PianoView* _pianoView;
      bool isBlack;
      QRect rect;

   public:
      PianoItem(Note*, NoteEvent*, PianoView*);
      ~PianoItem() {}
      Note* note()       { return _note; }
      NoteEvent* event() { return _event; }
      int startTick();
      int tickLength();
      int pitch();
      void updateValues();
      void paint(QPainter* painter);
      
      QRect boundingRect() { return rect; }
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
      QGraphicsLineItem* locatorLines[3];
      int ticks;
      TType _timeType;
      int _noteHeight;
      qreal _xZoom;
      
      bool mouseDown;
      bool dragStarted;
      QPointF mouseDownPos;
      QPointF lastMousePos;
      DragStyle dragStyle;
      int lastDragPitch;
      bool tempUndoEvent;
      
      QList<PianoItem*> noteList;

      virtual void drawBackground(QPainter* painter, const QRectF& rect);

      void createLocators();
      void addChord(Chord* chord);
      void updateBoundingSize();
      void clearNoteData();
      void selectNotes(int startTick, int endTick, int lowPitch, int highPitch, NoteSelectType selType);

      
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

      };


} // namespace Ms
#endif

