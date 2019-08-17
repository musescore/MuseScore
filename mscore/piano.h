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

#ifndef __PIANO_H__
#define __PIANO_H__

namespace Ms {

static const int pianoWidth = 40;
static const int keyHeight = 13;

enum class PianoOrientation : char { HORIZONTAL, VERTICAL };

//---------------------------------------------------------
//   Piano
//---------------------------------------------------------

class Piano : public QWidget {
      Q_OBJECT

      PianoOrientation _orientation;
      double _ymag;
      int _ypos;

      int yRange;
      int curPitch;
      int curKeyPressed;

      static QPixmap* octave;
      static QPixmap* mk1;
      static QPixmap* mk2;
      static QPixmap* mk3;
      static QPixmap* mk4;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

      int pitch2y(int pitch) const;
      int y2pitch(int y) const;

   signals:
      void pitchChanged(int);
      void keyPressed(int pitch);
      void keyReleased(int pitch);

   public slots:
      void setYpos(int val);
      void setMag(double, double);
      void setPitch(int);

   public:
      Piano(QWidget* parent = 0);
      void setOrientation(PianoOrientation);
      };


} // namespace Ms
#endif

