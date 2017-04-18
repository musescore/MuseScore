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

#include "fastlog.h"
#include "mslider.h"

namespace Awl {

//---------------------------------------------------------
//   MeterSlider
//---------------------------------------------------------

MeterSlider::MeterSlider(QWidget* parent)
   : VolSlider(parent)
      {
      setAttribute(Qt::WA_NoSystemBackground, true);
      _channel    = 0;
      yellowScale = -16; //-10;
      redScale    = 0;
      _meterWidth = _scaleWidth * 3;
      setChannel(1);
      setMinimumHeight(50);
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize MeterSlider::sizeHint() const
  	{
      int w = _meterWidth + _scaleWidth + _scaleWidth + 30;
 	return orientation() == Qt::Vertical ? QSize(w, 200) : QSize(200, w);
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void MeterSlider::setChannel(int n)
      {
      if (n > _channel) {
            for (int i = _channel; i < n; ++i) {
                  meterval.push_back(0.0f);
                  meterPeak.push_back(0.0f);
                  }
            }
      _channel = n;
      }

//---------------------------------------------------------
//   setMeterVal
//---------------------------------------------------------

void MeterSlider::setMeterVal(int channel, double v, double peak)
      {
      bool mustRedraw = false;
      if (meterval[channel] != v) {
            meterval[channel] = v;
            mustRedraw = true;
            }
      if (peak != meterPeak[channel]) {
            meterPeak[channel] = peak;
            mustRedraw = true;
            }
      if (mustRedraw) {
            int kh = sliderSize().height();
            int mh = height() - kh;
            update(20, kh / 2, _meterWidth-1, mh);
            }
      }

//---------------------------------------------------------
//   resetPeaks
//    reset peak and overflow indicator
//---------------------------------------------------------

void MeterSlider::resetPeaks()
      {
      for (int i = 0; i < _channel; ++i)
            meterPeak[i]  = meterval[i];
      update();
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void MeterSlider::resizeEvent(QResizeEvent* /*ev*/)
      {
      int h  = height();
      int kh = sliderSize().height();
      int mh  = h - kh;
      int mw = _meterWidth / _channel;

      onPm  = QPixmap(mw, mh);
      offPm = QPixmap(mw, mh);

      double range = maxValue() - minValue();
      int h1 = mh - lrint((maxValue() - redScale) * mh / range);
      int h2 = mh - lrint((maxValue() - yellowScale) * mh / range);

      QColor yellowRed;
	yellowRed.setHsv(QColor(Qt::yellow).hue()-8,
		     QColor(Qt::yellow).saturation(),
		     QColor(Qt::yellow).value());
	QColor yellRedRed;
	yellRedRed.setHsv(QColor(Qt::yellow).hue()-16,
		      QColor(Qt::yellow).saturation(),
		      QColor(Qt::yellow).value());

	QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, mh));
	linearGrad.setColorAt(0, Qt::red);
	linearGrad.setColorAt(1-(double)(h1-5)/(double)mh, yellRedRed);
	linearGrad.setColorAt(1-(double)(h1-6)/(double)mh, yellowRed);
	linearGrad.setColorAt(1-(double)h2/(double)mh, Qt::yellow);
	linearGrad.setColorAt(1, Qt::green);

	QColor darkYellowRed;
	darkYellowRed.setHsv(QColor(Qt::darkYellow).hue()-8,
			 QColor(Qt::darkYellow).saturation(),
			 QColor(Qt::darkYellow).value());
	QColor darkYellRedRed;
	darkYellRedRed.setHsv(QColor(Qt::darkYellow).hue()-16,
			  QColor(Qt::darkYellow).saturation(),
			  QColor(Qt::darkYellow).value());
	QLinearGradient linearDarkGrad(QPointF(0, 0), QPointF(0, mh));
	linearDarkGrad.setColorAt(0, Qt::darkRed);
	linearDarkGrad.setColorAt(1-(double)(h1-5)/(double)mh, darkYellRedRed);
	linearDarkGrad.setColorAt(1-(double)(h1-6)/(double)mh, darkYellowRed);
	linearDarkGrad.setColorAt(1-(double)h2/(double)mh, Qt::darkYellow);
	linearDarkGrad.setColorAt(1, Qt::darkGreen);

      QPainter p;
      p.begin(&onPm);
      p.fillRect(0, 0, mw, mh, linearGrad);
      p.end();
      p.begin(&offPm);
	p.fillRect(0, 0, mw, mh, linearDarkGrad);
      p.end();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void MeterSlider::paintEvent(QPaintEvent* ev)
      {
      int h  = height();
      int kh = sliderSize().height();
      int mh = h - kh;

      double range = maxValue() - minValue();
      int ppos     = int(mh * (_value - minValue()) / range);
      if (_invert)
            ppos = mh - ppos;

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, false);

      //---------------------------------------------------
      //    draw meter
      //---------------------------------------------------

      int mw = _meterWidth / _channel;
      int x  = 18;
      int y1 = kh / 2;
      int y3 = h - y1;

      p.setPen(QPen(Qt::white, 2));

      for (int i = 0; i < _channel; ++i) {
            int h = mh - (lrint(fast_log10(meterval[i]) * -20.0f * mh / range));
            if (h < 0)
                  h = 0;
            else if (h > mh)
                  h = mh;

	      p.drawPixmap(x, y1+mh-h, mw, h,    onPm,  0, mh-h, mw, h);
	      p.drawPixmap(x, y1,      mw, mh-h, offPm, 0, 0,    mw, mh-h);

            //---------------------------------------------------
            //    draw peak line
            //---------------------------------------------------

            h = mh - (lrint(fast_log10(meterPeak[i]) * -20.0f * mh / range));
            if (h > mh)
                  h = mh;
	      if (h > 0)
	            p.drawLine(x, y3-h, x+mw, y3-h);

            x += mw;
            }
      x += 4;

      // optimize common case:
      if (ev->rect() == QRect(20, kh/2, _meterWidth-1, mh))
            return;

      QColor sc(isEnabled() ? _scaleColor : Qt::gray);
      QColor svc(isEnabled() ? _scaleValueColor : Qt::gray);
      p.setBrush(svc);

      //---------------------------------------------------
      //    draw scale
      //---------------------------------------------------

      int y2 = h - (ppos + y1);
      p.fillRect(x, y1, _scaleWidth, y2-y1, sc);
      p.fillRect(x, y2, _scaleWidth, y3-y2, svc);

      //---------------------------------------------------
      //    draw tick marks
      //---------------------------------------------------

  	QFont f(p.font());
   	f.setPointSize(6);
   	p.setFont(f);
      p.setPen(QPen(Qt::darkGray, 2));
   	QFontMetrics fm(f);
      int xt = 20 - fm.width("00") - 5;

      QString s;
   	for (int i = 10; i < 70; i += 10) {
      	h  = y1 + lrint(i * mh / range);
         	s.setNum(i - 10);
  		p.drawText(xt,  h - 3, s);
		p.drawLine(15, h, 20, h);
         	}

      //---------------------------------------------------
      //    draw slider
      //---------------------------------------------------

      x  += _scaleWidth/2;
      p.setPen(QPen(svc, 0));
      p.translate(QPointF(x, y2));
      p.setRenderHint(QPainter::Antialiasing, true);
      p.drawPath(*points);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void MeterSlider::mousePressEvent(QMouseEvent* ev)
      {
      if (ev->pos().x() < _meterWidth) {
            emit meterClicked();
            return;
            }
      VolSlider::mousePressEvent(ev);
      }
}

