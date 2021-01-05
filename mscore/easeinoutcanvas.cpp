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
#include "libmscore/easeInOut.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   EaseInOutCanvas
//---------------------------------------------------------

EaseInOutCanvas::EaseInOutCanvas(QWidget* parent)
   : QFrame(parent),
     m_easeIn(0.0),
     m_easeOut(0.0),
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
      const int nPitches = std::abs(m_pitchDelta) + 1;

      const qreal graphWidth = static_cast<qreal>(w - 2 * border);
      const qreal graphHeight = static_cast<qreal>(h - 2 * border);
      const qreal nbEvents = static_cast<qreal>(m_events.size());
      const qreal pitchDelta = static_cast<qreal>(nPitches);

      // let half a column of margin around
      const qreal leftPos = static_cast<qreal>(border);           // also left margin
      const qreal topPos = leftPos;                               // also top margin
      const qreal bottomPos = static_cast<qreal>(h - border);     // bottom end position of graph
      const qreal rightPos = static_cast<qreal>(w - border);      // right end position of graph

      EaseInOut eio(static_cast<qreal>(m_easeIn) / 100.0, static_cast<qreal>(m_easeOut) / 100.0);

      char noteNames[] = ("C D EF G A B");

      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing, preferences.getBool(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING));

      painter.fillRect(rect(), QApplication::palette().color(QPalette::Window).lighter());
      QPen pen = painter.pen();
      pen.setWidth(1);

      QColor eventLinesColor(Qt::gray);
      eventLinesColor.setAlphaF(0.5);
      QColor pitchLinesColor(Qt::gray);
      pitchLinesColor.setAlphaF(0.25);

      // Color scheme based on the MuseScore blue.
      QColor borderLinesColor, warpLineColor, pitchFillColor, pitchGraphColor, pitchNameColor;
      if (preferences.isThemeDark()) {
            pitchFillColor.setRgbF(0.078, 0.284, 0.463);
            pitchGraphColor.setRgbF(0.125, 0.455, 0.741);
            warpLineColor.setRgbF(0.621, 0.315, 0.132);
            borderLinesColor.setRgbF(0.891, 0.932, 0.968);
            pitchNameColor.setRgbF(0.935, 0.782, 0.691);
            }
      else {
            pitchFillColor.setRgbF(0.891, 0.932, 0.968);
            pitchGraphColor.setRgbF(0.125, 0.455, 0.741);
            warpLineColor.setRgbF(0.914, 0.710, 0.588);
            borderLinesColor.setRgbF(0.016, 0.057, 0.093);
            pitchNameColor.setRgbF(0.310, 0.157, 0.066);
            }

      // this lambda takes as input a pitch value, and determines where what are its x and y coordinates
      auto getPosition = [graphWidth, graphHeight, leftPos, bottomPos](const QPointF& p) -> QPointF {
            return { leftPos + p.x() * graphWidth, bottomPos - p.y() * graphHeight };
            };

      std::vector<QPointF> pitchPoints;
      qreal offset = m_pitchDelta < 0 ? 1.0 : 0.0;
      QPointF prevPoint, currPoint;
      for (size_t i = 0; i < m_events.size(); i++) {
            currPoint = getPosition({ eio.XfromY(static_cast<qreal>(i) / nbEvents), offset + static_cast<qreal>(m_events[i]) / static_cast<qreal>(pitchDelta) });
            pitchPoints.push_back(currPoint);
            }

      // Draw the pitches barchart graph in the background first
      if (pitchPoints.size() > 1) {
            prevPoint = pitchPoints[0];
            for (size_t i = 1; i < pitchPoints.size(); i++) {
                  currPoint = pitchPoints[i];
                  painter.fillRect(prevPoint.x(), bottomPos, (currPoint.x() - prevPoint.x()) + 1, prevPoint.y() - bottomPos, pitchFillColor);
                  prevPoint = currPoint;
                  }
            painter.fillRect(prevPoint.x(), bottomPos, (rightPos - prevPoint.x()) + 1, prevPoint.y() - bottomPos, pitchFillColor);

            // draw time-warped vertical lines in lighter gray.
            // These lines will move as ease-in and ease-out are adjusted.
            pen.setWidth(0);
            pen.setColor(eventLinesColor);
            painter.setPen(pen);
            for (size_t i = 1; i < pitchPoints.size(); ++i) {
                  qreal xPos = pitchPoints[i].x();
                  painter.drawLine(xPos, topPos, xPos, bottomPos);
                  }
            }

      // draw half step horigontal lines in even lighter gray
      pen.setWidth(0);
      pen.setColor(pitchLinesColor);
      painter.setPen(pen);
      for (int i = 1; i < nPitches; ++i) {
            qreal yPos = topPos + (static_cast<qreal>(i) / pitchDelta) * graphHeight;
            painter.drawLine(leftPos, yPos, rightPos, yPos);
            }

      // draw note names
      pen.setColor(pitchNameColor);
      painter.setPen(pen);
      QFont font;
      qreal fontHeight = std::min(12.0, (graphHeight * 0.875) / pitchDelta);
      font.setPixelSize(fontHeight);
      painter.setFont(font);
      int curPitch = m_bottomPitch;
      for (int i = 0; i <= nPitches; ++i) {
            QString pitchName(noteNames[(curPitch - 60) % 12]);
            QPointF pos = { 4, topPos + fontHeight * 0.3 + (1.0 - (static_cast<qreal>(i) / pitchDelta)) * graphHeight };
            painter.drawText(pos, pitchName);
            curPitch++;
            }

      if (m_events.size()) {
            // Not a portamento style glissando.
            // Draw the Bezier transfer curve only in ease-in or ease-out are not zero. This warps
            // the event times so this curve always go from lower left corner to upper-right corner.
            if (m_easeIn != 0 || m_easeOut != 0) {
                  pen.setWidth(2);
                  pen.setColor(warpLineColor);
                  painter.setPen(pen);
                  prevPoint = { leftPos, bottomPos };
                  for (int i = 1; i <= 33; i++) {
                        currPoint = getPosition(eio.Eval(static_cast<qreal>(i) / 33.0));
                        painter.drawLine(prevPoint, currPoint);
                        prevPoint = currPoint;
                        }
                  }
            }
      else {
            // Draw the portamento style glissando curve instead of the transfer curve
            pen.setWidth(3);
            pen.setColor(pitchGraphColor);
            painter.setPen(pen);
            prevPoint = { leftPos, m_pitchDelta > 0 ? bottomPos : topPos };
            for (int i = 1; i <= 33; i++) {
                  currPoint = eio.Eval(static_cast<qreal>(i) / 33.0);
                  if (m_pitchDelta < 0)
                        currPoint.setY(1.0 - currPoint.y());
                  currPoint = getPosition(currPoint);
                  painter.drawLine(prevPoint, currPoint);
                  prevPoint = currPoint;
                  }
            }

      // Draw the pitches level lines next so they cover the Bezier transfer curve.
      if (pitchPoints.size() > 1) {
            pen.setWidth(3);
            pen.setCapStyle(Qt::FlatCap);
            pen.setColor(pitchGraphColor);
            painter.setPen(pen);
            prevPoint = pitchPoints[0];
            for (size_t i = 1; i < pitchPoints.size(); i++) {
                  currPoint = pitchPoints[i];
                  painter.drawLine(prevPoint, { currPoint.x(), prevPoint.y() });
                  prevPoint = currPoint;
                  }
            painter.drawLine(prevPoint, { rightPos, prevPoint.y() });
            }

      // draw the graph frame after all the other graphics elements to cover them
      pen.setColor(borderLinesColor);
      pen.setWidth(1);
      painter.setPen(pen);
      painter.drawRect(border, border, w - 2 * border, h - 2 * border);

      QFrame::paintEvent(ev);
      }

} // namespace Ms
