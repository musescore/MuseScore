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

#include "mixerslider.h"
#include <QStyleOptionSlider>
#include <QPainter>
#include <QStylePainter>
#include <QApplication>

namespace Ms {

MixerSlider::MixerSlider(QWidget *parent) :
      QSlider(parent)
      {
      

      }



void MixerSlider::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
      {
      if (panning) {
            setSliderPosition(0);
            return;
            }

      QSlider::mouseDoubleClickEvent(mouseEvent);

}

void MixerSlider::setSecondaryMode(bool on)
{
      secondary = on;
      repaint();
}

void MixerSlider::setPanMode(bool on)
{
      panning = on;
      setMinimum(panning ? -63 : 0);
      setMaximum(panning ? 63 : 127);
}


// Groove background color is dervied from the palette's button color
QColor MixerSlider::grooveBackgroundColor() {

      QColor buttonColor = QApplication::palette().color(QPalette::Button);
      QColor grooveBackgroundColor;
      grooveBackgroundColor.setHsv(buttonColor.hue(),
                                   qMin(255, static_cast<int>(buttonColor.saturation())),
                                   qMin(255, static_cast<int>(buttonColor.value()*0.9)));
      return grooveBackgroundColor;
}

// Groove outline color is dervied from the palette's button color
QColor MixerSlider::grooveOutlineColor() {

      QColor buttonColor = QApplication::palette().color(QPalette::Button);

      QColor outlineColor;
      outlineColor.setHsv(buttonColor.hue(),
                          qMin(255, static_cast<int>(buttonColor.saturation())),
                          qMin(255, static_cast<int>(buttonColor.value()*0.6)));

      return outlineColor;

}


// Slider fill is derived from the palette's highlight color
// In secondary mode the slider fill is derived from the "opposite" of
// the highlight color. This should make the color reasonable even when
// themes change the highlight color.
QColor MixerSlider::grooveFillColor()
{
      QColor secondaryColor = QApplication::palette().color(QPalette::Highlight);

      if (secondary) {
            secondaryColor.setHsv(secondaryColor.hue() + 180 % 360,
                                  secondaryColor.saturation(),
                                  secondaryColor.value());
      }


      QColor grooveFillColor;
      grooveFillColor.setHsv(secondaryColor.hue(),
                             qMin(255, static_cast<int>(secondaryColor.saturation() * 3)),
                             qMin(255, static_cast<int>(secondaryColor.value() * 3)));

      return grooveFillColor;
}

QLinearGradient gradientForRect(QRect rect, QColor color)
      {
      QLinearGradient gradient;
      gradient.setStart(rect.center().x(), rect.top());
      gradient.setFinalStop(rect.center().x(), rect.bottom());
      gradient.setColorAt(0, color.darker(110));
      gradient.setColorAt(1, color.lighter(110));
      return gradient;
      }

void MixerSlider::paintEvent(QPaintEvent *ev)
      {

      QStyleOptionSlider opt;
      initStyleOption(&opt);

      QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
      QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
      grooveRect = grooveRect.adjusted(1, 1, -2, -2);


      QStylePainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.setPen(QPen(grooveOutlineColor())); // QApplication::palette().color(QPalette::Text)));

      if (panning) {
            int grooveCentre = grooveRect.width() / 2;
            int handlePosition = ((handleRect.left() - grooveRect.left()) + handleRect.width() / 2) - grooveCentre;

            int cursor = 0;
            int segmentWidth = handlePosition < 0 ? grooveCentre + handlePosition : grooveRect.width() / 2;
            QRect unfilledLeft = QRect(grooveRect.left() + cursor, grooveRect.top(), segmentWidth, grooveRect.height());

            cursor = cursor + segmentWidth;
            segmentWidth = abs(handlePosition);
            QRect filledCentre = QRect(grooveRect.left() + cursor, grooveRect.top(), segmentWidth, grooveRect.height());

            cursor = cursor + segmentWidth;
            segmentWidth = grooveRect.width() - cursor;

            QRect unfilledRight = QRect(grooveRect.left() + cursor, grooveRect.top(), grooveCentre - handlePosition, grooveRect.height());

            QLinearGradient leftGradient = gradientForRect(unfilledLeft, grooveBackgroundColor());
            QLinearGradient centreGradient = gradientForRect(filledCentre, grooveFillColor());
            QLinearGradient rightGradient = gradientForRect(unfilledRight, grooveBackgroundColor());

            painter.setBrush(leftGradient);
            painter.drawRect(unfilledLeft);

            painter.setBrush(centreGradient);
            painter.drawRect(filledCentre);

            painter.setBrush(rightGradient);
            painter.drawRect(unfilledRight);
            }
      else {

            int handleMargin = handleRect.width() / 2;

            QRect lowerGroove = QRect(grooveRect.left(), grooveRect.top(), handleMargin + handleRect.left(), grooveRect.height());
            QRect upperGroove = QRect(handleRect.right() - handleMargin, grooveRect.top(), (grooveRect.right() - handleRect.right()) + handleMargin, grooveRect.height());

            QLinearGradient lowerGradient = gradientForRect(lowerGroove, grooveFillColor());
            QLinearGradient upperGradient = gradientForRect(upperGroove, grooveBackgroundColor());

            painter.setBrush(lowerGradient);
            painter.drawRect(lowerGroove);

            painter.setBrush(upperGradient);
            painter.drawRect(upperGroove);
      }

      // draw the slider handle and tick marks
      opt.subControls = QStyle::SC_SliderHandle | QStyle::SC_SliderTickmarks;
      painter.drawComplexControl(QStyle::CC_Slider, opt);

      }

} // namespace Ms
