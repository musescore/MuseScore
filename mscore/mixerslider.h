//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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


#ifndef __MIXERSLIDER_H__
#define __MIXERSLIDER_H__

namespace Ms {

class MixerSlider : public QSlider
      {
      Q_OBJECT

      bool panning = false;         // used for pan-style where filling does not make sense
      bool secondary = false;       // visual indicator when the slider is in its "secondary" mode

      QColor grooveFillColor();
      QColor grooveOutlineColor();
      QColor grooveBackgroundColor();

public:
      explicit MixerSlider(QWidget *parent = nullptr);

      void paintEvent(QPaintEvent *ev);

      void setPanMode(bool on) { panning = on; }
      bool panMode() { return panning; }
      void setSecondaryMode (bool on);                // will force a repaint
      bool secondaryMode() { return secondary; }

      void mouseDoubleClickEvent(QMouseEvent* mouseEvent);
      };

} // namespace Ms
#endif /* __MIXEROPTIONS_H__ */
