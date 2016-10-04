//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/page.h"
#include "libmscore/sym.h"
#include "libmscore/instrument.h"
#include "libmscore/part.h"
#include "libmscore/timesig.h"
#include "libmscore/keysig.h"
#include "libmscore/barline.h"

#include "preferences.h"
#include "scoreview.h"
#include "continuouspanel.h"

namespace Ms {

//---------------------------------------------------------
//   ContinuousPanel
//---------------------------------------------------------

ContinuousPanel::ContinuousPanel(ScoreView* sv)
      {
      _sv                     = sv;
      _active                 = true;
      _visible                = false;
      _width                  = 0.0;
      _oldWidth               = 0.0;
      _newWidth               = 0.0;
      _measureWidth           = 0.0;
      _height                 = 0.0;
      _offsetPanel            = 0.0;
      _x                      = 0.0;
      _y                      = 0.0;
      _widthClef              = 0.0;
      _widthKeySig            = 0.0;
      _widthTimeSig           = 0.0;
      _leftMarginTotal        = 0.0;
      _panelRightPadding      = 5;
      _xPosTimeSig            = 0.0;
      _currentMeasure         = nullptr;
      _currentMeasureTick     = 0;
      _currentMeasureNo       = 0;
      _mmRestCount            = 0;
      _xPosMeasure            = 0;
      }


//---------------------------------------------------------
//   paintContinousPanel
//---------------------------------------------------------

void ContinuousPanel::paint(const QRect& /*r*/, QPainter& p)
      {
      if (!_active) {
            _visible = false;
            return;
            }

      Measure* measure = _score->tick2measure(0);
      if (measure == 0){
            _visible = false;
            return;
            }

      if (measure->mmRest()) {
            measure = measure->mmRest();
            _mmRestCount = measure->mmRestCount();
            }

      System* system = measure->system();
            if (system == 0) {
                  _visible = false;
                  return;
                  }

      Segment* s = _score->tick2segment(0);
      double _spatium = _score->spatium();
      _x = 0;
      if (_width <= 0)
            _width  = s->x();

      //
      // Set panel height for whole system
      //
      _height = 6 * _spatium;
      _y = system->staffYpage(0) + system->page()->pos().y();
      double y2 = 0.0;
      for (int i = 0; i < _score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show() || !_score->staff(i)->show())
                  continue;
            y2 = ss->y() + ss->bbox().height();
            }
      _height += y2 + 6*_spatium;
      _y -= 6 * _spatium;

      //
      // Check elements at current panel position
      //
      _offsetPanel = -(_sv->xoffset()) / _sv->mag();
      _rect = QRect(_offsetPanel + _width, _y, 1, _height);
      //qDebug() << "width=" << _width << "_y="<< _y << "_offsetPanel=" << _offsetPanel << "_sv->xoffset()" << _sv->xoffset() << "_sv->mag()" << _sv->mag() <<"_spatium" << _spatium << "s->canvasPos().x()" << s->canvasPos().x() << "s->x()" << s->x();
      Page* page = _score->pages().front();
      QList<Element*> elementsCurrent = page->items(_rect);
      if (elementsCurrent.empty()) {
            _visible = false;
            return;
            }
      qStableSort(elementsCurrent.begin(), elementsCurrent.end(), elementLessThan);

      _currentMeasure = nullptr;
      for (const Element* e : elementsCurrent) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (_score->printing() || !_score->showInvisible())
                        continue;
                  }

            if (e->type() == Element::Type::MEASURE) {
                  _currentMeasure = static_cast<const Measure*>(e);
                  _currentTimeSig = _currentMeasure->timesig();
                  _currentMeasureTick = _currentMeasure->tick();
                  _currentMeasureNo = _currentMeasure->no();

                  // Find number of multi measure rests to display in the panel
                  if (_currentMeasure->isMMRest())
                        _mmRestCount = _currentMeasure->mmRestCount();
                  else if (_currentMeasure->mmRest())
                        _mmRestCount = _currentMeasure->mmRest()->mmRestCount();
                  else
                        _mmRestCount = 0;
                  _xPosMeasure = e->canvasX();
                  _measureWidth = e->width();
                  break;
                  }
            }
      if (_currentMeasure == nullptr)
            return;

      findElementWidths(elementsCurrent);

      // Don't show panel if staff names are visible
      if (_currentMeasure == _score->firstMeasure() && _sv->toPhysical(_currentMeasure->canvasPos()).x() > 0) {
            _visible = false;
            return;
            }
      //qDebug() << "_sv->xoffset()=" <<_sv->xoffset() << " _sv->mag()="<< _sv->mag() <<" s->x=" << s->x() << " width=" << _width << " currentMeasue=" << _currentMeasure->x() << " _xPosMeasure=" << _xPosMeasure;

      draw(p, elementsCurrent);
      _visible = true;
      }


