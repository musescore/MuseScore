//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#include "libmscore/fret.h"
#include "fretproperties.h"
#include "libmscore/measure.h"
#include "libmscore/system.h"
#include "libmscore/score.h"
#include "fretcanvas.h"
#include "preferences.h"
#include "libmscore/tablature.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"

//---------------------------------------------------------
//   FretDiagramProperties
//---------------------------------------------------------

FretDiagramProperties::FretDiagramProperties(FretDiagram* _fd, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      fd = _fd;
      frets->setValue(fd->frets());
      strings->setValue(fd->strings());
      diagram->setFretDiagram(fd);

      diagramScrollBar->setRange(0, fd->maxFrets());
      diagramScrollBar->setValue(fd->fretOffset());

      connect(strings, SIGNAL(valueChanged(int)), SLOT(stringsChanged(int)));
      connect(frets, SIGNAL(valueChanged(int)), SLOT(fretsChanged(int)));
      connect(diagramScrollBar, SIGNAL(valueChanged(int)), SLOT(fretOffsetChanged(int)));
      }

//---------------------------------------------------------
//   fretsChanged
//---------------------------------------------------------

void FretDiagramProperties::fretsChanged(int val)
      {
      fd->setFrets(val);
      diagram->update();
      }

//---------------------------------------------------------
//   stringsChanged
//---------------------------------------------------------

void FretDiagramProperties::stringsChanged(int val)
      {
      fd->setStrings(val);
      diagram->update();
      }

//---------------------------------------------------------
//   fretOffsetChanged
//---------------------------------------------------------

void FretDiagramProperties::fretOffsetChanged(int val)
      {
      fd->setFretOffset(val);
      diagram->update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void FretCanvas::paintEvent(QPaintEvent* ev)
      {
      double mag        = 1.5;
      double _spatium   = 20.0 * mag;
      double lw1        = _spatium * 0.08;
      int fretOffset    = diagram->fretOffset();
      double lw2        = fretOffset ? lw1 : _spatium * 0.2;
      double stringDist = _spatium * .7;
      double fretDist   = _spatium * .8;
      int _strings      = diagram->strings();
      int _frets        = diagram->frets();
      char* _dots       = diagram->dots();
      char* _marker     = diagram->marker();

      double w  = (_strings - 1) * stringDist;
      double xo = (width() - w) * .5;
      double h  = (_frets * fretDist) + fretDist * .5;
      double yo = (height() - h) * .5;

      QFont font("FreeSans");
      int size = lrint(18.0 * mag);
      font.setPixelSize(size);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.translate(xo, yo);

      QPen pen(p.pen());
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);
      p.setBrush(pen.color());
      double x2 = (_strings-1) * stringDist;
      p.drawLine(QLineF(-lw1 * .5, 0.0, x2+lw1*.5, 0.0));

      pen.setWidthF(lw1);
      p.setPen(pen);
      double y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            double x = stringDist * i;
            p.drawLine(QLineF(x, fretOffset ? -_spatium*.2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            double y = fretDist * i;
            p.drawLine(QLineF(0.0, y, x2, y));
            }
      for (int i = 0; i < _strings; ++i) {
            p.setPen(Qt::NoPen);
            if (_dots && _dots[i]) {
                  double dotd = stringDist * .6 + lw1;
                  int fret = _dots[i] - 1;
                  double x = stringDist * i - dotd * .5;
                  double y = fretDist * fret + fretDist * .5 - dotd * .5;
                  p.drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            p.setPen(pen);
            if (_marker && _marker[i]) {
                  p.setFont(font);
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  p.drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      if (cfret > 0 && cfret <= _frets && cstring >= 0 && cstring < _strings) {
            double dotd;
            if (_dots[cstring] != cfret) {
                  p.setPen(Qt::NoPen);
                  dotd = stringDist * .6 + lw1;
                  }
            else {
                  p.setPen(pen);
                  dotd = stringDist * .6;
                  }
            double x = stringDist * cstring - dotd * .5;
            double y = fretDist * (cfret-1) + fretDist * .5 - dotd * .5;
            p.setBrush(Qt::lightGray);
            p.drawEllipse(QRectF(x, y, dotd, dotd));
            }
      if (fretOffset > 0) {
          p.setPen(pen);
            }
      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   getPosition
//---------------------------------------------------------

void FretCanvas::getPosition(const QPointF& p, int* string, int* fret)
      {
      double mag = 1.5;
      double _spatium   = 20.0 * mag;
      int _strings      = diagram->strings();
      int _frets        = diagram->frets();
      double stringDist = _spatium * .7;
      double fretDist   = _spatium * .8;

      double w  = (_strings - 1) * stringDist;
      double xo = (width() - w) * .5;
      double h  = (_frets * fretDist) + fretDist * .5;
      double yo = (height() - h) * .5;
      *fret  = (p.y() - yo + fretDist) / fretDist;
      *string = (p.x() - xo + stringDist * .5) / stringDist;
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void FretCanvas::mousePressEvent(QMouseEvent* ev)
      {
      int string;
      int fret;
      getPosition(ev->pos(), &string, &fret);

      int _strings = diagram->strings();
      int _frets   = diagram->frets();
      if (fret < 0 || fret > _frets || string < 0 || string >= _strings)
            return;

      char* _marker = diagram->marker();
      char* _dots   = diagram->dots();
      if (fret == 0) {
            switch(_marker[string]) {
                  case 'O':
                        _marker[string] = 'X';
                        break;
                  case 'X':
                        _marker[string] = 'O';
                        break;
                  default:
                        _marker[string] = 'O';
                        _dots[string] = 0;
                        break;
                  }
            }
      else {
            if (_dots[string] == fret) {
                  _dots[string] = 0;
                  _marker[string] = 'O';
                  }
            else {
                  _dots[string] = fret;
                  _marker[string] = 0;
                  }
            }
      update();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void FretCanvas::mouseMoveEvent(QMouseEvent* ev)
      {
      int string;
      int fret;
      getPosition(ev->pos(), &string, &fret);
      if (string != cstring || cfret != fret) {
            cfret = fret;
            cstring = string;
            update();
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void FretCanvas::mouseReleaseEvent(QMouseEvent*)
      {
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void FretCanvas::dragEnterEvent(QDragEnterEvent*)
      {
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void FretCanvas::dragMoveEvent(QDragMoveEvent*)
      {
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void FretCanvas::dropEvent(QDropEvent*)
      {
      }

//---------------------------------------------------------
//   FretCanvas
//---------------------------------------------------------

FretCanvas::FretCanvas(QWidget* parent)
   : QFrame(parent)
      {
      setAcceptDrops(true);
      setFrameStyle(QFrame::Raised | QFrame::Panel);
      cstring = -2;
      cfret   = -2;
      }

//---------------------------------------------------------
//   setFretDiagram
//---------------------------------------------------------

void FretCanvas::setFretDiagram(FretDiagram* fd)
      {
      diagram = fd;
      update();
      }

