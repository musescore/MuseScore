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
     m_nEvents(11)
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
      const int border = 5;

      const qreal graphWidth = static_cast<qreal>(w - 2 * border);
      const qreal graphHeight = static_cast<qreal>(h - 2 * border);
      const qreal nbEvents = static_cast<qreal>(m_nEvents);

      // let half a column of margin around
      const qreal leftPos = static_cast<qreal>(border);           // also left margin
      const qreal topPos = leftPos;                               // also top margin
      const qreal bottomPos = static_cast<qreal>(h - border);     // bottom end position of graph

      EaseInOut eio(static_cast<qreal>(m_easeIn) / 100.0, static_cast<qreal>(m_easeOut) / 100.0);

      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing, preferences.getBool(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING));

      painter.fillRect(rect(), QApplication::palette().color(QPalette::Window).lighter());
      QPen pen = painter.pen();
      pen.setWidth(1);

      QColor primaryLinesColor(preferences.isThemeDark() ? Qt::white : Qt::black);
      QColor secondaryLinesColor(Qt::gray);

      // draw time-warped vertical lines in lighter gray
      pen.setColor(secondaryLinesColor);
      painter.setPen(pen);
      for (int i = 1; i < m_nEvents; ++i) {
            qreal xPos = leftPos + eio.XfromY(static_cast<qreal>(i) / nbEvents) * graphWidth;
            painter.drawLine(xPos, topPos, xPos, bottomPos);
            }

      // this lambda takes as input a pitch value, and determines where what are its x and y coordinates
      auto getPosition = [this, graphWidth, graphHeight, leftPos, bottomPos] (const QPointF& p) -> QPointF {
            return {leftPos + p.x() * graphWidth, bottomPos - p.y() * graphHeight };
            };

      pen = painter.pen();
      pen.setWidth(2);
      pen.setColor(Qt::red); // not theme dependant
      painter.setPen(pen);
      // draw line between points
      QPointF lastPoint = getPosition({0.0, 0.0});
      for (int i = 1; i <= 33; i++) {
            QPointF currentPoint = getPosition(eio.Eval(static_cast<qreal>(i) / 33.0));
            painter.drawLine(lastPoint, currentPoint);
            lastPoint = currentPoint;
            }

      // draw the graph frame after the other lines to cover them
      pen.setColor(primaryLinesColor);
      pen.setWidth(1);
      painter.setPen(pen);
      painter.drawRect(border, border, w - 2 * border, h - 2 * border);

      QFrame::paintEvent(ev);
      }

} // namespace Ms
