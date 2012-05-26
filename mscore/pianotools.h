//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __PIANOTOOLS_H__
#define __PIANOTOOLS_H__

class HPiano;

//---------------------------------------------------------
//   PianoKeyItem
//---------------------------------------------------------

class PianoKeyItem : public QGraphicsPathItem {
      int type;
      int pitch;
      bool pressed;
      HPiano* piano;

      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
      virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
      virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);

   public:
      PianoKeyItem(HPiano* , int pitch);
      void setType(int val);
      };

//---------------------------------------------------------
//   HPiano
//---------------------------------------------------------

class HPiano : public QGraphicsView {
      Q_OBJECT
      int _firstKey;
      int _lastKey;
      int _currentKey;
      QList<PianoKeyItem*> keys;
      qreal scaleVal;
      virtual void wheelEvent(QWheelEvent*);
      void setScale(qreal);

   signals:
      void keyPressed(int pitch, bool chord);

   public:
      HPiano(QWidget* parent = 0);
      friend class PianoKeyItem;
      virtual QSize sizeHint() const;
      };

//---------------------------------------------------------
//   PianoTools
//---------------------------------------------------------

class PianoTools : public QDockWidget {
      Q_OBJECT

      virtual void closeEvent(QCloseEvent*);

   signals:
      void pianoVisible(bool);

   signals:
      void keyPressed(int pitch, bool ctrl);

   public:
      PianoTools(QWidget* parent = 0);
      };

#endif

