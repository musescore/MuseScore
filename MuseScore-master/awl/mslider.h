//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#ifndef __AWLMSLIDER_H__
#define __AWLMSLIDER_H__

#include "volslider.h"

namespace Awl {

//---------------------------------------------------------
//   MeterSlider
//    volume slider with meter display
//---------------------------------------------------------

class MeterSlider : public VolSlider
      {
      Q_PROPERTY(int meterWidth READ meterWidth WRITE setMeterWidth)
      Q_PROPERTY(int channel READ channel WRITE setChannel)
      Q_OBJECT

      int _channel;
      std::vector<double> meterval;
      std::vector<double> meterPeak;
      int yellowScale, redScale;
      int _meterWidth;
      QPixmap onPm, offPm;  // cached pixmap values

      virtual void mousePressEvent(QMouseEvent*);
      virtual void paintEvent(QPaintEvent*);
      virtual void resizeEvent(QResizeEvent*);

   signals:
      void meterClicked();

   public slots:
      void resetPeaks();
      void setMeterVal(int channel, double value, double peak);

   public:
      MeterSlider(QWidget* parent = 0);
      void setChannel(int n);
      int channel() const       { return _channel; }
      int meterWidth() const    { return _meterWidth; }
      void setMeterWidth(int v) { _meterWidth = v; }
      virtual QSize sizeHint() const;
      };
}

#endif

