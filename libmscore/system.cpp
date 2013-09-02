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
                  if (s->bracket(i) == NO_BRACKET)
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
            foreach(InstrumentName* t, _staves[staffIdx]->instrumentNames) {
                  t->layout();
                  qreal w = t->width() + point(instrumentNameOffset);
                  if (w > xoff2)
                        xoff2 = w;
                  }
            }

      for (Bracket* b : bl)
            score()->undoRemoveElement(b);
//      qDeleteAll(bl);   // delete unused brackets

      //---------------------------------------------------
      //  layout  SysStaff and StaffLines
      //---------------------------------------------------

      // xoff2 += xo1;
      _leftMargin = xoff2;

      qreal bd = point(score()->styleS(ST_bracketDistance));
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
            s->bbox().setRect(_leftMargin + xo1, 0.0, 0.0,
               (staff->lines()-1) * staff->lineDistance() * spatium() * staffMag);
            }

      if ((nstaves > 1 && score()->styleB(ST_startBarlineMultiple)) || (nstaves <= 1 && score()->styleB(ST_startBarlineSingle))) {
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

      foreach (Bracket* b, _brackets) {
            qreal xo     = -xo1;
            int level   = b->level();
            for (int i = 0; i < level; ++i)
                  xo += bracketWidth[i] + bd;
            b->rxpos() = _leftMargin - xo - b->width();
            }

      //---------------------------------------------------
      //  layout instrument names x position
      //---------------------------------------------------

      int idx = 0;
      foreach (const Part* p, score()->parts()) {
            SysStaff* s = staff(idx);
            int nstaves = p->nstaves();
            if (s->show() && p->show()) {
                  foreach(InstrumentName* t, s->instrumentNames) {
                        t->rxpos() = xoff2 - point(instrumentNameOffset) + xo1;
                        }
                  }
            idx += nstaves;
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
                        int type = mb->next()->type();
                        if (type == VBOX || type == TBOX || type == FBOX)
                              nextMeasureIsVBOX = true;
                        }
                  downDistance = nextMeasureIsVBOX ? ST_systemFrameDistance : ST_minSystemDistance;
                  }
            else if (staff->rstaff() < (staff->part()->staves()->size()-1)) {
                  //
                  // staff is not last staff in a part
                  //
                  downDistance = ST_akkoladeDistance;
                  userDist = score()->staff(staffIdx + 1)->userDist();
                  }
            else {
                  downDistance = ST_staffDistance;
                  userDist = score()->staff(staffIdx + 1)->userDist();
                  }

            SysStaff* s    = _staves[staffIdx];
            qreal distDown = score()->styleS(downDistance).val() * _spatium + userDist;
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
            qreal sHeight = staff->height();   // (staff->lines() - 1) * _spatium * staffMag;
            qreal dup = staffIdx == 0 ? 0.0 : s->distanceUp();
            s->bbox().setRect(_leftMargin, y + dup, width() - _leftMargin, sHeight);
            y += dup + sHeight + s->distanceDown();
            lastStaffIdx = staffIdx;
            if (firstStaffIdx == -1)
                  firstStaffIdx = staffIdx;
            }
      if (firstStaffIdx == -1)
            firstStaffIdx = 0;

      qreal systemHeight = staff(lastStaffIdx)->bbox().bottom();
      setHeight(systemHeight);

      int n = ml.size();
      for (int i = 0; i < n; ++i) {
            MeasureBase* m = ml.at(i);
            if (m->type() == MEASURE) {
                  m->bbox().setRect(0.0, -_spatium, m->width(), systemHeight + 2 * _spatium);
                  }
            else if (m->type() == HBOX) {
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
            qreal sy = _staves[b->firstStaff()]->bbox().top();
            qreal ey = _staves[b->lastStaff()]->bbox().bottom();
            b->rypos() = sy;
            b->setHeight(ey - sy);
            b->layout();
            }

      //---------------------------------------------------
      //  layout instrument names
      //---------------------------------------------------

      int staffIdx = 0;
      n = score()->parts().size();
      for (int i = 0; i < n; ++i) {
            Part* p = score()->parts().at(i);
            SysStaff* s = staff(staffIdx);
            int nstaves = p->nstaves();
            int nn = s->instrumentNames.size();
            for (int k = 0; k < nn; ++k) {
                  InstrumentName* t = s->instrumentNames.at(k);
                  //
                  // override Text->layout()
                  //
                  qreal y1, y2;
                  switch(t->layoutPos()) {
                        default:
                        case 0:           // center at part
                              y1 = s->bbox().top();
                              y2 = staff(staffIdx + nstaves - 1)->bbox().bottom();
                              break;
                        case 1:           // center at first staff
                              y1 = s->bbox().top();
                              y2 = s->bbox().bottom();
                              break;
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
                  qreal y  = y1 + (y2 - y1) * .5;
                  t->rypos() = y;
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
      if (!score()->showInstrumentNames()) {
            for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                  SysStaff* staff = _staves[staffIdx];
                  foreach(InstrumentName* t, staff->instrumentNames)
                        //score()->undoRemoveElement(t);
                        score()->removeElement(t);
                  }
            return;
            }
      int tick = ml.isEmpty() ? 0 : ml.front()->tick();
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            SysStaff* staff = _staves[staffIdx];
            Staff* s        = score()->staff(staffIdx);
            if (!s->isTop()) {
                  foreach(InstrumentName* t, staff->instrumentNames)
                        //score()->undoRemoveElement(t);
                        score()->removeElement(t);
                  continue;
                  }

            Part* part = s->part();
            const QList<StaffNameDoc>& names = longName? part->longNames(tick) : part->shortNames(tick);

            int idx = 0;
            foreach(const StaffNameDoc& sn, names) {
                  InstrumentName* iname = staff->instrumentNames.value(idx);

                  if (iname == 0) {
                        iname = new InstrumentName(score());
                        iname->setGenerated(true);
                        iname->setParent(this);
                        iname->setTrack(staffIdx * VOICES);
                        iname->setInstrumentNameType(longName ? INSTRUMENT_NAME_LONG : INSTRUMENT_NAME_SHORT);
                        // score()->undoAddElement(iname);
                        score()->addElement(iname);
                        }
                  iname->setText(sn.name);
                  iname->setLayoutPos(sn.pos);
                  ++idx;
                  }
            for (; idx < staff->instrumentNames.size(); ++idx)
                  // score()->undoRemoveElement(staff->instrumentNames[idx]);
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
 considered "inside" the staff is increased a bit.
 TODO: replace magic number "0.6" by something more appropriate.
*/

int System::y2staff(qreal y) const
      {
      y -= pos().y();
      int idx = 0;
      foreach(SysStaff* s, _staves) {
            qreal t = s->bbox().top();
            qreal b = s->bbox().bottom();
            qreal y1 = t - 0.6 * (b - t);
            qreal y2 = b + 0.6 * (b - t);
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
            case INSTRUMENT_NAME:
// qDebug("  staffIdx %d, staves %d", el->staffIdx(), _staves.size());
                  _staves[el->staffIdx()]->instrumentNames.append(static_cast<InstrumentName*>(el));
                  break;

            case BEAM:
                  score()->add(el);
                  break;

            case BRACKET:
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

            case MEASURE:
            case HBOX:
            case VBOX:
            case TBOX:
            case FBOX:
                  score()->add(static_cast<MeasureBase*>(el));
                  break;
            case TEXTLINE_SEGMENT:
            case HAIRPIN_SEGMENT:
            case OTTAVA_SEGMENT:
            case TRILL_SEGMENT:
            case VOLTA_SEGMENT:
            case SLUR_SEGMENT:
            case PEDAL_SEGMENT:
                  {
                  SpannerSegment* ss = static_cast<SpannerSegment*>(el);
#ifndef NDEBUG
                  if (_spannerSegments.contains(ss))
                        qDebug("System::add() spanner already there");
                  else
#endif
                  _spannerSegments.append(ss);
                  }
                  break;
            case BAR_LINE:
                  if (_barLine)
                        qDebug("%p System::add(%s, %p): there is alread a barline %p", this, el->name(), el, _barLine);
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
// qDebug("%p System::remove: %p %s", this, el, el->name());

//no!      el->setParent(0);
      switch (el->type()) {
            case INSTRUMENT_NAME:
                  _staves[el->staffIdx()]->instrumentNames.removeOne(static_cast<InstrumentName*>(el));
                  break;
            case BEAM:
                  score()->remove(el);
                  break;
            case BRACKET:
                  {
                  Bracket* b = static_cast<Bracket*>(el);
                  if (!_brackets.removeOne(b))
                        qDebug("System::remove: bracket not found");
//                  b->staff()->setBracket(b->level(), NO_BRACKET);
                  }
                  break;
            case MEASURE:
            case HBOX:
            case VBOX:
            case TBOX:
            case FBOX:
                  score()->remove(el);
                  break;
            case TEXTLINE_SEGMENT:
            case HAIRPIN_SEGMENT:
            case OTTAVA_SEGMENT:
            case TRILL_SEGMENT:
            case VOLTA_SEGMENT:
            case SLUR_SEGMENT:
            case PEDAL_SEGMENT:
// qDebug("System::remove: %p %s spanner %p %s", el, el->name(),
//            ((SpannerSegment*)el)->spanner(), ((SpannerSegment*)el)->spanner()->name());

                  if (!_spannerSegments.removeOne(static_cast<SpannerSegment*>(el))) {
                        qDebug("System::remove: %p(%s) not found, score %p == %p", el, el->name(), score(), el->score());
                        Q_ASSERT(score() == el->score());
                        }
                  break;
            case BAR_LINE:
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
      if (o->type() == VBOX || o->type() == HBOX || o->type() == TBOX || o->type() == FBOX) {
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
      for (MeasureBase* mb = ml.front(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
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
            if (mb->type() != MEASURE)
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
      while ((s = s->next1(Segment::SegChordRest))) {
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
      if ((l->syllabic() == Lyrics::SINGLE || l->syllabic() == Lyrics::END) && (l->ticks() == 0)) {
            l->clearSeparator();
            return;
            }
      qreal _spatium = spatium();

      const TextStyle& ts = l->textStyle();
      qreal lmag          = qreal(ts.size()) / 11.0;
      qreal staffMag = l->staff()->mag();

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
                        qreal headWidth = symbols[0][quartheadSym].width(1.0);
                        qreal len = seg->pagePos().x() - l->pagePos().x() - x1 + headWidth;
                        line->setLen(Spatium(len / _spatium));
                        Lyrics* nl = searchNextLyrics(seg, staffIdx, l->no());
                        // small correction if next lyrics is moved? not needed if on another system
                        if (nl && nl->measure()->system() == s1) {
                              qreal x2  = nl->bbox().left() + nl->pagePos().x();
                              qreal lx2 = line->pagePos().x() + len;
// printf("line %f  text %f\n", lx2, x2);
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

      foreach(Bracket* b, _brackets)
            func(data, b);

      int idx = 0;
      foreach (SysStaff* st, _staves) {
            if (!all && !(st->show() && score()->staff(idx)->show())) {
                  ++idx;
                  continue;
                  }
            foreach (InstrumentName* t, st->instrumentNames)
                  func(data, t);
            ++idx;
            }
      foreach (SpannerSegment* ss, _spannerSegments) {
            int staffIdx = ss->spanner()->staffIdx();
            if (staffIdx == -1) {
                  qDebug("System::scanElements: staffIDx == -1: %s %p", ss->spanner()->name(), ss->spanner());
                  staffIdx = 0;
                  }
            bool v = true;
            Spanner* spanner = ss->spanner();
            if (spanner->anchor() == Spanner::ANCHOR_SEGMENT || spanner->anchor() == Spanner::ANCHOR_CHORD) {
                  Element* se = spanner->startElement();
                  Element* ee = spanner->endElement();
                  bool v1 = true;
                  if (se && (se->type() == Element::CHORD || se->type() == Element::REST)) {
                        ChordRest* cr = static_cast<ChordRest*>(se);
                        Measure* m    = cr->measure();
                        MStaff* mstaff = m->mstaff(cr->staffIdx());
                        v1 = mstaff->visible();
                        }
                  bool v2 = true;
                  if(!v1 && ee && (ee->type() == Element::CHORD || ee->type() == Element::REST)) {
                        ChordRest* cr = static_cast<ChordRest*>(ee);
                        Measure* m    = cr->measure();
                        MStaff* mstaff = m->mstaff(cr->staffIdx());
                        v2 = mstaff->visible();
                        }
                  v = v1 || v2; // hide spanner if both chords are hidden
                  }
            if (all || (score()->staff(staffIdx)->show() && v) || (spanner->type() == Element::VOLTA))
                  ss->scanElements(data, func, all);
            }
      }

//---------------------------------------------------------
//   staffYpage
//    return page coordinates
//---------------------------------------------------------

qreal System::staffYpage(int staffIdx) const
      {
      if (_staves.size() <= staffIdx) {
            qDebug("staffY: staves %d <= staffIdx %d, vbox %d",
               _staves.size(), staffIdx, _vbox);
            // abort();
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

}

