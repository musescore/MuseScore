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

#ifndef __BENDCANVAS_H__
#define __BENDCANVAS_H__

#include "libmscore/pitchvalue.h"

namespace Ms {

//---------------------------------------------------------
//   BendCanvas
///   As shown in in the inspector for bend properties
///   A graphic showing a list of points with lines between them
///   to describe a bend
//---------------------------------------------------------

class BendCanvas : public QFrame {
      Q_OBJECT
      QList<PitchValue> _points; /// the list of points describing the bend

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);

   public:
      BendCanvas(QWidget* parent = 0);
      const QList<PitchValue>& points() const { return _points; }
      QList<PitchValue>& points()             { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p; }

   signals:
      void bendCanvasChanged();
      };


} // namespace Ms
#endif

