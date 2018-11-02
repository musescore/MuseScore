//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __KEYCANVAS_H__
#define __KEYCANVAS_H__

namespace Ms {

class Accidental;
class Clef;

//---------------------------------------------------------
//   KeyCanvas
//---------------------------------------------------------

class KeyCanvas : public QFrame {
      Q_OBJECT

      Accidental* dragElement;
      Accidental* moveElement;
      QTransform _matrix, imatrix;
      double extraMag;
      QList<Accidental*> accidentals;
      QPointF startMove;
      QPointF base;
      Clef* clef;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      void snap(Accidental*);

   private slots:
      void deleteElement();

   public:
      KeyCanvas(QWidget* parent = 0);
      void clear();
      const QList<Accidental*> getAccidentals() const { return accidentals; }
      };


} // namespace Ms
#endif

