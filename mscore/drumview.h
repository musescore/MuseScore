//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __DRUMVIEW_H__
#define __DRUMVIEW_H__

#include "libmscore/pos.h"

namespace Ms {

class Staff;
class Score;
class Note;

//---------------------------------------------------------
//   DrumItem
//---------------------------------------------------------

class DrumItem : public QGraphicsPolygonItem {
      Note* _note;

      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

   public:
      DrumItem(Note*);
      };

//---------------------------------------------------------
//   DrumView
//---------------------------------------------------------

class DrumView : public QGraphicsView {
      Q_OBJECT

      Staff* staff;
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

   public:
      DrumView();
      void setStaff(Staff*, Pos* locator);
      void ensureVisible(int tick);
      QList<QGraphicsItem*> items() { return scene()->selectedItems(); }
      };



} // namespace Ms
#endif
