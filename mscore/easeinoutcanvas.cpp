//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2019 Werner Schweer and others
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

#include "easeinoutcanvas.h"
#include "..\libmscore\easeInOut.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   EaseInOutCanvas
//---------------------------------------------------------

EaseInOutCanvas::EaseInOutCanvas(QWidget* parent)
   : QFrame(parent),
     m_easeIn(0.0),
     m_easeOut(0.0),
     m_nEvents(25),
     m_pitchDelta(0)
      {
      setFrameStyle(QFrame::NoFrame);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void EaseInOutCanvas::paintEvent(QPaintEvent* ev)
      {
      // not qreal here, even though elsewhere yes,
      // because width and height return a number of pixels,
      // hence integers.
      const int w = width();
      const int h = height();
      const int border = 1;

      const qreal graphWidth = static_cast<qreal>(w - 2 * border);
      const qreal graphHeight = static_cast<qreal>(h - 2 * border);
      const qreal nbEvents = static_cast<qreal>(m_nEvents);
      const qreal pitchDelta = static_cast<qreal>(m_pitchDelta);

      // let half a column of margin around
      const qreal leftPos = static_cast<qreal>(border);           // also left margin
      const qreal topPos = leftPos;                               // also top margin
      const qreal bottomPos = static_cast<qreal>(h - border);     // bottom end position of graph
      const qreal rightPos = static_cast<qreal>(w - border);      // right end position of graph

      EaseInOut eio(static_cast<qreal>(m_easeIn) / 100.0, static_cast<qreal>(m_easeOut) / 100.0);

      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing, preferences.getBool(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING));

      painter.fillRect(rect(), QApplication::palette().color(QPalette::Window).lighter());
      QPen pen = painter.pen();
      pen.setWidth(1);

      QColor eventLinesColor(Qt::gray);
      eventLinesColor.setAlphaF(0.5);
      QColor pitchLinesColor(Qt::gray);
      pitchLinesColor.setAlphaF(0.25);
      QColor borderLinesColor, warpLineColor, pitchFillColor, pitchGraphColor;
      if (preferences.isThemeDark()) {
            pitchFillColor.setRgbF(0.078, 0.284, 0.463);
            pitchGraphColor.setRgbF(0.125, 0.455, 0.741);
            warpLineColor.setRgbF(0.574, 0.368, 0.255);
            borderLinesColor.setRgbF(0.891, 0.932, 0.968);
      }
      else {
            pitchFillColor.setRgbF(0.891, 0.932, 0.968);
            pitchGraphColor.setRgbF(0.125, 0.455, 0.741);
            warpLineColor.setRgbF(0.938, 0.691, 0.556);
            borderLinesColor.setRgbF(0.016, 0.057, 0.093);
      }

      // this lambda takes as input a pitch value, and determines where what are its x and y coordinates
      auto getPosition = [this, graphWidth, graphHeight, leftPos, bottomPos](const QPointF& p) -> QPointF {
            return { leftPos + p.x() * graphWidth, bottomPos - p.y() * graphHeight };
      };

      // Draw the pitches staircase graph
      if (m_events.size() > 1) {
            pen.setWidth(3);
            pen.setColor(pitchGraphColor);
            painter.setPen(pen);
            // draw line between points
            qreal offset = m_events[m_events.size()-1] < 0 ? 1.0 : 0.0;
            QPointF lastPoint = getPosition({0.0, offset + 0.0});
            for (int i = 1; i < m_events.size(); i++) {
                  qreal xPos = eio.XfromY(static_cast<qreal>(i) / nbEvents);
                  qreal yPos = static_cast<qreal>(m_events[i]) / static_cast<qreal>(pitchDelta);
                  QPointF currentPoint = getPosition({xPos, offset + yPos});
                  painter.fillRect(lastPoint.x(), bottomPos, (currentPoint.x() - lastPoint.x()) + 1, lastPoint.y() - bottomPos, pitchFillColor);
                  painter.drawLine(lastPoint, {currentPoint.x(), lastPoint.y()});
                  lastPoint = currentPoint;
            }
            painter.fillRect(lastPoint.x(), bottomPos, (rightPos - lastPoint.x()) + 1, lastPoint.y() - bottomPos, pitchFillColor);
            painter.drawLine(lastPoint, {rightPos, lastPoint.y()});
      }

      // draw half step horigontal lines in lighter gray
      pen.setWidth(0);
      pen.setColor(pitchLinesColor);
      painter.setPen(pen);
      for (int i = 1; i < m_pitchDelta; ++i) {
            qreal yPos = topPos + (static_cast<qreal>(i) / pitchDelta) * graphHeight;
            painter.drawLine(leftPos, yPos, rightPos, yPos);
            }

      // draw time-warped vertical lines in lighter gray
      pen.setWidth(0);
      pen.setColor(eventLinesColor);
      painter.setPen(pen);
      for (int i = 1; i < m_nEvents; ++i) {
            qreal xPos = leftPos + eio.XfromY(static_cast<qreal>(i) / nbEvents) * graphWidth;
            painter.drawLine(xPos, topPos, xPos, bottomPos);
      }

      // Draw the Bezier transfer curve
      pen.setWidth(2);
      pen.setColor(warpLineColor);
      painter.setPen(pen);
      QPointF lastPoint = getPosition({0.0, 0.0});
      for (int i = 1; i <= 33; i++) {
            QPointF currentPoint = getPosition(eio.Eval(static_cast<qreal>(i) / 33.0));
            painter.drawLine(lastPoint, currentPoint);
            lastPoint = currentPoint;
            }

      // draw the graph frame after the other lines to cover them
      pen.setColor(borderLinesColor);
      pen.setWidth(1);
      painter.setPen(pen);
      painter.drawRect(border, border, w - 2 * border, h - 2 * border);

      QFrame::paintEvent(ev);
      }

} // namespace Ms
