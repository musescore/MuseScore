//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __PIANO_KEYBOARD_H__
#define __PIANO_KEYBOARD_H__

#include "piano.h"

namespace Ms {

class Staff;

static const int PIANO_KEYBOARD_WIDTH = 100;
static const int BLACK_KEY_WIDTH = PIANO_KEYBOARD_WIDTH * 9 / 14;
const int MAX_KEY_HEIGHT = 20;
const int MIN_KEY_HEIGHT = 8;
const int DEFAULT_KEY_HEIGHT = 14;
const int BEAT_WIDTH_IN_PIXELS = 50;
const double X_ZOOM_RATIO = 1.1;
const double X_ZOOM_INITIAL = 0.1;

      
//Alternative implementation with evenly spaced notes
class PianoKeyboard : public QWidget {
      Q_OBJECT

      static const QString pitchNames[];

      PianoOrientation _orientation;
      int _ypos;

      int noteHeight;
      int yRange;
      int curPitch;
      int curKeyPressed;
      Staff* _staff;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

   signals:
      void pitchChanged(int);
      void keyPressed(int pitch);
      void keyReleased(int pitch);

   public slots:
      void setYpos(int val);
      void setNoteHeight(int);
      void setPitch(int);

   public:
      PianoKeyboard(QWidget* parent = 0);
      Staff* staff() { return _staff; }
      void setStaff(Staff* staff);
      void setOrientation(PianoOrientation);
      };


} // namespace Ms
#endif