//---------------------------------------------------------
//   findElementWidths
//      determines the max width for each element types
//---------------------------------------------------------

void ContinuousPanel::findElementWidths(const QList<Element*>& el) {
      // The first pass serves to get the maximum width for each elements
      qreal lineWidthName = 0;
      _widthClef = 0;
      _widthKeySig = 0;
      _widthTimeSig = 0;
      _xPosTimeSig = 0;
      for (const Element* e : el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (_score->printing() || !_score->showInvisible())
                        continue;
                  }

            if (e->type() == Element::Type::STAFF_LINES) {
                  Staff* currentStaff = _score->staff(e->staffIdx());
                  Segment* parent = _score->tick2segment(_currentMeasureTick);

                  // Find maximum width for the staff name
                  QList<StaffName>& staffNamesLong = currentStaff->part()->instrument()->longNames();
                  QString staffName = staffNamesLong.isEmpty() ? " " : staffNamesLong[0].name();
                  if (staffName == "") {
                        QList<StaffName>& staffNamesShort = currentStaff->part()->instrument()->shortNames();
                        staffName = staffNamesShort.isEmpty() ? "" : staffNamesShort[0].name();
                        }
                  Text* newName = new Text(_score);
                  newName->setXmlText(staffName);
                  newName->setParent(parent);
                  newName->setTrack(e->track());
                  newName->textStyle().setFamily("FreeSans");
                  newName->textStyle().setSizeIsSpatiumDependent(true);
                  newName->layout();
                  newName->setPlainText(newName->plainText());
                  newName->layout();

                  // Find maximum width for the current Clef
                  Clef* newClef = new Clef(_score);
                  ClefType currentClef = currentStaff->clef(_currentMeasureTick);
                  newClef->setClefType(currentClef);
                  newClef->setParent(parent);
                  newClef->setTrack(e->track());
                  newClef->layout();

                  // Find maximum width for the current KeySignature
                  KeySig* newKs = new KeySig(_score);
                  KeySigEvent currentKeySigEvent = currentStaff->keySigEvent(_currentMeasureTick);
                  newKs->setKeySigEvent(currentKeySigEvent);
                  // The Parent and the Track must be set to have the key signature layout adjusted to different clefs
                  // This also adds naturals to the key signature (if set in the score style)
                  newKs->setParent(parent);
                  newKs->setTrack(e->track());
                  newKs->setHideNaturals(true);
                  newKs->layout();

                  // Find maximum width for the current TimeSignature
                  TimeSig* newTs = new TimeSig(_score);

                  // Try to get local time signature, if not, get the current measure one
                  TimeSig* currentTimeSig = currentStaff->timeSig(_currentMeasureTick);
                  if (currentTimeSig)
                        newTs->setFrom(currentTimeSig);
                  else
                        newTs->setSig(Fraction(_currentTimeSig.numerator(), _currentTimeSig.denominator()), TimeSigType::NORMAL);
                  newTs->setParent(parent);
                  newTs->setTrack(e->track());
                  newTs->layout();

                  if ((newName->width() > lineWidthName) && (newName->xmlText() != ""))
                        lineWidthName = newName->width();

                  if (newClef->width() > _widthClef)
                        _widthClef = newClef->width();

                  if (newKs->width() > _widthKeySig)
                        _widthKeySig = newKs->width();

                  if (newTs->width() > _widthTimeSig)
                        _widthTimeSig = newTs->width();

                  delete newClef;
                  delete newName;
                  delete newKs;
                  delete newTs;
                 }
            }

      _leftMarginTotal = _score->styleP(StyleIdx::clefLeftMargin);
      _leftMarginTotal += _score->styleP(StyleIdx::keysigLeftMargin);
      _leftMarginTotal += _score->styleP(StyleIdx::timesigLeftMargin);

      _newWidth = _widthClef + _widthKeySig + _widthTimeSig + _leftMarginTotal + _panelRightPadding;
      _xPosMeasure -= _offsetPanel;

      lineWidthName += _score->spatium() + _score->styleP(StyleIdx::clefLeftMargin) + _widthClef;
      if (_newWidth < lineWidthName) {
            _newWidth = lineWidthName;
            _oldWidth = 0;
            }
      if (_oldWidth == 0) {
            _oldWidth = _newWidth;
            _width = _newWidth;
            }
      else if (_newWidth > 0) {
            if (_newWidth == _width) {
                  _oldWidth = _width;
                  _width = _newWidth;
                  }
            else if (((_xPosMeasure <= _newWidth) && (_xPosMeasure >= _oldWidth)) ||
                     ((_xPosMeasure >= _newWidth) && (_xPosMeasure <= _oldWidth)))
                        _width = _xPosMeasure;
            else if (((_xPosMeasure+_measureWidth <= _newWidth) && (_xPosMeasure+_measureWidth >= _oldWidth)) ||
                     ((_xPosMeasure+_measureWidth >= _newWidth) && (_xPosMeasure+_measureWidth <= _oldWidth)))
                        _width = _xPosMeasure+_measureWidth;
            else {
                  _oldWidth = _width;
                  _width = _newWidth;
                  }
            }

      _rect = QRect(0, _y, _width, _height);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ContinuousPanel::draw(QPainter& painter, const QList<Element*>& el) {
      painter.save();
      painter.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      painter.setRenderHint(QPainter::TextAntialiasing, true);

      // Draw colored rectangle
      painter.setClipping(false);
      QPointF pos(_offsetPanel, 0);
      painter.translate(pos);
      QPen pen;
      pen.setWidthF(0.0);
      pen.setStyle(Qt::NoPen);
      painter.setPen(pen);
      painter.setBrush(preferences.fgColor);
      QRectF bg(_rect);
      bg.setWidth(_widthClef + _widthKeySig + _widthTimeSig + _leftMarginTotal + _panelRightPadding);
      QPixmap* fgPixmap = _sv->fgPixmap();
      if (fgPixmap == 0 || fgPixmap->isNull())
            painter.fillRect(bg, preferences.fgColor);
      else {
            painter.setMatrixEnabled(false);
            painter.drawTiledPixmap(bg, *fgPixmap, bg.topLeft()
               - QPoint(lrint(_sv->matrix().dx()), lrint(_sv->matrix().dy())));
            painter.setMatrixEnabled(true);
            }

      painter.setClipRect(_rect);
      painter.setClipping(true);

      QColor color(MScore::layoutBreakColor);

      // Draw measure text number
      QString text = _mmRestCount ? QString("#%1-%2").arg(_currentMeasureNo+1).arg(_currentMeasureNo+_mmRestCount) : QString("#%1").arg(_currentMeasureNo+1);
      Text* newElement = new Text(_score);
      newElement->setTextStyleType(TextStyleType::DEFAULT);
      newElement->setFlag(ElementFlag::MOVABLE, false);
      newElement->setXmlText(text);
      newElement->textStyle().setFamily("FreeSans");
      newElement->textStyle().setSizeIsSpatiumDependent(true);
      newElement->setColor(color);
      newElement->sameLayout();
      pos = QPointF(_score->styleP(StyleIdx::clefLeftMargin) + _widthClef, _y + newElement->height());
      painter.translate(pos);
      newElement->draw(&painter);
      pos += QPointF(_offsetPanel, 0);
      painter.translate(-pos);
      delete newElement;

      // This second pass draws the elements spaced evently using the width of the largest element
      for (const Element* e : el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (_score->printing() || !_score->showInvisible())
                        continue;
                  }

           if (e->type() == Element::Type::STAFF_LINES) {
                  Staff* currentStaff = _score->staff(e->staffIdx());
                  Segment* parent = _score->tick2segmentMM(_currentMeasureTick);

                  pos = QPointF (_offsetPanel, e->pagePos().y());
                  painter.translate(pos);

                  // Draw staff lines
                  StaffLines* newStaffLines = static_cast<StaffLines*>(e->clone());
                  newStaffLines->setWidth(bg.width());
                  newStaffLines->setParent(parent);
                  newStaffLines->setTrack(e->track());
                  newStaffLines->layout();
                  newStaffLines->setColor(color);
                  newStaffLines->draw(&painter);
                  delete newStaffLines;

                  // Draw barline
                  BarLine* newBarLine = new BarLine(_score);
                  newBarLine->setBarLineType(BarLineType::NORMAL);
                  newBarLine->setParent(parent);
                  newBarLine->setTrack(e->track());
                  newBarLine->setSpan(currentStaff->barLineSpan());
                  newBarLine->setSpanFrom(currentStaff->barLineFrom());
                  newBarLine->setSpanTo(currentStaff->barLineTo());
                  newBarLine->layout();
                  newBarLine->setColor(color);
                  newBarLine->draw(&painter);
                  delete newBarLine;

                  // Draw the current staff name
                  QList<StaffName>& staffNamesLong = currentStaff->part()->instrument()->longNames();
                  QString staffName = staffNamesLong.isEmpty() ? " " : staffNamesLong[0].name();
                  if (staffName == "") {
                        QList<StaffName>& staffNamesShort = currentStaff->part()->instrument()->shortNames();
                        staffName = staffNamesShort.isEmpty() ? "" : staffNamesShort[0].name();
                        }

                  Text* newName = new Text(_score);
                  newName->setXmlText(staffName);
                  newName->setParent(parent);
                  newName->setTrack(e->track());
                  newName->setColor(color);
                  newName->textStyle().setFamily("FreeSans");
                  newName->textStyle().setSizeIsSpatiumDependent(true);
                  newName->layout();
                  newName->setPlainText(newName->plainText());
                  newName->layout();
                  if (currentStaff->part()->staff(0) == currentStaff) {
                        double _spatium = _score->spatium();
                        pos = QPointF (_score->styleP(StyleIdx::clefLeftMargin) + _widthClef, 0 - _spatium * 2);
                        painter.translate(pos);
                        newName->draw(&painter);
                        painter.translate(-pos);
                        }
                  delete newName;

                  // Draw the current Clef
                  Clef* newClef = new Clef(_score);
                  ClefType currentClef = currentStaff->clef(_currentMeasureTick);
                  newClef->setClefType(currentClef);
                  newClef->setParent(parent);
                  newClef->setTrack(e->track());
                  newClef->setColor(color);
                  newClef->layout();
                  pos = QPointF(_score->styleP(StyleIdx::clefLeftMargin), 0);
                  painter.translate(pos);
                  newClef->draw(&painter);
                  pos = QPointF(_widthClef,0);
                  painter.translate(pos);
                  delete newClef;

                  // Draw the current KeySignature
                  KeySig* newKs = new KeySig(_score);
                  KeySigEvent currentKeySigEvent = currentStaff->keySigEvent(_currentMeasureTick);
                  newKs->setKeySigEvent(currentKeySigEvent);

                  // The Parent and the track must be set to have the key signature layout adjusted to different clefs
                  // This also adds naturals to the key signature (if set in the score style)
                  newKs->setParent(parent);
                  newKs->setTrack(e->track());
                  newKs->setColor(color);

                  newKs->setHideNaturals(true);
                  pos = QPointF(_score->styleP(StyleIdx::keysigLeftMargin),0);
                  painter.translate(pos);
                  newKs->layout();
                  newKs->draw(&painter);
                  delete newKs;

                  pos = QPointF(_widthKeySig + _xPosTimeSig, 0);
                  painter.translate(pos);

                  // Draw the current TimeSignature
                  TimeSig* newTs = new TimeSig(_score);

                  // Try to get local time signature, if not, get the current measure one
                  TimeSig* currentTimeSig = currentStaff->timeSig(_currentMeasureTick);
                  if (currentTimeSig) {
                        newTs->setFrom(currentTimeSig);
                        newTs->setParent(parent);
                        newTs->setTrack(e->track());
                        newTs->setColor(color);
                        newTs->layout();
                        pos = QPointF(_score->styleP(StyleIdx::timesigLeftMargin),0);
                        painter.translate(pos);
                        newTs->draw(&painter);
                        delete newTs;
                        }
                  pos = QPointF(_offsetPanel + _widthClef + _widthKeySig + _xPosTimeSig + _leftMarginTotal, e->pagePos().y());
                  painter.translate(-pos);
                  }
            }
      painter.restore();
      }
}
