//=============================================================================
//  MusE Score
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

#ifndef __RULER_H__
#define __RULER_H__

#include "libmscore/pos.h"

namespace Ms {

class Score;

static const int rulerHeight = 28;

//---------------------------------------------------------
//   Ruler
//---------------------------------------------------------

class Ruler : public QWidget {
      Q_OBJECT

      Score* _score;
      Pos _cursor;
      Pos* _locator;

      int magStep;
      double _xmag;
      int _xpos;
      TType _timeType;
      QFont _font1, _font2;

      static QPixmap* markIcon[3];

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

      Pos pix2pos(int x) const;
      int pos2pix(const Pos& p) const;
      void moveLocator(QMouseEvent*);

   signals:
      void posChanged(const Pos&);
      void locatorMoved(int idx, const Pos&);

   public slots:
      void setXpos(int);
      void setMag(double xmag, double ymag);
      void setPos(const Pos&);

   public:
      Ruler(QWidget* parent = 0);
      void setScore(Score*, Pos* locator);
      double xmag() const { return _xmag; }
      int xpos() const { return _xpos; }
      };


} // namespace Ms
#endif

