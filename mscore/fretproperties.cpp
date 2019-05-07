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
#include "libmscore/undo.h"
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
      double lw2        = (fretOffset || !diagram->showNut()) ? lw1 : _spatium * 0.2;
      double stringDist = _spatium * .7;
      double fretDist   = _spatium * .8;
      int _strings      = diagram->strings();
      int _frets        = diagram->frets();
      double dotd       = stringDist * .6 + lw1;

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
      p.drawLine(QLineF(-lw1 * .5, 0.0, x2 + lw1 * .5, 0.0));

      pen.setWidthF(lw1);
      p.setPen(pen);
      double y2 = (_frets+1) * fretDist - fretDist*.5;

      QPen symPen(pen);
      symPen.setCapStyle(Qt::RoundCap);
      symPen.setWidthF(lw1 * 1.2);

      // Draw strings and frets
      for (int i = 0; i < _strings; ++i) {
            double x = stringDist * i;
            p.drawLine(QLineF(x, fretOffset ? -_spatium*.2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            double y = fretDist * i;
            p.drawLine(QLineF(0.0, y, x2, y));
            }

      // Draw dots and markers
      for (int i = 0; i < _strings; ++i) {
            for (auto const& d : diagram->dot(i)) {
                  if (d.exists()) {
                        p.setPen(symPen);
                        int fret = d.fret;
                        double x = stringDist * i - dotd * .5;
                        double y = fretDist * (fret - 1) + fretDist * .5 - dotd * .5;

                        paintDotSymbol(p, symPen, x, y, dotd, d.dtype);
                        }
                  }
            p.setPen(pen);

            FretItem::Marker mark = diagram->marker(i);
            if (mark.exists()) {
                  p.setFont(font);
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  p.drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, FretItem::markerToChar(mark.mtype));
                  }
            }

      // Draw barres
      p.setPen(pen);
      for (auto const& i : diagram->barres()) {            
            int fret        = i.first;
            int startString = i.second.startString;
            int endString   = i.second.endString;

            qreal x1   = stringDist * startString;
            qreal newX2 = endString == -1 ? x2 : stringDist * endString;

            qreal y    = fretDist * (fret - 1) + fretDist * .5;
            pen.setWidthF(dotd * diagram->score()->styleD(Sid::barreLineWidth));      // don’t use style barreLineWidth - why not?
            pen.setCapStyle(Qt::RoundCap);
            p.setPen(pen);
            p.drawLine(QLineF(x1, y, newX2, y));
            }

      // Draw 'hover' dot
      if ((cfret > 0) && (cfret <= _frets) && (cstring >= 0) && (cstring < _strings)) {
            FretItem::Dot cd = diagram->dot(cstring, cfret)[0];
            std::vector<FretItem::Dot> otherDots = diagram->dot(cstring);
            FretDotType dtype;
            symPen.setColor(Qt::lightGray);

            if (cd.exists()) {
                  dtype = cd.dtype;
                  symPen.setColor(Qt::red);
                  }
            else {
                  dtype = _automaticDotType ? FretDotType::NORMAL : _currentDtype;
                  }
            p.setPen(symPen);

            double x = stringDist * cstring - dotd * .5;
            double y = fretDist * (cfret-1) + fretDist * .5 - dotd * .5;
            p.setBrush(Qt::lightGray);
            paintDotSymbol(p, symPen, x, y, dotd, dtype);
            }

      if (fretOffset > 0) {
            qreal fretNumMag = 2.0; // TODO: get the value from Sid::fretNumMag
            QFont scaledFont(font);
            scaledFont.setPixelSize(font.pixelSize() * fretNumMag);
            p.setFont(scaledFont);
            p.setPen(pen);
            // Todo: make dependent from Sid::fretNumPos
            p.drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
               Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
               QString("%1").arg(fretOffset+1));
            p.setFont(font);
            }

      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   paintDotSymbol
//---------------------------------------------------------

