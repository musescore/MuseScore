//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of classes SysStaff and System.
*/

#include "system.h"
#include "measure.h"
#include "segment.h"
#include "score.h"
#include "sig.h"
#include "key.h"
#include "xml.h"
#include "clef.h"
#include "text.h"
#include "navigate.h"
#include "select.h"
#include "staff.h"
#include "part.h"
#include "page.h"
#include "style.h"
#include "bracket.h"
#include "mscore.h"
#include "barline.h"
#include "lyrics.h"
#include "system.h"
#include "box.h"
#include "chordrest.h"
#include "iname.h"
#include "spanner.h"

namespace Ms {

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

SysStaff::SysStaff()
      {
      idx   = 0;
      _show = true;
      }

//---------------------------------------------------------
//   ~SysStaff
//---------------------------------------------------------

SysStaff::~SysStaff()
      {
      qDeleteAll(instrumentNames);
      }

//---------------------------------------------------------
//   System
//---------------------------------------------------------

System::System(Score* s)
   : Element(s)
      {
      _barLine     = 0;
      _leftMargin  = 0.0;
      _pageBreak   = false;
      _firstSystem = false;
      _vbox        = false;
      _sameLine    = false;
      _addStretch  = false;
      }

//---------------------------------------------------------
//   ~System
//---------------------------------------------------------

System::~System()
      {
      delete _barLine;
      qDeleteAll(_staves);
      qDeleteAll(_brackets);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

SysStaff* System::insertStaff(int idx)
      {
      SysStaff* staff = new SysStaff;
      if (idx) {
            // HACK: guess position
            staff->rbb().setY(_staves[idx-1]->y() + 6 * spatium());
            }
      _staves.insert(idx, staff);
      return staff;
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void System::removeStaff(int idx)
      {
      _staves.takeAt(idx);
      }

//---------------------------------------------------------
//   layout
///   Layout the System
//    If first MeasureBase is a HBOX, then xo1 is the
//    width of this box.
//---------------------------------------------------------

void System::layout(qreal xo1)
      {
      if (isVbox())                 // ignore vbox
            return;
      static const Spatium instrumentNameOffset(1.0);

      int nstaves  = _staves.size();
      if (nstaves != score()->nstaves())
            qDebug("System::layout: nstaves %d != %d", nstaves, score()->nstaves());

      //---------------------------------------------------
      //  find x position of staves
      //    create brackets
      //---------------------------------------------------

      qreal xoff2 = 0.0;         // x offset for instrument name

      int bracketLevels = 0;
      for (int idx = 0; idx < nstaves; ++idx)
            bracketLevels = qMax(bracketLevels, score()->staff(idx)->bracketLevels());

      qreal bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      QList<Bracket*> bl = _brackets;
      _brackets.clear();

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* s = score()->staff(staffIdx);
            for (int i = 0; i < bracketLevels; ++i) {
                  if (s->bracket(i) == BracketType::NO_BRACKET)
                        continue;
                  int firstStaff = staffIdx;
                  int lastStaff  = staffIdx + s->bracketSpan(i) - 1;
                  if (lastStaff >= nstaves)
                        lastStaff = nstaves - 1;

                  for (; firstStaff <= lastStaff; ++firstStaff) {
                        if (score()->staff(firstStaff)->show())
                              break;
                        }
                  for (; lastStaff >= firstStaff; --lastStaff) {
                        if (score()->staff(lastStaff)->show())
                              break;
                        }
                  int span = lastStaff - firstStaff + 1;
                  //
                  // do not show bracket if it only spans one
                  // system due to some invisible staves
                  //
                  if ((span > 1) || (s->bracketSpan(i) == span)) {
                        //
                        // this bracket is visible
                        //
                        Bracket* b = 0;
                        int track = staffIdx * VOICES;
                        for (int k = 0; k < bl.size(); ++k) {
                              if (bl[k]->track() == track && bl[k]->level() == i) {
                                    b = bl.takeAt(k);
                                    break;
                                    }
                              }
                        if (b == 0) {
                              b = new Bracket(score());
                              b->setGenerated(true);
                              b->setParent(this);
                              b->setTrack(track);
                              b->setLevel(i);
                              b->setBracketType(s->bracket(i));
                              b->setSpan(s->bracketSpan(i));
                              score()->undoAddElement(b);
                              }
                        else
                              _brackets.append(b);
                        b->setFirstStaff(firstStaff);
                        b->setLastStaff(lastStaff);
                        bracketWidth[i] = qMax(bracketWidth[i], b->width());
                        }
                  }
            if (!s->show())
                  continue;
            for (InstrumentName* t : _staves[staffIdx]->instrumentNames) {
                  t->layout();
                  qreal w = t->width() + point(instrumentNameOffset);
                  if (w > xoff2)
                        xoff2 = w;
                  }
            }

      for (Bracket* b : bl)
            score()->undoRemoveElement(b);

      //---------------------------------------------------
      //  layout  SysStaff and StaffLines
      //---------------------------------------------------

      // xoff2 += xo1;
      _leftMargin = xoff2;

      qreal bd = point(score()->styleS(StyleIdx::bracketDistance));
      if ( _brackets.size() > 0) {
            for (int i = 0; i < bracketLevels; ++i)
                  _leftMargin += bracketWidth[i] + bd;
            }


      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* s  = _staves[staffIdx];
            Staff* staff = score()->staff(staffIdx);
            if (!staff->show() || !s->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            qreal staffMag = staff->mag();
            qreal h;
            if (staff->lines() == 1)
                  h = 2;
            else
                  h = (staff->lines()-1) * staff->lineDistance();
            h = h * staffMag * spatium();
            s->bbox().setRect(_leftMargin + xo1, 0.0, 0.0, h);
            }

      if ((nstaves > 1 && score()->styleB(StyleIdx::startBarlineMultiple)) || (nstaves <= 1 && score()->styleB(StyleIdx::startBarlineSingle))) {
            if (_barLine == 0) {
                  BarLine* bl = new BarLine(score());
                  bl->setParent(this);
                  bl->setTrack(0);
                  bl->setGenerated(true);
                  score()->undoAddElement(bl);
                  }
            }
      else if (_barLine)
            score()->undoRemoveElement(_barLine);
      if (_barLine)
            _barLine->rxpos() = _leftMargin + xo1;

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      for (Bracket* b : _brackets) {
            qreal xo = -xo1;
            for (const Bracket* b2 : _brackets) {
                   if (b->level() > b2->level() &&
                      ((b->firstStaff() >= b2->firstStaff() && b->firstStaff() <= b2->lastStaff()) ||
                      (b->lastStaff() >= b2->firstStaff() && b->lastStaff() <= b2->lastStaff())))
                        xo += b2->width() + bd;
                   }
            b->rxpos() = _leftMargin - xo - b->width();
            }

      //---------------------------------------------------
      //  layout instrument names x position
      //---------------------------------------------------

      int idx = 0;
      for (const Part* p : score()->parts()) {
            SysStaff* s = staff(idx);
            if (s->show() && p->show()) {
                  for (InstrumentName* t : s->instrumentNames) {
                        switch (t->textStyle().align() & AlignmentFlags::HMASK) {
                              case int(AlignmentFlags::LEFT):
                                    t->rxpos() = 0;
                                    break;
                              case int(AlignmentFlags::HCENTER):
                                    t->rxpos() = (xoff2 - point(instrumentNameOffset) + xo1) * .5;
                                    break;
                              case int(AlignmentFlags::RIGHT):
                              default:
                                    t->rxpos() = xoff2 - point(instrumentNameOffset) + xo1;
                                    break;
                              }
                        t->rxpos() += t->textStyle().offset(t->spatium()).x();
                        }
                  }
            idx += p->nstaves();
            }
      }

//---------------------------------------------------------
//   layout2
//    called after measure layout
//    adjusts staff distance
//---------------------------------------------------------

void System::layout2()
      {
      if (isVbox())                 // ignore vbox
            return;

      int nstaves    = _staves.size();
      qreal _spatium = spatium();

      qreal y = 0.0;
      int lastStaffIdx  = 0;   // last visible staff
      int firstStaffIdx = -1;
      qreal lastStaffDistanceDown = 0.0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* staff = score()->staff(staffIdx);
            StyleIdx downDistance;
            qreal userDist = 0.0;
            if ((staffIdx + 1) == nstaves) {
                  //
                  // last staff in system
                  //
                  MeasureBase* mb = ml.last();
                  bool nextMeasureIsVBOX = false;
                  if (mb->next()) {
                        Element::Type type = mb->next()->type();
                        if (type == Element::Type::VBOX || type == Element::Type::TBOX || type == Element::Type::FBOX)
                              nextMeasureIsVBOX = true;
                        }
                  downDistance = nextMeasureIsVBOX ? StyleIdx::systemFrameDistance : StyleIdx::minSystemDistance;
                  }
            else if (staff->rstaff() < (staff->part()->staves()->size()-1)) {
                  //
                  // staff is not last staff in a part
                  //
                  downDistance = StyleIdx::akkoladeDistance;
                  userDist = score()->staff(staffIdx + 1)->userDist();
                  }
            else {
                  downDistance = StyleIdx::staffDistance;
                  userDist = score()->staff(staffIdx + 1)->userDist();
                  }

            SysStaff* s    = _staves[staffIdx];
            qreal distDown = score()->styleS(downDistance).val() * _spatium + userDist;
            qreal nominalDistDown = distDown;
            qreal distUp   = 0.0;
            int n = ml.size();
            for (int i = 0; i < n; ++i) {
                  MeasureBase* m = ml.at(i);
                  distDown = qMax(distDown, m->distanceDown(staffIdx));
                  distUp   = qMax(distUp,   m->distanceUp(staffIdx));
                  }
            s->setDistanceDown(distDown);
            s->setDistanceUp(distUp);

            if (!staff->show() || !s->show()) {
                  s->setbbox(QRectF());  // already done in layout() ?
                  continue;
                  }
            qreal sHeight = staff->height();
            qreal dup = staffIdx == 0 ? 0.0 : s->distanceUp();
            if (staff->lines() == 1)
                  dup -= _spatium * staff->mag();
            s->bbox().setRect(_leftMargin, y + dup, width() - _leftMargin, sHeight);
            y += dup + sHeight + s->distanceDown();
            lastStaffIdx = staffIdx;
            lastStaffDistanceDown = distDown - nominalDistDown;
            if (firstStaffIdx == -1)
                  firstStaffIdx = staffIdx;
            }
      if (firstStaffIdx == -1)
            firstStaffIdx = 0;

      qreal systemHeight = staff(lastStaffIdx)->bbox().bottom();
      if (lastStaffIdx < nstaves - 1)
            systemHeight += lastStaffDistanceDown;
      setHeight(systemHeight);

      int n = ml.size();
      for (int i = 0; i < n; ++i) {
            MeasureBase* m = ml.at(i);
            if (m->type() == Element::Type::MEASURE) {
                  // note that the factor 2 * _spatium must be corrected for when exporting
                  // system distance in MusicXML (issue #24733)
                  m->bbox().setRect(0.0, -_spatium, m->width(), systemHeight + 2 * _spatium);
                  }
            else if (m->type() == Element::Type::HBOX) {
                  m->bbox().setRect(0.0, 0.0, m->width(), systemHeight);
                  static_cast<HBox*>(m)->layout2();
                  }
            }

      if (_barLine) {
            _barLine->setTrack(firstStaffIdx * VOICES);
            _barLine->setSpan(lastStaffIdx - firstStaffIdx + 1);
            if (score()->staff(0)->lines() == 1)
                  _barLine->setSpanFrom(BARLINE_SPAN_1LINESTAFF_FROM);

            int spanTo = (score()->staff(lastStaffIdx)->lines() == 1) ?
                              BARLINE_SPAN_1LINESTAFF_TO :
                              (score()->staff(lastStaffIdx)->lines()-1)*2;
            _barLine->setSpanTo(spanTo);
            _barLine->layout();
            }

      //---------------------------------------------------
      //  layout brackets vertical position
      //---------------------------------------------------

      n = _brackets.size();
      for (int i = 0; i < n; ++i) {
            Bracket* b = _brackets.at(i);
            int staffIdx1 = b->firstStaff();
            int staffIdx2 = b->lastStaff();
            qreal sy = 0;                       // assume bracket not visible
            qreal ey = 0;
            // if start staff not visible, try next staff
            while (staffIdx1 <= staffIdx2 && !_staves[staffIdx1]->show())
                  ++staffIdx1;
            // if end staff not visible, try prev staff
            while (staffIdx1 <= staffIdx2 && !_staves[staffIdx2]->show())
                  --staffIdx2;
            // the bracket will be shown IF:
            // it spans at least 2 visible staves (staffIdx1 < staffIdx2) OR
            // it spans just one visible staff (staffIdx1 == staffIdx2) but it is required to do so
            // (the second case happens at least when the bracket is initially dropped)
            bool notHidden = (staffIdx1 < staffIdx2) || (b->span() == 1 && staffIdx1 == staffIdx2);
            if (notHidden) {                    // set vert. pos. and height to visible spanned staves
                  sy = _staves[staffIdx1]->bbox().top();
                  ey = _staves[staffIdx2]->bbox().bottom();
                  }
            b->rypos() = sy;
            b->setHeight(ey - sy);
            b->layout();
            }

      //---------------------------------------------------
      //  layout instrument names
      //---------------------------------------------------

      int staffIdx = 0;
      n = score()->parts().size();

      for (Part* p : score()->parts()) {
            SysStaff* s = staff(staffIdx);
            SysStaff* s2;
            int nstaves = p->nstaves();
            if (s->show()) {
                  for (InstrumentName* t : s->instrumentNames) {
                        //
                        // override Text->layout()
                        //
                        qreal y1, y2;
                        switch (t->layoutPos()) {
                              default:
                              case 0:           // center at part
                                    y1 = s->bbox().top();
                                    s2 = staff(staffIdx);
                                    for (int i = staffIdx + nstaves - 1; i > 0; --i) {
                                          SysStaff* s = staff(i);
                                          if (s->show()) {
                                                s2 = s;
                                                break;
                                                }
                                          }
                                    y2 = s2->bbox().bottom();
                                    break;
                              case 1:           // center at first staff
                                    y1 = s->bbox().top();
                                    y2 = s->bbox().bottom();
                                    break;

                              // TODO:
                              // sort out invisible staves

                              case 2:           // center between first and second staff
                                    y1 = s->bbox().top();
                                    y2 = staff(staffIdx + 1)->bbox().bottom();
                                    break;
                              case 3:           // center at second staff
                                    y1 = staff(staffIdx + 1)->bbox().top();
                                    y2 = staff(staffIdx + 1)->bbox().bottom();
                                    break;
                              case 4:           // center between first and second staff
                                    y1 = staff(staffIdx + 1)->bbox().top();
                                    y2 = staff(staffIdx + 2)->bbox().bottom();
                                    break;
                              case 5:           // center at third staff
                                    y1 = staff(staffIdx + 2)->bbox().top();
                                    y2 = staff(staffIdx + 2)->bbox().bottom();
                                    break;
                              }
                        t->rypos() = y1 + (y2 - y1) * .5 + t->textStyle().offset(t->spatium()).y();
                        }
                  }
            staffIdx += nstaves;
            }
      }

//---------------------------------------------------------
///   clear
///   Clear layout of System
//---------------------------------------------------------

void System::clear()
      {
      ml.clear();
      foreach (SpannerSegment* ss, _spannerSegments) {
            // qDebug("System::clear %s", ss->name());
            if (ss->system() == this)
                  ss->setParent(0);       // assume parent() is System
            }
      _spannerSegments.clear();
      _vbox        = false;
      _firstSystem = false;
      _pageBreak   = false;
      }

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void System::setInstrumentNames(bool longName)
      {
      //
      // remark: add/remove instrument names is not undo/redoable
      //         as add/remove of systems is not undoable
      //
      if (isVbox())                 // ignore vbox
            return;
      if (!score()->showInstrumentNames()
              || (score()->styleB(StyleIdx::hideInstrumentNameIfOneInstrument) && score()->parts().size() == 1)) {
            for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                  SysStaff* staff = _staves[staffIdx];
                  foreach(InstrumentName* t, staff->instrumentNames)
                        score()->removeElement(t);
                  }
            return;
            }

