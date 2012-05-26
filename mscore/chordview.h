//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __CHORDVIEW_H__
#define __CHORDVIEW_H__

#include "libmscore/pos.h"

class Staff;
class Chord;
class Note;
class NoteEvent;
class ChordItem;
class ChordView;

enum { GripTypeItem = QGraphicsItem::UserType, ChordTypeItem };

//---------------------------------------------------------
//   GripItem
//---------------------------------------------------------

class GripItem : public QGraphicsRectItem {
      ChordItem* _event;
      int _gripType;          // 0 - start grip   1 - end grip
      ChordView* _view;


      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

   protected:
      virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);

   public:
      GripItem(int gripType, ChordView*);
      virtual int type() const    { return GripTypeItem; }
      ChordItem* event() const    { return _event; }
      void setEvent(ChordItem* e) { _event = e; }
      };

//---------------------------------------------------------
//   ChordItem
//---------------------------------------------------------

class ChordItem : public QGraphicsRectItem {
      Note*      _note;
      NoteEvent* _event;
      bool       _current;

      virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = 0);
      virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);

   public:
      ChordItem(Note*, NoteEvent*);
      virtual int type() const { return ChordTypeItem; }
      NoteEvent* event() const { return _event; }
      Note* note() const       { return _note;  }
      bool current() const     { return _current; }
      void setCurrent(bool v);
      };

//---------------------------------------------------------
//   ChordView
//---------------------------------------------------------

class ChordView : public QGraphicsView {
      Q_OBJECT

      Chord* chord;
      Note* curNote;
      int _locator;
      int _pos;
      QGraphicsLineItem* locatorLine;
      int ticks;
      int magStep;
      GripItem* lg;
      GripItem* rg;
      ChordItem* curEvent;

      bool _evenGrid;

      virtual void drawBackground(QPainter*, const QRectF& rect);

   protected:
      virtual void wheelEvent(QWheelEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void leaveEvent(QEvent*);
      virtual void mousePressEvent(QMouseEvent*);

   signals:
      void magChanged(double, double);
      void xposChanged(int);
      void pitchChanged(int);
      void posChanged(int);

   protected slots:
      void deleteItem();

   public slots:
      void moveLocator();
      void selectionChanged();

   public:
      ChordView();
      void setChord(Chord*);
      void ensureVisible(int tick);
      QList<QGraphicsItem*> items() { return scene()->selectedItems(); }
      bool evenGrid() const         { return _evenGrid; }
      void setEvenGrid(bool val)    { _evenGrid = val;  }
      GripItem* rightGrip() const   { return rg;       }
      void setCurItem(ChordItem*);

      static int pos2pix(int pos);
      static int pix2pos(int pix);
      static int y2pitch(int pix);
      static int pitch2y(int pitch);
      };

#endif
