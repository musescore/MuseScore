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

#ifndef __EASEINOUTCANVAS_H__
#define __EASEINOUTCANVAS_H__

namespace Ms {

//---------------------------------------------------------
//   EaseInOutCanvas
//---------------------------------------------------------

class EaseInOutCanvas : public QFrame {
      Q_OBJECT

      int m_easeIn;
      int m_easeOut;
      int m_nEvents;
      int m_pitchDelta;
      int m_bottomPitch;
      std::vector<int> m_events;

      virtual void paintEvent(QPaintEvent*) override;

   public:
      EaseInOutCanvas(QWidget* parent = nullptr);

      void setEaseInOut(const int easeIn, const int easeOut) { m_easeIn = easeIn; m_easeOut = easeOut; }

      /// number of lines to draw vertically
      int NbEvents() const                            { return m_nEvents;    }
      void setNbEvents(int nEvents)                   { m_nEvents = nEvents; }
      int pitchDelta() const                          { return m_pitchDelta; }
      void setPitchDelta(int pitchDelta)              { m_pitchDelta = pitchDelta; }
      const std::vector<int>& Events() const          { return m_events; }
      void setEvents(const std::vector<int>& events)  { m_events = events; }
      int bottomPitch() const                         { return m_bottomPitch; }
      void setBottomPitch(int bottomPitch)            { m_bottomPitch = bottomPitch; }

   signals:
      void canvasChanged();
      };

} // namespace Ms
#endif