      int tick = ml.isEmpty() ? 0 : ml.front()->tick();
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            SysStaff* staff = _staves[staffIdx];
            Staff* s        = score()->staff(staffIdx);
            if (!s->isTop() || !s->show()) {
                  foreach(InstrumentName* t, staff->instrumentNames)
                        score()->removeElement(t);
                  continue;
                  }

            Part* part = s->part();
            const QList<StaffName>& names = longName? part->longNames(tick) : part->shortNames(tick);

            int idx = 0;
            foreach(const StaffName& sn, names) {
                  InstrumentName* iname = staff->instrumentNames.value(idx);
                  if (iname == 0) {
                        iname = new InstrumentName(score());
                        iname->setGenerated(true);
                        iname->setParent(this);
                        iname->setTrack(staffIdx * VOICES);
                        iname->setInstrumentNameType(longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT);
                        score()->addElement(iname);
                        }
                  iname->setText(sn.name);
                  iname->setLayoutPos(sn.pos);
                  ++idx;
                  }
            for (; idx < staff->instrumentNames.size(); ++idx)
                  score()->removeElement(staff->instrumentNames[idx]);
            }
      }

//---------------------------------------------------------
//   y2staff
//---------------------------------------------------------

/**
 Return staff number for canvas relative y position \a y
 or -1 if not found.

 To allow drag and drop above and below the staff, the actual y range
 considered "inside" the staff is increased by "margin".
*/

