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

#include "scoreview.h"
#include "continuouspanel.h"

namespace Ms {

//---------------------------------------------------------
//   ContinuousPanel
//---------------------------------------------------------

ContinuousPanel::ContinuousPanel(ScoreView* sv)
      {
      _sv                     = sv;
      _visible                = false;
      _width                  = 0.0;
      _oldWidth               = 0.0;
      _newWidth               = 0.0;
      _measureWidth           = 0.0;
      _height                 = 0.0;
      _offsetPanel            = 0.0;
      _x                      = 0.0;
      _y                      = 0.0;
      _heightName             = 0.0;
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
      if (!_visible)
            return;
      Measure* measure = _score->tick2measure(0);
      if (measure == 0)
            return;

      if (measure->mmRest()) {
            measure = measure->mmRest();
            _mmRestCount = measure->mmRestCount();
            }

      System* system = measure->system();
            if (system == 0)
                  return;

      Segment* s = _score->tick2segment(0);
      double _spatium = _score->spatium();
      _x = 0;
      if (_width <= 0)
            _width  = s->x();

      //
      // Don't show panel if staff names are visible
      //
      if (_sv->xoffset() / _sv->mag() + _width >= 0)
           return;

      //
      // Set panel height for whole system
      //
      _height  = 6 * _spatium;
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
      if (elementsCurrent.empty())
            return;
      qStableSort(elementsCurrent.begin(), elementsCurrent.end(), elementLessThan);

      _currentMeasure = nullptr;
      foreach(const Element* e, elementsCurrent) {
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
      draw(p, elementsCurrent);
      }


//---------------------------------------------------------
//   findElementWidths
//      determines the max width for each element types
//---------------------------------------------------------

void ContinuousPanel::findElementWidths(const QList<Element*>& el) {
      //
      // The first pass serves to get the maximum width for each elements
      //
      _heightName = 0;
      _widthClef = 0;
      _widthKeySig = 0;
      _widthTimeSig = 0;
      _xPosTimeSig = 0;
      foreach(const Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (_score->printing() || !_score->showInvisible())
                        continue;
                  }

           if (e->type() == Element::Type::STAFF_LINES) {
                  Staff* currentStaff = _score->staff(e->staffIdx());
                  Segment* parent = _score->tick2segment(_currentMeasureTick);

                  //
                  // Find maximum height for the staff name
                  //
                  QList<StaffName>& staffNamesShort = currentStaff->part()->instr()->shortNames();
                  QString staffName = staffNamesShort.isEmpty() ? "" : staffNamesShort[0].name;
                  if (staffName == "") {
                        QList<StaffName>& staffNamesLong = currentStaff->part()->instr()->longNames();
                        staffName = staffNamesLong.isEmpty() ? " " : staffNamesLong[0].name;
                  }
                  Text* newName = new Text(_score);
                  newName->setText(staffName);
                  newName->setParent(parent);
                  newName->setTrack(e->track());
                  newName->layout();

                  //
                  // Find maximum width for the current Clef
                  //
                  Clef* newClef = new Clef(_score);
                  ClefType currentClef = currentStaff->clef(_currentMeasureTick);
                  newClef->setClefType(currentClef);
                  newClef->setParent(parent);
                  newClef->setTrack(e->track());
                  newClef->layout();

                  //
                  // Find maximum width for the current KeySignature
                  //
                  KeySig* newKs = new KeySig(_score);
                  KeySigEvent currentKeySigEvent = currentStaff->key(_currentMeasureTick);
                  newKs->setKeySigEvent(currentKeySigEvent);
                  // The Parent and the Track must be set to have the key signature layout adjusted to different clefs
                  // This also adds naturals to the key signature (if set in the score style)
                  newKs->setParent(parent);
                  newKs->setTrack(e->track());
                  newKs->setHideNaturals(true);
                  newKs->layout();

                  //
                  // Find maximum width for the current TimeSignature
                  //
                  TimeSig* newTs = new TimeSig(_score);

                  // Try to get local time signature, if not, get the current measure one
                  TimeSig* currentTimeSig = currentStaff->timeSig(_currentMeasureTick);
                  if (currentTimeSig)
                        newTs->setFrom(currentTimeSig);
                  else
                        newTs->setSig(_currentTimeSig.numerator(), _currentTimeSig.denominator(), TimeSigType::NORMAL);
                  newTs->setParent(parent);
                  newTs->setTrack(e->track());
                  newTs->layout();

                  if ((newName->height() > _heightName) && (newName->text() != ""))
                        _heightName = newName->height();

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

      _newWidth = _heightName + _widthClef + _widthKeySig + _widthTimeSig + _leftMarginTotal + _panelRightPadding;
      _xPosMeasure -= _offsetPanel;
      //qDebug() << "_xPosMeasure="<< _xPosMeasure << "_width ="<<_width<< " offsetpanel ="<<_offsetPanel << "newWidth ="<<_newWidth  << "oldWidth ="<<_oldWidth << "_measureWidth ="<<_measureWidth;
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

      //
      // Draw colored rectangle
      //
      painter.setClipping(false);
      QPointF pos(_offsetPanel, 0);
      painter.translate(pos);
      QColor c(MScore::selectColor[0]);
      QPen pen(c);
      pen.setWidthF(0.0);
      pen.setStyle( Qt::NoPen );
      painter.setPen(pen);
      painter.setOpacity(0.80);
      painter.setBrush(QColor(205, 210, 235, 255));
      painter.drawRect(_rect);
      painter.setClipRect(_rect);
      painter.setClipping(true);

      //
      // Draw measure text number
      //
      painter.setOpacity(1);
      painter.setBrush(QColor(0, 0, 0, 255));
      QString text = _mmRestCount ? QString("#%1-%2").arg(_currentMeasureNo+1).arg(_currentMeasureNo+_mmRestCount) : QString("#%1").arg(_currentMeasureNo+1);
      Text* newElement = new Text(_score);
      newElement->setTextStyleType(TextStyleType::DEFAULT);
      newElement->setFlag(ElementFlag::MOVABLE, false);
      newElement->setText(text);
      newElement->sameLayout();
      pos = QPointF (_heightName * 1.5, _y + newElement->height());
      painter.translate(pos);
      newElement->draw(&painter);
      pos += QPointF (_offsetPanel, 0);
      painter.translate(-pos);
      delete newElement;

      //
      // This second pass draws the elements spaced evently using the width of the largest element
      //
      foreach(const Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (_score->printing() || !_score->showInvisible())
                        continue;
                  }

           if (e->type() == Element::Type::STAFF_LINES) {
                  Staff* currentStaff = _score->staff(e->staffIdx());
                  Segment* parent = _score->tick2segment(_currentMeasureTick);

                  //
                  // Draw the current staff name
                  //
                  QList<StaffName>& staffNamesShort = currentStaff->part()->instr()->shortNames();
                  QString staffName = staffNamesShort.isEmpty() ? "" : staffNamesShort[0].name;
                  if (staffName == "") {
                        QList<StaffName>& staffNamesLong = currentStaff->part()->instr()->longNames();
                        staffName = staffNamesLong.isEmpty() ? " " : staffNamesLong[0].name;
                  }

                  Text* newName = new Text(_score);
                  newName->setText(staffName);
                  newName->setParent(parent);
                  newName->setTrack(e->track());
                  newName->layout();
                  pos = QPointF (_offsetPanel, e->pagePos().y());
                  painter.translate(pos);
                  //qDebug() << "_heightName=" << _heightName << "  newName->height=" << newName->height() << "  staff (e->height)=" << e->height() << "  newName->width()=" << newName->width() << "  newName->linespace()=" << newName->lineSpacing() << "  newName->lineHeight()=" << newName->lineHeight();

                  if (currentStaff->primaryStaff()) {
                        painter.rotate(-90);
                        int nbLines = newName->height() / newName->lineHeight();
                        pos = QPointF (-e->height() + (e->height() - newName->width()) / 2, _heightName - (_heightName - _heightName / nbLines) - newName->lineSpacing() * 0.1);  // Because we rotate the canvas, height and width are swaped
                        painter.translate(pos);
                        newName->draw(&painter);
                        painter.translate(-pos);
                        painter.rotate(90);
                  }
                  pos = QPointF (_heightName, 0);
                  painter.translate(pos);
                  delete newName;

                  //
                  // Draw staff lines
                  //
                  StaffLines* newStaffLines = static_cast<StaffLines*>(e->clone());
                  newStaffLines->setWidth(_width);
                  newStaffLines->setParent(parent);
                  newStaffLines->setTrack(e->track());
                  newStaffLines->layout();
                  newStaffLines->draw(&painter);
                  delete newStaffLines;

                  //
                  // Draw barline
                  //
                  BarLine* newBarLine = new BarLine(_score);
                  newBarLine->setBarLineType(BarLineType::NORMAL);
                  newBarLine->setParent(parent);
                  newBarLine->setTrack(e->track());
                  newBarLine->setSpan(currentStaff->barLineSpan());
                  newBarLine->setSpanFrom(currentStaff->barLineFrom());
                  newBarLine->setSpanTo(currentStaff->barLineTo());
                  newBarLine->layout();
                  newBarLine->draw(&painter);
                  delete newBarLine;

                  //
                  // Draw the current Clef
                  //
                  Clef* newClef = new Clef(_score);
                  ClefType currentClef = currentStaff->clef(_currentMeasureTick);
                  newClef->setClefType(currentClef);
                  newClef->setParent(parent);
                  newClef->setTrack(e->track());
                  newClef->layout();
                  pos = QPointF(_score->styleP(StyleIdx::clefLeftMargin),0);
                  painter.translate(pos);
                  newClef->draw(&painter);
                  pos = QPointF(_widthClef,0);
                  painter.translate(pos);
                  delete newClef;

                  //
                  // Draw the current KeySignature
                  //
                  KeySig* newKs = new KeySig(_score);
                  KeySigEvent currentKeySigEvent = currentStaff->key(_currentMeasureTick);
                  newKs->setKeySigEvent(currentKeySigEvent);

                  // The Parent and the track must be set to have the key signature layout adjusted to different clefs
                  // This also adds naturals to the key signature (if set in the score style)
                  newKs->setParent(parent);
                  newKs->setTrack(e->track());

                  newKs->setHideNaturals(true);
                  pos = QPointF(_score->styleP(StyleIdx::keysigLeftMargin),0);
                  painter.translate(pos);
                  newKs->layout();
                  newKs->draw(&painter);
                  delete newKs;

                  pos = QPointF(_widthKeySig + _xPosTimeSig, 0);
                  painter.translate(pos);

                  //
                  // Draw the current TimeSignature
                  //
                  TimeSig* newTs = new TimeSig(_score);

                  // Try to get local time signature, if not, get the current measure one
                  TimeSig* currentTimeSig = currentStaff->timeSig(_currentMeasureTick);
                  if (currentTimeSig) {
                        newTs->setFrom(currentTimeSig);
                      //else
                        //    newTs->setSig(_currentTimeSig.numerator(), _currentTimeSig.denominator(), TimeSigType::NORMAL);
                      newTs->setParent(parent);
                      newTs->setTrack(e->track());
                      newTs->layout();
                      pos = QPointF(_score->styleP(StyleIdx::timesigLeftMargin),0);
                      painter.translate(pos);
                      newTs->draw(&painter);
                      delete newTs;
                      }
                  pos = QPointF(_offsetPanel + _heightName + _widthClef + _widthKeySig + _xPosTimeSig + _leftMarginTotal, e->pagePos().y());
                  painter.translate(-pos);
                  }
            }
      painter.restore();
      }
}
