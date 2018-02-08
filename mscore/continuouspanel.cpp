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
#include "libmscore/rest.h"
#include "libmscore/stafflines.h"

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
      }

//---------------------------------------------------------
//   paintContinousPanel
//---------------------------------------------------------

void ContinuousPanel::paint(const QRect&, QPainter& painter)
      {
      qreal _offsetPanel = 0;
      qreal _y = 0;
      qreal _oldWidth = 0;        // The last final panel width
      qreal _newWidth = 0;        // New panel width
      qreal _height = 0;
      qreal _leftMarginTotal = 0; // Sum of all elments left margin
      qreal _panelRightPadding = 5;  // Extra space for the panel after last element

      Measure* measure = _score->firstMeasure();

      if (!_active || !measure) {
            _visible = false;
            return;
            }

      if (measure->mmRest()) {
            measure = measure->mmRest();
            }

      System* system   = measure->system();
      if (system == 0) {
            _visible = false;
            return;
            }

      Segment* s      = measure->first();
      double _spatium = _score->spatium();
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
      _rect        = QRect(_offsetPanel + _width, _y, 1, _height);
      Page* page   = _score->pages().front();
      QList<Element*> el = page->items(_rect);
      if (el.empty()) {
            _visible = false;
            return;
            }
      qStableSort(el.begin(), el.end(), elementLessThan);

      const Measure*_currentMeasure = 0;
      for (const Element* e : el) {
            e->itemDiscovered = 0;
            if (!e->visible() && !_score->showInvisible())
                  continue;

            if (e->isMeasure()) {
                  _currentMeasure = toMeasure(e);
                  break;
                  }
            }
      if (!_currentMeasure)
            return;

      // Don't show panel if staff names are visible
      if (_currentMeasure == _score->firstMeasure() && _sv->toPhysical(_currentMeasure->canvasPos()).x() > 0) {
            _visible = false;
            return;
            }

      qreal _xPosMeasure       = _currentMeasure->canvasX();
      qreal _measureWidth      = _currentMeasure->width();
      int tick                 = _currentMeasure->tick();
      Fraction _currentTimeSig = _currentMeasure->timesig();
      //qDebug() << "_sv->xoffset()=" <<_sv->xoffset() << " _sv->mag()="<< _sv->mag() <<" s->x=" << s->x() << " width=" << _width << " currentMeasure=" << _currentMeasure->x() << " _xPosMeasure=" << _xPosMeasure;

      //---------------------------------------------------------
      //   findElementWidths
      //      determines the max width for each element types
      //---------------------------------------------------------

      // The first pass serves to get the maximum width for each elements

      qreal lineWidthName = 0;
      qreal _widthClef    = 0;
      qreal _widthKeySig  = 0;
      qreal _widthTimeSig = 0;
      qreal _xPosTimeSig  = 0;

      for (const Element* e : el) {
            e->itemDiscovered = 0;
            if (!e->visible() && !_score->showInvisible())
                  continue;

            if (e->isRest() && toRest(e)->isGap())
                  continue;

            if (e->isStaffLines()) {
                  Staff* currentStaff = _score->staff(e->staffIdx());
                  Segment* parent = _score->tick2segment(tick);

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
                  newName->setFamily("FreeSans");
                  newName->setSizeIsSpatiumDependent(true);
                  newName->layout();
                  newName->setPlainText(newName->plainText());
                  newName->layout();

                  // Find maximum width for the current Clef
                  Clef* newClef = new Clef(_score);
                  ClefType currentClef = currentStaff->clef(tick);
                  newClef->setClefType(currentClef);
                  newClef->setParent(parent);
                  newClef->setTrack(e->track());
                  newClef->layout();
                  if (newClef->width() > _widthClef)
                        _widthClef = newClef->width();

                  // Find maximum width for the current KeySignature
                  KeySig* newKs = new KeySig(_score);
                  KeySigEvent currentKeySigEvent = currentStaff->keySigEvent(tick);
                  newKs->setKeySigEvent(currentKeySigEvent);
                  // The Parent and the Track must be set to have the key signature layout adjusted to different clefs
                  // This also adds naturals to the key signature (if set in the score style)
                  newKs->setParent(parent);
                  newKs->setTrack(e->track());
                  newKs->setHideNaturals(true);
                  newKs->layout();
                  if (newKs->width() > _widthKeySig)
                        _widthKeySig = newKs->width();

                  // Find maximum width for the current TimeSignature
                  TimeSig* newTs = new TimeSig(_score);

                  // Try to get local time signature, if not, get the current measure one
                  TimeSig* currentTimeSig = currentStaff->timeSig(tick);
                  if (currentTimeSig)
                        newTs->setFrom(currentTimeSig);
                  else
                        newTs->setSig(Fraction(_currentTimeSig.numerator(), _currentTimeSig.denominator()), TimeSigType::NORMAL);
                  newTs->setParent(parent);
                  newTs->setTrack(e->track());
                  newTs->layout();

                  if ((newName->width() > lineWidthName) && (newName->xmlText() != ""))
                        lineWidthName = newName->width();

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

      //====================

      painter.save();

      // Draw colored rectangle
      painter.setClipping(false);
      QPointF pos(_offsetPanel, 0);

      painter.translate(pos);
      QPen pen;
      pen.setWidthF(0.0);
      pen.setStyle(Qt::NoPen);
      painter.setPen(pen);
      painter.setBrush(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
      QRectF bg(_rect);

      bg.setWidth(_widthClef + _widthKeySig + _widthTimeSig + _leftMarginTotal + _panelRightPadding);
      QPixmap* fgPixmap = _sv->fgPixmap();
      if (fgPixmap == 0 || fgPixmap->isNull())
            painter.fillRect(bg, preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
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
      // TODO: simplify (no Text element)
      QString text = QString("#%1").arg(_currentMeasure->no()+1);
      Text* newElement = new Text(_score);
      newElement->setFlag(ElementFlag::MOVABLE, false);
      newElement->setXmlText(text);
      newElement->setFamily("FreeSans");
      newElement->setSizeIsSpatiumDependent(true);
      newElement->setColor(color);
      newElement->layout1();
      pos = QPointF(_score->styleP(StyleIdx::clefLeftMargin) + _widthClef, _y + newElement->height());
      painter.translate(pos);
      newElement->draw(&painter);
      pos += QPointF(_offsetPanel, 0);
      painter.translate(-pos);
      delete newElement;

      // This second pass draws the elements spaced evently using the width of the largest element
      for (const Element* e : el) {
            if (!e->visible() && !_score->showInvisible())
                  continue;

            if (e->isRest() && toRest(e)->isGap())
                  continue;

            if (e->isStaffLines()) {
                  painter.save();
                  Staff* currentStaff = _score->staff(e->staffIdx());
                  Segment* parent = _score->tick2segmentMM(tick);

                  pos = QPointF (_offsetPanel, e->pagePos().y());
                  painter.translate(pos);

                  // Draw staff lines
                  StaffLines newStaffLines(*toStaffLines(e));
                  newStaffLines.setParent(parent);
                  newStaffLines.setTrack(e->track());
                  newStaffLines.layout();
                  newStaffLines.setColor(color);
                  newStaffLines.setWidth(bg.width());
                  newStaffLines.draw(&painter);

                  // Draw barline
                  BarLine barLine(_score);
                  barLine.setBarLineType(BarLineType::NORMAL);
                  barLine.setParent(parent);
                  barLine.setTrack(e->track());
                  barLine.setSpanStaff(currentStaff->barLineSpan());
                  barLine.setSpanFrom(currentStaff->barLineFrom());
                  barLine.setSpanTo(currentStaff->barLineTo());
                  barLine.layout();
                  barLine.setColor(color);
                  barLine.draw(&painter);

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
                  newName->setFamily("FreeSans");
                  newName->setSizeIsSpatiumDependent(true);
                  newName->layout();
                  newName->setPlainText(newName->plainText());
                  newName->layout();
                  if (currentStaff->part()->staff(0) == currentStaff) {
                        double _spatium = _score->spatium();
                        pos = QPointF (_score->styleP(StyleIdx::clefLeftMargin) + _widthClef, -_spatium * 2);
                        painter.translate(pos);
                        newName->draw(&painter);
                        painter.translate(-pos);
                        }
                  delete newName;

                  qreal posX = 0.0;

                  // Draw the current Clef
                  Clef clef(_score);
                  clef.setClefType(currentStaff->clef(tick));
                  clef.setParent(parent);
                  clef.setTrack(e->track());
                  clef.setColor(color);
                  clef.layout();
                  posX += _score->styleP(StyleIdx::clefLeftMargin);
                  clef.drawAt(&painter, QPointF(posX, clef.pos().y()));
                  posX += _widthClef;

                  // Draw the current KeySignature
                  KeySig newKs(_score);
                  newKs.setKeySigEvent(currentStaff->keySigEvent(tick));

                  // The Parent and the track must be set to have the key signature layout adjusted to different clefs
                  // This also adds naturals to the key signature (if set in the score style)
                  newKs.setParent(parent);
                  newKs.setTrack(e->track());
                  newKs.setColor(color);
                  newKs.setHideNaturals(true);
                  newKs.layout();
                  posX += _score->styleP(StyleIdx::keysigLeftMargin);
                  newKs.drawAt(&painter, QPointF(posX, 0.0));

                  posX += _widthKeySig + _xPosTimeSig;

                  // Draw the current TimeSignature
                  TimeSig newTs(_score);

                  // Try to get local time signature, if not, get the current measure one
                  TimeSig* currentTimeSig = currentStaff->timeSig(tick);
                  if (currentTimeSig) {
                        newTs.setFrom(currentTimeSig);
                        newTs.setParent(parent);
                        newTs.setTrack(e->track());
                        newTs.setColor(color);
                        newTs.layout();
                        posX += _score->styleP(StyleIdx::timesigLeftMargin);
                        newTs.drawAt(&painter, QPointF(posX, 0.0));
                        }
                  painter.restore();
                  }
            }
      painter.restore();
      _visible = true;
      }

}
