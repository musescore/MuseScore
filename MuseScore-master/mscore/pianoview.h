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

const int PianoItemType = QGraphicsItem::UserType + 1;

//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

class PianoItem : public QGraphicsRectItem {
      Note*      _note;
      NoteEvent* _event;
      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

   public:
      PianoItem(Note*, NoteEvent*);
      virtual ~PianoItem() {}
      virtual int type() const { return PianoItemType; }
      Note* note()       { return _note; }
      NoteEvent* event() { return _event; }
      QRectF updateValues();
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

      virtual void drawBackground(QPainter* painter, const QRectF& rect);

      int y2pitch(int y) const;
      Pos pix2pos(int x) const;
      int pos2pix(const Pos& p) const;
      void createLocators();
      void addChord(Chord* chord);

   protected:
      virtual void wheelEvent(QWheelEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

   signals:
      void magChanged(double, double);
      void xposChanged(int);
      void pitchChanged(int);
      void posChanged(const Pos&);

   public slots:
      void moveLocator(int);
      void updateNotes();

   public:
      PianoView();
      void setStaff(Staff*, Pos* locator);
      void ensureVisible(int tick);
      QList<QGraphicsItem*> items() { return scene()->selectedItems(); }
      };


} // namespace Ms
#endif

