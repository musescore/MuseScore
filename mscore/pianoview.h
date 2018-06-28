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

const int PianoItemType = QGraphicsItem::UserType + 1;

//const QColor noteDeselected = Qt::blue;
const QColor noteDeselected = QColor(27, 198, 156);
const QColor noteSelected = Qt::yellow;

//const QColor colPianoBg(0x71, 0x8d, 0xbe);
//const QColor colPianoBg(85, 106, 143);
const QColor colPianoBg(54, 54, 54);

const QColor noteDeselectedBlack = noteDeselected.darker(150);
const QColor noteSelectedBlack = noteSelected.darker(150);

//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

class PianoItem : public QGraphicsRectItem {
      Note*      _note;
      NoteEvent* _event;
      PianoView* _pianoView;
      bool isBlack;
      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

   public:
      PianoItem(Note*, NoteEvent*, PianoView*);
      virtual ~PianoItem() {}
      virtual int type() const { return PianoItemType; }
      Note* note()       { return _note; }
      NoteEvent* event() { return _event; }
//      QRectF updateValues();
      void updateValues();
      };

//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

class PianoView : public QGraphicsView {
      Q_OBJECT

      Staff* staff;
      Chord* chord;
      Pos pos;
      Pos* _locator;
      QGraphicsLineItem* locatorLines[3];
      int ticks;
      TType _timeType;
      int magStep;
      int _noteHeight;
      qreal _xZoom;

      virtual void drawBackground(QPainter* painter, const QRectF& rect);

//      int y2pitch(int y) const;
//      Pos pix2pos(int x) const;
//      int pos2pix(const Pos& p) const;
      void createLocators();
      void addChord(Chord* chord);
      void updateBoundingSize();
      
   protected:
      virtual void wheelEvent(QWheelEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

   signals:
      void magChanged(double, double);
      void xZoomChanged(qreal);
      void noteHeightChanged(int);
      void xposChanged(int);
      void pitchChanged(int);
      void posChanged(const Pos&);

   public slots:
      void moveLocator(int);
      void updateNotes();

   public:
      PianoView();
      void setStaff(Staff*, Pos* locator);
      void scrollToTick(int tick);
      int noteHeight() { return _noteHeight; }
      qreal xZoom() { return _xZoom; }
      QList<QGraphicsItem*> items() { return scene()->selectedItems(); }

      int pixelXToTick(int pixX);
      int tickToPixelX(int tick);

      };


} // namespace Ms
#endif