int System::y2staff(qreal y) const
      {
      y -= pos().y();
      int idx = 0;
      qreal margin = spatium() * 2;
      foreach (SysStaff* s, _staves) {
            qreal y1 = s->bbox().top()    - margin;
            qreal y2 = s->bbox().bottom() + margin;
            if (y >= y1 && y < y2)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void System::add(Element* el)
      {
// qDebug("%p System::add: %p %s", this, el, el->name());

      el->setParent(this);
      switch(el->type()) {
            case Element::Type::INSTRUMENT_NAME:
// qDebug("  staffIdx %d, staves %d", el->staffIdx(), _staves.size());
                  _staves[el->staffIdx()]->instrumentNames.append(static_cast<InstrumentName*>(el));
                  break;

            case Element::Type::BEAM:
                  score()->addElement(el);
                  break;

            case Element::Type::BRACKET:
                  {
                  Bracket* b   = static_cast<Bracket*>(el);
                  int staffIdx = b->staffIdx();
                  int level    = b->level();
                  if (level == -1) {
                        level = 0;
                        foreach(Bracket* bb, _brackets) {
                              if (staffIdx >= bb->firstStaff() && staffIdx <= bb->lastStaff())
                                    ++level;
                              }
                        b->setLevel(level);
                        b->setSpan(1);
                        }
//                  b->staff()->setBracket(level,     b->bracketType());
//                  b->staff()->setBracketSpan(level, b->span());
                  _brackets.append(b);
                  }
                  break;

            case Element::Type::MEASURE:
            case Element::Type::HBOX:
            case Element::Type::VBOX:
            case Element::Type::TBOX:
            case Element::Type::FBOX:
                  score()->addElement(static_cast<MeasureBase*>(el));
                  break;
            case Element::Type::TEXTLINE_SEGMENT:
            case Element::Type::HAIRPIN_SEGMENT:
            case Element::Type::OTTAVA_SEGMENT:
            case Element::Type::TRILL_SEGMENT:
            case Element::Type::VOLTA_SEGMENT:
            case Element::Type::SLUR_SEGMENT:
            case Element::Type::PEDAL_SEGMENT:
                  {
                  SpannerSegment* ss = static_cast<SpannerSegment*>(el);
#ifndef NDEBUG
                  if (_spannerSegments.contains(ss))
                        qDebug("System::add() %s %p already there", ss->name(), ss);
                  else
#endif
                  _spannerSegments.append(ss);
                  }
                  break;
            case Element::Type::BAR_LINE:
                  if (_barLine)
                        qDebug("%p System::add(%s, %p): there is already a barline %p", this, el->name(), el, _barLine);
                  _barLine = static_cast<BarLine*>(el);
                  break;
            default:
                  qDebug("System::add(%s) not implemented", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void System::remove(Element* el)
      {
      switch (el->type()) {
            case Element::Type::INSTRUMENT_NAME:
                  _staves[el->staffIdx()]->instrumentNames.removeOne(static_cast<InstrumentName*>(el));
                  break;
            case Element::Type::BEAM:
                  score()->removeElement(el);
                  break;
            case Element::Type::BRACKET:
                  {
                  Bracket* b = static_cast<Bracket*>(el);
                  if (!_brackets.removeOne(b))
                        qDebug("System::remove: bracket not found");
//                  b->staff()->setBracket(b->level(), NO_BRACKET);
                  }
                  break;
            case Element::Type::MEASURE:
            case Element::Type::HBOX:
            case Element::Type::VBOX:
            case Element::Type::TBOX:
            case Element::Type::FBOX:
                  score()->removeElement(el);
                  break;
            case Element::Type::TEXTLINE_SEGMENT:
            case Element::Type::HAIRPIN_SEGMENT:
            case Element::Type::OTTAVA_SEGMENT:
            case Element::Type::TRILL_SEGMENT:
            case Element::Type::VOLTA_SEGMENT:
            case Element::Type::SLUR_SEGMENT:
            case Element::Type::PEDAL_SEGMENT:
                  if (!_spannerSegments.removeOne(static_cast<SpannerSegment*>(el))) {
                        qDebug("System::remove: %p(%s) not found, score %p", el, el->name(), score());
                        Q_ASSERT(score() == el->score());
                        }
                  break;
            case Element::Type::BAR_LINE:
                  if (_barLine == 0)
                        qDebug("System::remove(%s): there is no barline", el->name());
                  _barLine = 0;
                  break;
            default:
                  qDebug("System::remove(%s) not implemented", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void System::change(Element* o, Element* n)
      {
      if (o->type() == Element::Type::VBOX || o->type() == Element::Type::HBOX || o->type() == Element::Type::TBOX || o->type() == Element::Type::FBOX) {
            int idx = ml.indexOf((MeasureBase*)o);
            if (idx != -1)
                  ml.removeAt(idx);
            ml.insert(idx, (MeasureBase*)n);
            score()->measures()->change((MeasureBase*)o, (MeasureBase*)n);
            }
      else {
            remove(o);
            add(n);
            }
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snap(int tick, const QPointF p) const
      {
      foreach(const MeasureBase* m, ml) {
            if (p.x() < m->x() + m->width())
                  return ((Measure*)m)->snap(tick, p - m->pos()); //TODO: MeasureBase
            }
      return ((Measure*)ml.back())->snap(tick, p-pos());          //TODO: MeasureBase
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snapNote(int tick, const QPointF p, int staff) const
      {
      foreach(const MeasureBase* m, ml) {
            if (p.x() < m->x() + m->width())
                  return ((Measure*)m)->snapNote(tick, p - m->pos(), staff);  //TODO: MeasureBase
            }
      return ((Measure*)ml.back())->snap(tick, p-pos());          // TODO: MeasureBase
      }

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* System::firstMeasure() const
      {
      if (ml.isEmpty())
            return 0;
      for (MeasureBase* mb = ml.front(); mb; mb = mb->nextMM()) {
            if (mb->type() != Element::Type::MEASURE)
                  continue;
            return static_cast<Measure*>(mb);
            }
      return 0;
      }

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* System::lastMeasure() const
      {
      if (ml.isEmpty())
            return 0;
      for (MeasureBase* mb = ml.back(); mb; mb = mb->prev()) {
            if (mb->type() != Element::Type::MEASURE)
                  continue;
            return static_cast<Measure*>(mb);
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

MeasureBase* System::prevMeasure(const MeasureBase* m) const
      {
      if (m == ml.front())
            return 0;
      return m->prev();
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

MeasureBase* System::nextMeasure(const MeasureBase* m) const
      {
      if (m == ml.back())
            return 0;
      return m->next();
      }

//---------------------------------------------------------
//   searchNextLyrics
//---------------------------------------------------------

static Lyrics* searchNextLyrics(Segment* s, int staffIdx, int verse)
      {
      Lyrics* l = 0;
      while ((s = s->next1(Segment::Type::ChordRest))) {
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            QList<Lyrics*>* nll = 0;
            for (int track = strack; track < etrack; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  if (cr && !cr->lyricsList().isEmpty()) {
                        nll = &cr->lyricsList();
                        break;
                        }
                  }
            if (!nll)
                  continue;
            l = nll->value(verse);
            if (l)
                  break;
            }
      return l;
      }

//---------------------------------------------------------
//   layoutLyrics
//    layout lyrics separator
//---------------------------------------------------------

void System::layoutLyrics(Lyrics* l, Segment* s, int staffIdx)
      {
      if ((l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END) && (l->ticks() == 0)) {
            l->clearSeparator();
            return;
            }
      qreal _spatium = spatium();

      const TextStyle& ts = l->textStyle();
      qreal lmag          = qreal(ts.size()) / 11.0;
      qreal staffMag      = l->staff()->mag();

      if (l->ticks()) {
            // melisma
            Segment* seg = score()->tick2segment(l->endTick());
            if (seg == 0) {
                  qDebug("System::layoutLyrics: no segment found for tick %d", l->endTick());
                  return;
                  }

            QList<Line*>* sl = l->separatorList();
            QList<System*>* systems = score()->systems();
            System* s1  = this;
            System* s2  = seg->measure()->system();
            int sysIdx1 = systems->indexOf(s1);
            int sysIdx2 = systems->indexOf(s2);

            qreal  x1 = l->bbox().right();      // lyrics width
            QPointF p1(x1, 0);                  // melisma y is at base line

            int segIdx = 0;
            for (int i = sysIdx1; i <= sysIdx2; ++i, ++segIdx) {
                  System* system = (*systems)[i];
                  Line* line = sl->value(segIdx);
                  if (line == 0) {
                        line = new Line(l->score(), false);
                        l->add(line);
                        }
                  line->setLineWidth(Spatium(0.1 * lmag * staffMag));
                  line->setPos(p1);
                  if (sysIdx1 == sysIdx2) {
                        // single segment
                        qreal headWidth = score()->noteHeadWidth();
                        qreal len = seg->pagePos().x() - l->pagePos().x() - x1 + headWidth;
                        line->setLen(Spatium(len / _spatium));
                        Lyrics* nl = searchNextLyrics(seg, staffIdx, l->no());
                        // small correction if next lyrics is moved? not needed if on another system
                        if (nl && nl->measure()->system() == s1) {
                              qreal x2  = nl->bbox().left() + nl->pagePos().x();
                              qreal lx2 = line->pagePos().x() + len;
                              //qDebug("Line %f  text %f", lx2, x2);
                              if (lx2 > x2)
                                    len -= (lx2 - x2);
                              }
                        line->setLen(Spatium(len / _spatium));
                        }
                  else if (i == sysIdx1) {
                        // start segment
                        qreal w   = system->staff(l->staffIdx())->right();
                        qreal x   = system->pagePos().x() + w;
                        qreal len = x - l->pagePos().x() - x1;
                        line->setLen(Spatium(len / _spatium));
                        }
                  else if (i > 0 && i != sysIdx2) {
qDebug("Lyrics: melisma middle segment not implemented");
                        // middle segment
                        }
                  else if (i == sysIdx2) {
                        // end segment
qDebug("Lyrics: melisma end segment not implemented");
                        }
                  line->layout();
                  }
            return;
            }
      //
      // we have to layout a separator to the next
      // Lyric syllable
      //
      int verse   = l->no();
      Segment* ns = s;

      // TODO: the next two values should be style parameters
      // TODO: as well as the 0.3 factor a few lines below
      const qreal maxl = 0.5 * _spatium * lmag * staffMag;   // lyrics hyphen length
      const Spatium hlw(0.14 * lmag * staffMag);              // hyphen line width

      Lyrics* nl = searchNextLyrics(ns, staffIdx, verse);
      if (!nl) {
            l->clearSeparator();
            return;
            }
      QList<Line*>* sl = l->separatorList();
      Line* line;
      if (sl->isEmpty()) {
            line = new Line(l->score(), false);
            l->add(line);
            }
      else {
            line = (*sl)[0];
            }
      qreal x = l->bbox().right();
      // convert font size to raster units, scaling if spatium-dependent
      qreal size = ts.size();
      if(ts.sizeIsSpatiumDependent())
            size *= _spatium / (SPATIUM20 * PPI);    // <= (MScore::DPI / PPI) * (_spatium / (SPATIUM20 * Mscore::DPI))
      else
            size *= MScore::DPI / PPI;
      qreal y = -size * staffMag * 0.3;                    // a conventional percentage of the whole font height

      qreal x1 = x + l->pagePos().x();
      qreal x2 = nl->bbox().left() + nl->pagePos().x();
      qreal len;
      if (x2 < x1 || s->measure()->system()->page() != ns->measure()->system()->page()) {
            System* system = s->measure()->system();
            x2 = system->pagePos().x() + system->bbox().width();
            }

      qreal gap = x2 - x1;
      len       = gap;
      if (len > maxl)
            len = maxl;
      qreal xo = (gap - len) * .5;

      line->setLineWidth(hlw);
      line->setPos(QPointF(x + xo, y));
      line->setLen(Spatium(len / _spatium));
      line->layout();
      }

//---------------------------------------------------------
//   scanElements
//    collect all visible elements
//---------------------------------------------------------

void System::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (isVbox())
            return;
      if (_barLine)
            func(data, _barLine);

      for (Bracket* b : _brackets)
            func(data, b);

      int idx = 0;
      for (const SysStaff* st : _staves) {
            if (all || st->show()) {
                  for (InstrumentName* t : st->instrumentNames)
                        func(data, t);
                  }
            ++idx;
            }
      for (SpannerSegment* ss : _spannerSegments) {
            int staffIdx = ss->spanner()->staffIdx();
            if (staffIdx == -1) {
                  qDebug("System::scanElements: staffIDx == -1: %s %p", ss->spanner()->name(), ss->spanner());
                  staffIdx = 0;
                  }
            bool v = true;
            Spanner* spanner = ss->spanner();
            if (spanner->anchor() == Spanner::Anchor::SEGMENT || spanner->anchor() == Spanner::Anchor::CHORD) {
                  Element* se = spanner->startElement();
                  Element* ee = spanner->endElement();
                  bool v1 = true;
                  if (se && (se->type() == Element::Type::CHORD || se->type() == Element::Type::REST)) {
                        ChordRest* cr = static_cast<ChordRest*>(se);
                        Measure* m    = cr->measure();
                        MStaff* mstaff = m->mstaff(cr->staffIdx());
                        v1 = mstaff->visible();
                        }
                  bool v2 = true;
                  if (!v1 && ee && (ee->type() == Element::Type::CHORD || ee->type() == Element::Type::REST)) {
                        ChordRest* cr = static_cast<ChordRest*>(ee);
                        Measure* m    = cr->measure();
                        MStaff* mstaff = m->mstaff(cr->staffIdx());
                        v2 = mstaff->visible();
                        }
                  v = v1 || v2; // hide spanner if both chords are hidden
                  }
            if (all || (score()->staff(staffIdx)->show() && v) || (spanner->type() == Element::Type::VOLTA))
                  ss->scanElements(data, func, all);
            }
      }

//---------------------------------------------------------
//   staffYpage
//    return page coordinates
//---------------------------------------------------------

qreal System::staffYpage(int staffIdx) const
      {
      if (_staves.size() <= staffIdx || staffIdx < 0) {
            qDebug("staffY: staves %d: bad staffIdx %d, vbox %d",
               _staves.size(), staffIdx, _vbox);
//            abort();
            return pagePos().y();
            }
      return _staves[staffIdx]->y() + y(); // pagePos().y();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void System::write(Xml& xml) const
      {
      xml.stag("System");
      if (_barLine && !_barLine->generated())
            _barLine->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void System::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "BarLine") {
                  _barLine = new BarLine(score());
                  _barLine->read(e);
                  _barLine->setTrack(0);
                  _barLine->setParent(this);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* System::nextElement()
      {
      Measure* m = firstMeasure();
      if (m) {
            Segment* firstSeg = m->segments()->first();
            if (firstSeg)
                  return firstSeg->element(0);
            }
      return score()->firstElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* System::prevElement()
      {
      Segment* seg = firstMeasure()->first();
      Element* re = 0;
      while (!re) {
            seg = seg->prev1MM();
            if (!seg)
                  return score()->lastElement();

            if (seg->segmentType() == Segment::Type::EndBarLine)
                  score()->inputState().setTrack((score()->staves().size() - 1) * VOICES); //corection

            re = seg->lastElement(score()->staves().size() - 1);
            }
      return re;
      }
}

