//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: system.cpp 5656 2012-05-21 15:36:47Z wschweer $
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
      foreach(Bracket* b, brackets)
            delete b;
      brackets.clear();
      qDeleteAll(instrumentNames);
      }

//---------------------------------------------------------
//   System
//---------------------------------------------------------

System::System(Score* s)
   : Element(s)
      {
      barLine      = 0;
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
      delete barLine;
      qDeleteAll(_staves);
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

      qreal _spatium = score()->spatium();

      int nstaves  = _staves.size();
      if (nstaves != score()->nstaves())
            qDebug("System::layout: nstaves %d != %d\n", nstaves, score()->nstaves());

      //---------------------------------------------------
      //  find x position of staves
      //    create brackets
      //---------------------------------------------------

      qreal xoff2 = 0.0;         // x offset for instrument name

      int bracketLevels = 0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            int n = score()->staff(staffIdx)->bracketLevels();
            if (n > bracketLevels)
                  bracketLevels = n;
            }
      qreal bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* s     = score()->staff(staffIdx);
            SysStaff* ss = _staves[staffIdx];
            if (!s->show() || !ss->show())
                  continue;

            if (bracketLevels < ss->brackets.size()) {
                  for (int i = bracketLevels; i < ss->brackets.size(); ++i) {
                        Bracket* b = ss->brackets.takeLast();
                        delete b;
                        }
                  }
            else if (bracketLevels > ss->brackets.size()) {
                  for (int i = ss->brackets.size(); i < bracketLevels; ++i)
                        ss->brackets.append(0);
                  }
            for (int i = 0; i < bracketLevels; ++i) {
                  Bracket* b = ss->brackets[i];
                  if (s->bracket(i) == NO_BRACKET) {
                        delete b;
                        ss->brackets[i] = 0;
                        }
                  else {
                        if (b == 0) {
                              ss->brackets[i] = b = new Bracket(score());
                              b->setParent(this);
                              b->setTrack(staffIdx * VOICES);
                              }
                        b->setSubtype(s->bracket(i));
                        int span = s->bracketSpan(i);
                        b->setSpan(span);
                        b->setLevel(i);
                        qreal w = b->width();
                        if (w > bracketWidth[i])
                              bracketWidth[i] = w;
                        }
                  }
            foreach(InstrumentName* t, ss->instrumentNames) {
                  t->layout();
                  qreal w = t->width() + point(instrumentNameOffset);
                  if (w > xoff2)
                        xoff2 = w;
                  }
            }

      //---------------------------------------------------
      //  layout  SysStaff and StaffLines
      //---------------------------------------------------

      // xoff2 += xo1;
      _leftMargin = xoff2;

      for (int i = 0; i < bracketLevels; ++i)
            _leftMargin += bracketWidth[i] + point(score()->styleS(ST_bracketDistance));

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* s  = _staves[staffIdx];
            Staff* staff = score()->staff(staffIdx);
            if (!staff->show() || !s->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            qreal staffMag = staff->mag();
            s->setbbox(QRectF(_leftMargin + xo1, 0.0, 0.0, 4 * spatium() * staffMag));
            }

      if ((nstaves > 1 && score()->styleB(ST_startBarlineMultiple)) || (nstaves <= 1 && score()->styleB(ST_startBarlineSingle))) {
            if (barLine == 0) {
                  barLine = new Line(score(), true);
                  barLine->setLineWidth(score()->styleS(ST_barWidth));
                  barLine->setParent(this);
                  }
            }
      else if (barLine) {
            delete barLine;
            barLine = 0;
            }
      if (barLine)
            barLine->setPos(_leftMargin + xo1 + barLine->lineWidth().val() * _spatium * .5, 0.0);

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* s = score()->staff(staffIdx);
            SysStaff* ss = _staves[staffIdx];
            if (!(s->show() && ss->show()))
                  continue;

            qreal xo = -xo1;
            for (int i = 0; i < bracketLevels; ++i) {
                  xo += bracketWidth[i] + point(score()->styleS(ST_bracketDistance));
                  Bracket* b = ss->brackets[i];
                  if (b == 0)
                        continue;

                  if (b->span() >= (nstaves - staffIdx)) {
                        //
                        // this may happen if a system was removed in
                        // instruments dialog
                        //
                        b->setSpan(nstaves - staffIdx);
                        }
                  int idx = staffIdx + b->span() - 1;
                  if (!_staves[idx]->show() || !score()->staff(idx)->show()) {
                        //
                        // if the bracket ends on an invisible staff
                        // find last visible staff in bracket
                        //
                        for (int j = staffIdx + b->span() - 2; j >= staffIdx; --j) {
                              if (_staves[j]->show() && score()->staff(j)->show()) {
                                    b->setSpan(j - staffIdx + 1);
                                    break;
                                    }
                              }
                        }
                  // right align bracket
                  b->rxpos() = _leftMargin - xo + bracketWidth[i] - b->width();
                  }
            }

      //---------------------------------------------------
      //  layout instrument names x position
      //---------------------------------------------------

      int idx = 0;
      foreach (Part* p, score()->parts()) {
            SysStaff* s = staff(idx);
            int nstaves = p->nstaves();
            if (s->show() && p->show()) {
                  foreach(InstrumentName* t, s->instrumentNames) {
                        qreal d  = point(instrumentNameOffset) + t->bbox().width();
                        t->rxpos() = xoff2 - d + xo1;
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

      int nstaves     = _staves.size();
      qreal _spatium = spatium();

      qreal y = 0.0;
      int lastStaffIdx  = 0;   // last visible staff
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
            foreach(MeasureBase* m, ml) {
                  distDown = qMax(distDown, m->distanceDown(staffIdx));
                  distDown = qMax(distDown, m->userDistanceDown(staffIdx));
                  distUp   = qMax(distUp,   m->distanceUp(staffIdx));
                  distUp   = qMax(distUp,   m->userDistanceUp(staffIdx));
                  }
            s->setDistanceDown(distDown);
            s->setDistanceUp(distUp);

            if (!staff->show() || !s->show()) {
                  s->setbbox(QRectF());  // already done in layout() ?
                  continue;
                  }
            qreal sHeight = staff->height();   // (staff->lines() - 1) * _spatium * staffMag;
            s->setbbox(QRectF(_leftMargin, y, width() - _leftMargin, sHeight));
            y += sHeight + s->distanceDown();
            lastStaffIdx = staffIdx;
            }

      qreal systemHeight = staff(lastStaffIdx)->bbox().bottom();
      setHeight(systemHeight);
      foreach(MeasureBase* m, ml) {
            if (m->type() == MEASURE) {
                  m->setbbox(QRectF(0.0, -_spatium, m->width(), systemHeight + 2 * _spatium));
                  }
            else if (m->type() == HBOX) {
                  m->setbbox(QRectF(0.0, 0.0, m->width(), systemHeight));
                  static_cast<HBox*>(m)->layout2();
                  }
            }

      if (barLine) {
            barLine->setLen(Spatium(systemHeight / _spatium));
            barLine->layout();
            }

      //---------------------------------------------------
      //  layout brackets vertical position
      //---------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* ss = _staves[staffIdx];
            if (!ss->show() || !score()->staff(staffIdx)->show())
                  continue;

            foreach(Bracket* b, ss->brackets) {
                  if (b == 0)
                        continue;
                  int restStaves = nstaves - staffIdx;
                  if (b->span() >= restStaves) {
                        //
                        // this may happen if a system was removed in
                        // instruments dialog
                        //
                        b->setSpan(restStaves);
                        }
                  qreal sy = ss->bbox().top();
                  qreal ey = _staves[staffIdx + b->span() - 1]->bbox().bottom();
                  b->rypos() = sy;
                  b->setHeight(ey - sy);
                  b->layout();
                  }
            }

      //---------------------------------------------------
      //  layout instrument names
      //---------------------------------------------------

      int staffIdx = 0;
      foreach(Part* p, score()->parts()) {
            SysStaff* s = staff(staffIdx);
            int nstaves = p->nstaves();
            foreach (InstrumentName* t, s->instrumentNames) {
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
                  qreal y  = y1 + (y2 - y1) * .5 - t->bbox().height() * .5;
                  t->rypos() = y;
                  }
            staffIdx += nstaves;
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SysStaff::move(qreal x, qreal y)
      {
      _bbox.translate(x, y);
      foreach(Bracket* b, brackets)
            b->move(x, y);
      foreach(InstrumentName* t, instrumentNames)
            t->move(x, y);
      }

//---------------------------------------------------------
///   clear
///   Clear layout of System
//---------------------------------------------------------

void System::clear()
      {
      ml.clear();
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
      if (isVbox())                 // ignore vbox
            return;
      int tick = ml.isEmpty() ? 0 : ml.front()->tick();
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            SysStaff* staff = _staves[staffIdx];
            Staff* s        = score()->staff(staffIdx);
            if (!s->isTop()) {
                  foreach(InstrumentName* t, staff->instrumentNames)
                        score()->undoRemoveElement(t);
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
                        iname->setSubtype(longName ? INSTRUMENT_NAME_LONG : INSTRUMENT_NAME_SHORT);
//                        if (score()->undoRedo())
//                              qFatal("System::setInstrumentNames <%s> in undo/redo",
//                                 qPrintable(sn.name.toPlainText()));
                        score()->undoAddElement(iname);
                        }
                  iname->setText(sn.name);
                  iname->setLayoutPos(sn.pos);
                  ++idx;
                  }
            for (; idx < staff->instrumentNames.size(); ++idx)
                  score()->undoRemoveElement(staff->instrumentNames[idx]);
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
//      qDebug("System::add: %s\n", el->name());

      el->setParent(this);
      switch(el->type()) {
            case INSTRUMENT_NAME:
                  _staves[el->staffIdx()]->instrumentNames.append(static_cast<InstrumentName*>(el));
                  break;
            case BEAM:
                  score()->add(el);
                  break;
            case BRACKET:
                  {
                  SysStaff* ss = _staves[el->staffIdx()];
                  Bracket* b   = static_cast<Bracket*>(el);
                  int level    = b->level();
                  if (level == -1) {
                        level = ss->brackets.size() - 1;
                        if (level >= 0 && ss->brackets.last() == 0) {
                              ss->brackets[level] = b;
                              }
                        else {
                              ss->brackets.append(b);
                              level = ss->brackets.size() - 1;
                              }
                        }
                  else {
                        while (level >= ss->brackets.size())
                              ss->brackets.append(0);
                        ss->brackets[level] = b;
                        }
                  b->staff()->setBracket(level,   b->subtype());
                  b->staff()->setBracketSpan(level, b->span());
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
                  {
// qDebug("System::add: %p %s spanner %p %s\n", el, el->name(),
//            ((SpannerSegment*)el)->spanner(), ((SpannerSegment*)el)->spanner()->name());
                  SpannerSegment* ss = static_cast<SpannerSegment*>(el);
                  if (!_spannerSegments.contains(ss))
                        _spannerSegments.append(ss);
                  else {
                        // qDebug("System::add() spanner already there\n");
                        }
                  }
                  break;
            default:
                  qDebug("System::add(%s) not implemented\n", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void System::remove(Element* el)
      {
//      qDebug("System::remove: %s\n", el->name());

      switch (el->type()) {
            case INSTRUMENT_NAME:
                  _staves[el->staffIdx()]->instrumentNames.removeOne(static_cast<InstrumentName*>(el));
                  break;
            case BEAM:
                  score()->remove(el);
                  break;
            case BRACKET:
                  {
                  SysStaff* staff = _staves[el->staffIdx()];
                  for (int i = 0; i < staff->brackets.size(); ++i) {
                        if (staff->brackets[i] == el) {
                              staff->brackets[i] = 0;
                              el->staff()->setBracket(i, NO_BRACKET);
                              return;
                              }
                        }
                  qDebug("internal error: bracket not found\n");
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
// qDebug("System::remove: %p %s spanner %p %s\n", el, el->name(),
//            ((SpannerSegment*)el)->spanner(), ((SpannerSegment*)el)->spanner()->name());

                  if (!_spannerSegments.removeOne(static_cast<SpannerSegment*>(el))) {
//                        qDebug("System::remove: %p(%s) not found, score %p == %p\n", el, el->name(), score(), el->score());
                        Q_ASSERT(score() == el->score());
                        }
                  break;
            default:
                  qDebug("System::remove(%s) not implemented\n", el->name());
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
      while ((s = s->next1(Segment::SegChordRest | Segment::SegGrace))) {
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

      if (l->ticks()) {
            // melisma
            Segment* seg = score()->tick2segment(l->endTick());
            if (seg == 0) {
                  qDebug("System::layoutLyrics: no segment found for tick %d\n", l->endTick());
                  return;
                  }

            QList<Line*>* sl = l->separatorList();
            QList<System*>* systems = score()->systems();
            System* s1  = this;
            System* s2  = seg->measure()->system();
            int sysIdx1 = systems->indexOf(s1);
            int sysIdx2 = systems->indexOf(s2);

            qreal  x1 = l->bbox().right();            // lyrics width
            QPointF p1(x1, l->baseLine());

            int segIdx = 0;
            for (int i = sysIdx1; i <= sysIdx2; ++i, ++segIdx) {
                  System* system = (*systems)[i];
                  Line* line = sl->value(segIdx);
                  if (line == 0) {
                        line = new Line(l->score(), false);
                        l->add(line);
                        }
                  line->setLineWidth(Spatium(0.1 * lmag));
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
      const qreal maxl = 0.5 * _spatium * lmag;   // lyrics hyphen length
      const Spatium hlw(0.14 * lmag);              // hyphen line width

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
      QRectF b = l->bbox();
      qreal x  = b.right();
      qreal y  = b.y() + b.height() * .58;

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
      if (barLine)
            func(data, barLine);

      int idx = 0;
      foreach (SysStaff* st, _staves) {
            if (!all && !(st->show() && score()->staff(idx)->show())) {
                  ++idx;
                  continue;
                  }
            foreach(Bracket* b, st->brackets) {
                  if (b)
                        func(data, b);
                  }
            foreach (InstrumentName* t, st->instrumentNames)
                  func(data, t);
            ++idx;
            }
      foreach(SpannerSegment* ss, _spannerSegments) {
            int staffIdx;
            if (ss->spanner()->type() == SLUR)
                  staffIdx = ss->spanner()->startElement()->staffIdx();
            else
                  staffIdx = ss->spanner()->staffIdx();
            if (all || score()->staff(staffIdx)->show())
                  func(data, ss);
            }
      }

//---------------------------------------------------------
//   staffY
//---------------------------------------------------------

qreal System::staffY(int staffIdx) const
      {
      if (_staves.size() <= staffIdx) {
            qDebug("staffY: staves %d <= staff %d, vbox %d\n",
               _staves.size(), staffIdx, _vbox);
            return pagePos().y();
            }
      return _staves[staffIdx]->y() + pagePos().y();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void System::write(Xml& xml) const
      {
      xml.stag("System");
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void System::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());

            if (tag == "System") {
                  }
            else
                  domError(e);
            }
      }

