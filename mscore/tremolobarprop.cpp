//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2010-2011 Werner Schweer and others
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

#include "tremolobarprop.h"
#include "libmscore/tremolobar.h"
#include "tremolobarcanvas.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "musescore.h"

namespace Ms {

enum class TremoloBarType { DIP, DIVE, RELEASE_UP, INVERTED_DIP, RETURN, RELEASE_DOWN };

//---------------------------------------------------------
//   TremoloBarProperties
//---------------------------------------------------------

TremoloBarProperties::TremoloBarProperties(TremoloBar* b, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TremoloBarProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      bend = b;
      bendCanvas->setPoints(bend->points());
      bendTypes = new QButtonGroup(this);
      bendTypes->addButton(bend1, int(TremoloBarType::DIP));
      bendTypes->addButton(bend2, int(TremoloBarType::DIVE));
      bendTypes->addButton(bend3, int(TremoloBarType::RELEASE_UP));
      bendTypes->addButton(bend4, int(TremoloBarType::INVERTED_DIP));
      bendTypes->addButton(bend5, int(TremoloBarType::RETURN));
      bendTypes->addButton(bend6, int(TremoloBarType::RELEASE_DOWN));
      bendTypes->setExclusive(true);

      connect(bendTypes, SIGNAL(buttonClicked(int)), SLOT(bendTypeChanged(int)));
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   points
//---------------------------------------------------------

const QList<PitchValue>& TremoloBarProperties::points() const
      {
      return bendCanvas->points();
      }

//---------------------------------------------------------
//   bendTypeChanged
//---------------------------------------------------------

void TremoloBarProperties::bendTypeChanged(int n)
      {
      QList<PitchValue>& points = bendCanvas->points();

      points.clear();
      switch (TremoloBarType(n)) {
            case TremoloBarType::DIP:
                  points.append(PitchValue(0,0));
                  points.append(PitchValue(30,-100));
                  points.append(PitchValue(60,0));
                  break;

            // TODO: fill in the right points

            case TremoloBarType::DIVE:
                  points.append(PitchValue(0,0));
                  points.append(PitchValue(10,100));
                  points.append(PitchValue(20,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(60,0));
                  break;
            case TremoloBarType::RELEASE_UP:
                  points.append(PitchValue(0,0));
                  points.append(PitchValue(10,100));
                  points.append(PitchValue(20,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(40,0));
                  points.append(PitchValue(50,100));
                  points.append(PitchValue(60,100));
                  break;
            case TremoloBarType::INVERTED_DIP:
                  points.append(PitchValue(0,100));
                  points.append(PitchValue(60,100));
                  break;
            case TremoloBarType::RETURN:
                  points.append(PitchValue(0,100));
                  points.append(PitchValue(15,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(60,0));
                  break;
            case TremoloBarType::RELEASE_DOWN:
                  points.append(PitchValue(0,100));
                  points.append(PitchValue(15,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(60,50));
                  break;
            }
      update();
      }

//---------------------------------------------------------
//   TremoloBarCanvas
//---------------------------------------------------------

TremoloBarCanvas::TremoloBarCanvas(QWidget* parent)
   : QFrame(parent)
      {
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void TremoloBarCanvas::paintEvent(QPaintEvent* ev)
      {
      int w = width();
      int h = height();

      QPainter p(this);
      p.fillRect(rect(), Qt::white);

      static const int ROWS    = 25;
      static const int COLUMNS = 13;

      int xs = w / (COLUMNS);
      int ys = h / (ROWS);
      int lm = xs / 2;
      int tm = ys / 2;
      int tw = (COLUMNS - 1) * xs;
      int th = (ROWS - 1)    * ys;

      QPen pen = p.pen();
      pen.setWidth(1);
      for (int x = 0; x < COLUMNS; ++x) {
            int xx = lm + x * xs;
            pen.setColor(x % 3 ? Qt::gray : Qt::black);
            p.setPen(pen);
            p.drawLine(xx, tm, xx, tm + th);
            }

      for (int y = 0; y < ROWS; ++y) {
            int yy = tm + y * ys;
            pen.setColor(y % 2 ? Qt::gray : Qt::black);
            pen.setWidth(y == ROWS/2 ? 3 : 1);
            p.setPen(pen);
            p.drawLine(lm, yy, lm + tw, yy);
            }

      static const int GRIP  = 10;
      static const int GRIP2 = 5;

      int x1 = 0;
      int y1 = 0;
      int idx = 0;
      pen = p.pen();
      pen.setWidth(5);
      pen.setColor(Qt::gray);
      p.setPen(pen);
      for (const PitchValue& v : _points) {
            int x = ((tw * v.time) / 60) + lm;
            int y = th - ((th * v.pitch) / 300) - ys * (ROWS/2) + tm;
            if (idx)
                  p.drawLine(x1, y1, x, y);
            x1 = x;
            y1 = y;
            ++idx;
            }

      for (const PitchValue& v : _points) {
            int x = ((tw * v.time) / 60) + lm;
            int y = th - ((th * v.pitch) / 300) - ys * (ROWS/2) + tm;
            p.fillRect(x - GRIP2, y - GRIP2, GRIP, GRIP, Qt::blue);
            }

      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TremoloBarCanvas::mousePressEvent(QMouseEvent* ev)
      {
      static const int ROWS = 13;
      static const int COLUMNS = 13;

      int xs = width() / (COLUMNS);
      int ys = height() / (ROWS);
      int lm = xs / 2;
      int tm = ys / 2;
//      int tw = (COLUMNS - 1) * xs;
//      int th = (ROWS - 1)    * ys;

      int x = ev->x() - lm;
      int y = ev->y() - tm;
      x = (x + xs/2) / xs;
      y = (y + ys/2) / ys;
      if (x >= COLUMNS)
            x = COLUMNS - 1;
      if (y >= ROWS)
            y = ROWS - 1;
      y = ROWS - y - 1;

      int time = x * 5;
      int pitch = y * 25;

      int n = _points.size();
      bool found = false;
      for (int i = 0; i < n; ++i) {
            if (_points[i].time > time) {
                  _points.insert(i, PitchValue(time, pitch, false));
                  found = true;
                  break;
                  }
            if (_points[i].time == time) {
                  if (_points[i].pitch == pitch && i > 0 && i < (n-1)) {
                        _points.removeAt(i);
                        }
                  else {
                        _points[i].pitch = pitch;
                        }
                  found = true;
                  break;
                  }
            }
      if (!found)
            _points.append(PitchValue(time, pitch, false));
      update();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TremoloBarProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

