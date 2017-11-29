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
//#include "libmscore/stringdata.h"
#include "fretcanvas.h"
#include "preferences.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   FretDiagramProperties
//---------------------------------------------------------

FretDiagramProperties::FretDiagramProperties(FretDiagram* _fd, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("FretDiagramProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      fd = _fd;
      frets->setValue(fd->frets());
      strings->setValue(fd->strings());
      diagram->setFretDiagram(fd);

      diagramScrollBar->setRange(0, fd->maxFrets());
      diagramScrollBar->setValue(fd->fretOffset());

      connect(strings, SIGNAL(valueChanged(int)), SLOT(stringsChanged(int)));
      connect(frets,   SIGNAL(valueChanged(int)), SLOT(fretsChanged(int)));
      connect(diagramScrollBar, SIGNAL(valueChanged(int)), SLOT(fretOffsetChanged(int)));

      MuseScore::restoreGeometry(this);
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
//   hideEvent
//---------------------------------------------------------

void FretDiagramProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
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
      int _barre        = diagram->barre();

      double w  = (_strings - 1) * stringDist;
      double xo = (width() - w) * .5;
      double h  = (_frets * fretDist) + fretDist * .5;
      double yo = (height() - h) * .5;

      QFont font("FreeSans");
      int size = lrint(18.0 * mag);
      font.setPixelSize(size);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.getBool(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING));
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
            if (diagram->dot(i)) {
                  double dotd = stringDist * .6 + lw1;
                  int fret = diagram->dot(i) - 1;
                  double x = stringDist * i - dotd * .5;
                  double y = fretDist * fret + fretDist * .5 - dotd * .5;
                  p.drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            p.setPen(pen);
            if (diagram->marker(i)) {
                  p.setFont(font);
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  p.drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(diagram->marker(i)));
                  }
            }
      if (_barre) {
            int string = -1;
            for (int i = 0; i < _strings; ++i) {
                  if (diagram->dot(i) == _barre) {
                        string = i;
                        break;
                        }
                  }
            if (string != -1) {
                  qreal x1   = stringDist * string;
                  qreal x2   = stringDist * (_strings-1);
                  qreal y    = fretDist * (_barre-1) + fretDist * .5;
                  pen.setWidthF(stringDist * .6 * .7);      // dont use style barreLineWidth
                  pen.setCapStyle(Qt::RoundCap);
                  p.setPen(pen);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            }
      if ((cfret > 0) && (cfret <= _frets) && (cstring >= 0) && (cstring < _strings)) {
            double dotd;
            if (diagram->dot(cstring) != cfret) {
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
            qreal fretNumMag = 2.0; // TODO: get the value from StyleIdx::fretNumMag
            QFont scaledFont(font);
            scaledFont.setPixelSize(font.pixelSize() * fretNumMag);
            p.setFont(scaledFont);
            p.setPen(pen);
            // Todo: make dependant from StyleIdx::fretNumPos
            p.drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
               Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
               QString("%1").arg(fretOffset+1));
            p.setFont(font);
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

      if (fret == 0) {
            switch (diagram->marker(string)) {
                  case 'O':
                        diagram->setMarker(string, 'X');
                        break;
                  case 'X':
                        diagram->setMarker(string, 0);
                        break;
                  default:
                        diagram->setDot(string, 0);
                        diagram->setMarker(string, 'O');
                        break;
                  }
            }
      else {
            if (diagram->dot(string) == fret) {
                  diagram->setDot(string, 0);
                  diagram->setMarker(string, 'O');
                  bool removeBarre = true;
                  if (ev->modifiers() & Qt::ShiftModifier) {
                        for (int i = 0; i < _strings; ++i) {
                              if (diagram->dot(i)) {
                                    removeBarre = false;
                                    break;
                                    }
                              }
                        }
                  if (removeBarre)
                        diagram->setBarre(0);
                  }
            else {
                  diagram->setDot(string, fret);
                  diagram->setMarker(string, 0);
                  if (ev->modifiers() & Qt::ShiftModifier)
                        diagram->setBarre(diagram->barre() == fret ? 0 : fret);
                  else
                        diagram->setBarre(0);
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
}