void FretCanvas::paintDotSymbol(QPainter& p, QPen& pen, qreal x, qreal y, qreal dotd, FretDotType dtype)
      {
      switch (dtype) {
            case FretDotType::CROSS:
                  p.drawLine(QLineF(x, y, x + dotd, y + dotd));
                  p.drawLine(QLineF(x + dotd, y, x, y + dotd));
                  break;
            case FretDotType::SQUARE:
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(QRectF(x, y, dotd, dotd));
                  break;
            case FretDotType::TRIANGLE:
                  p.drawLine(QLineF(x, y + dotd, x + .5 * dotd, y));
                  p.drawLine(QLineF(x + .5 * dotd, y, x + dotd, y + dotd));
                  p.drawLine(QLineF(x + dotd, y + dotd, x, y + dotd));                                    
                  break;
            case FretDotType::NORMAL:
            default:
                  p.setBrush(pen.color());
                  p.setPen(Qt::NoPen);
                  p.drawEllipse(QRectF(x, y, dotd, dotd));
                  break;
            }
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

      diagram->score()->startCmd();

      // Click above the fret diagram, so change the open/closed string marker
      if (fret == 0) {
            switch (diagram->marker(string).mtype) {
                  case FretMarkerType::CIRCLE:
                        diagram->undoSetFretMarker(string, FretMarkerType::CROSS);
                        break;
                  case FretMarkerType::CROSS:
                        diagram->undoSetFretMarker(string, FretMarkerType::NONE);
                        break;
                  case FretMarkerType::NONE:
                  default:
                        diagram->undoSetFretDot(string, 0);
                        diagram->undoSetFretMarker(string, FretMarkerType::CIRCLE);
                        break;
                  }
            }
      // Otherwise, the click is on the fretboard itself
      else {
            FretItem::Dot thisDot = diagram->dot(string, fret)[0];
            bool haveShift = (ev->modifiers() & Qt::ShiftModifier) || _barreMode;
            bool haveCtrl  = (ev->modifiers() & Qt::ControlModifier) || _multidotMode;

            // Click on an existing dot
            if (thisDot.exists() && !haveShift)
                  diagram->undoSetFretDot(string, haveCtrl ? fret : 0, haveCtrl);
            else {
                  // Shift adds a barre
                  if (haveShift)
                        diagram->undoSetFretBarre(string, fret, haveCtrl);
                  else {
                        FretDotType dtype = FretDotType::NORMAL;
                        if (_automaticDotType && haveCtrl && diagram->dot(string)[0].exists()) {
                              dtype = FretDotType::TRIANGLE;

                              std::vector<FretDotType> dtypes { 
                                    FretDotType::NORMAL,
                                    FretDotType::CROSS,
                                    FretDotType::SQUARE,
                                    FretDotType::TRIANGLE
                              };

                              // Find the lowest dot type that doesn't already exist on the string
                              for (int i = 0; i < int(dtypes.size()); i++) {
                                    FretDotType t = dtypes[i];

                                    bool hasThisType = false;
                                    for (auto const& dot : diagram->dot(string)) {
                                          if (dot.dtype == t) {
                                                hasThisType = true;
                                                break;
                                                }
                                          }

                                    if (hasThisType)
                                          continue;

                                    dtype = t;
                                    break;
                                    }
                              }
                        else if (!_automaticDotType)
                              dtype = _currentDtype;

                        // Ctrl adds a dot without removing other dots on a string
                        diagram->undoSetFretDot(string, fret, haveCtrl, dtype);
                        }
                  }
            }
      diagram->triggerLayout();
      diagram->score()->endCmd();
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
//   FretCanvas
//---------------------------------------------------------

FretCanvas::FretCanvas(QWidget* parent)
   : QFrame(parent)
      {
      setAcceptDrops(true);
//      setFrameStyle(QFrame::Raised | QFrame::Panel);
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

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FretCanvas::clear()
      {
      diagram->score()->startCmd();
      diagram->undoFretClear();
      diagram->score()->endCmd();
      update();
      }
}

