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
#include "sym.h"
#include "spacer.h"

namespace Ms {

//---------------------------------------------------------
//   y
//---------------------------------------------------------

qreal SysStaff::y() const
      {
      return _bbox.y() + _yOff;
      }

//---------------------------------------------------------
//   setYOff
//---------------------------------------------------------

void SysStaff::setYOff(qreal offset)
      {
      _yOff = offset;
      }

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

SysStaff::SysStaff()
      {
      _yOff = 0.0;
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
      _leftMargin  = 0.0;
      _pageBreak   = false;
      _vbox        = false;
      }

//---------------------------------------------------------
//   ~System
//---------------------------------------------------------

System::~System()
      {
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

void System::layoutSystem(qreal xo1)
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
                              score()->undoAddElement(b);
                              }
                        else
                              _brackets.append(b);
                        b->setFirstStaff(firstStaff);
                        b->setLastStaff(lastStaff);
                        b->setBracketType(s->bracket(i));
                        b->setSpan(s->bracketSpan(i));
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

      int nVisible = 0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* s  = _staves[staffIdx];
            Staff* staff = score()->staff(staffIdx);
            if (!staff->show() || !s->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            ++nVisible;
            qreal staffMag = staff->mag();
            qreal h;
            if (staff->lines() == 1)
                  h = 2;
            else
                  h = (staff->lines()-1) * staff->lineDistance();
            h = h * staffMag * spatium();
            s->bbox().setRect(_leftMargin + xo1, 0.0, 0.0, h);
            }

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
      if (isVbox()) {
            vbox()->layout();
            setbbox(vbox()->bbox());
            return;
            }

      setPos(0.0, 0.0);
      QList<std::pair<int,SysStaff*>> visibleStaves;

      for (int i = 0; i < _staves.size(); ++i) {
            Staff*    s  = score()->staff(i);
            SysStaff* ss = _staves[i];
            if (s->show() && ss->show())
                  visibleStaves.append(std::pair<int,SysStaff*>(i, ss));
            else
                  ss->setbbox(QRectF());  // already done in layout() ?
            }

      qreal _spatium            = spatium();
      qreal y                   = 0.0;
      qreal minVerticalDistance = score()->styleS(StyleIdx::minVerticalDistance).val() * _spatium;
      qreal staffDistance       = score()->styleS(StyleIdx::staffDistance).val() * _spatium;
      qreal akkoladeDistance    = score()->styleS(StyleIdx::akkoladeDistance).val() * _spatium;

      Q_ASSERT(!visibleStaves.empty());

      for (auto i = visibleStaves.begin();; ++i) {
            SysStaff* ss  = i->second;
            int si1       = i->first;
            Staff* staff  = score()->staff(si1);
            auto ni       = i + 1;

            qreal h = score()->staff(si1)->height();
            if (ni == visibleStaves.end()) {
                  ss->bbox().setRect(0.0, y, width(), h);
                  break;
                  }

            int si2    = ni->first;
            qreal dist = h + staff->isTop() ? akkoladeDistance : staffDistance;
            dist += _spatium * 4;

            for (MeasureBase* mb : ml) {
                  if (mb->type() != Element::Type::MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  for (Segment* s = m->first(); s; s = s->next()) {
                        Shape s1(s->shape(si1));
                        s1.translate(s->pos());
                        Shape s2(s->shape(si2));
                        s2.translate(s->pos());

                        s1.add(QRectF(0.0, _spatium * 4, 1000.0, 0.0));   // bottom staff line
                        s2.add(QRectF(0.0, 0.0, 1000.0, 0.0));            // top staff line

                        // QPointF pt(s->pos().x() + m->pos().x() + system->pos().x(), system->pos().y());
                        for (Element* e : s->annotations()) {
                              if (e->staffIdx() == si1)
                                    s1.add(e->bbox().translated(e->pos() + s->pos()));
                              else if (e->staffIdx() == si2)
                                    s2.add(e->bbox().translated(e->pos() + s->pos()));
                              }
                        qreal d = s1.minVerticalDistance(s2) + minVerticalDistance;
                        dist    = qMax(dist, d);
                        }
                  Spacer* sp = m->mstaff(si1)->_vspacerDown;
                  if (sp)
                        dist = qMax(dist, _spatium * 4 + sp->gap());
                  sp = m->mstaff(si2)->_vspacerUp;
                  if (sp)
                        dist = qMax(dist, sp->gap());
                  }
            ss->bbox().setRect(0.0, y, width(), h);
            y += dist;
            }

      int lastStaffIdx = visibleStaves.back().first;
      qreal systemHeight = staff(lastStaffIdx)->bbox().bottom();
      setHeight(systemHeight);

      for (MeasureBase* m : ml) {
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

      int firstStaffIdx        = visibleStaves.front().first;
      int firstStaffInitialIdx = firstStaffIdx;  //??
      int lastStaffInitialIdx = lastStaffIdx;   //??

      BarLine* _barLine = 0;
      Segment* s = firstMeasure()->first();
      if (s->segmentType() == Segment::Type::BeginBarLine)
            _barLine = static_cast<BarLine*>(s->element(0));

      if (_barLine) {
            _barLine->setTrack(firstStaffInitialIdx * VOICES);
            _barLine->setSpan(lastStaffInitialIdx - firstStaffInitialIdx + 1);
            if (score()->staff(firstStaffInitialIdx)->lines() == 1)
                  _barLine->setSpanFrom(BARLINE_SPAN_1LINESTAFF_FROM);
            else
                  _barLine->setSpanFrom(0);
            int spanTo = (score()->staff(lastStaffInitialIdx)->lines() == 1) ?
                              BARLINE_SPAN_1LINESTAFF_TO :
                              (score()->staff(lastStaffInitialIdx)->lines() - 1) * 2;
            _barLine->setSpanTo(spanTo);
            _barLine->layout();
            }

      //---------------------------------------------------
      //  layout brackets vertical position
      //---------------------------------------------------

      for (Bracket* b : _brackets) {
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

      // TODO: ml is normally empty here, so we are unable to retrieve tick
      // thus, staff name does not reflect current instrument
      int tick = ml.empty() ? 0 : ml.front()->tick();
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
                  iname->setXmlText(sn.name());
                  iname->setLayoutPos(sn.pos());
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
      if (!el)
            return;
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
            case Element::Type::LYRICSLINE_SEGMENT:
            case Element::Type::GLISSANDO_SEGMENT:
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
            case Element::Type::LYRICSLINE_SEGMENT:
            case Element::Type::GLISSANDO_SEGMENT:
                  if (!_spannerSegments.removeOne(static_cast<SpannerSegment*>(el))) {
                        qDebug("System::remove: %p(%s) not found, score %p", el, el->name(), score());
                        Q_ASSERT(score() == el->score());
                        }
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
#if 0 // TODO??
      if (o->type() == Element::Type::VBOX || o->type() == Element::Type::HBOX || o->type() == Element::Type::TBOX || o->type() == Element::Type::FBOX) {
            auto idx = ml.indexOf((MeasureBase*)o);
            if (idx != -1)
                  ml.removeAt(idx);
            ml.insert(idx, (MeasureBase*)n);
            score()->measures()->change((MeasureBase*)o, (MeasureBase*)n);
            }
      else {
#endif
            remove(o);
            add(n);
//            }
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
      auto i = std::find_if(ml.begin(), ml.end(), [](MeasureBase* mb){return mb->isMeasure();});
      return i != ml.end() ? static_cast<Measure*>(*i) : 0;
      }

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* System::lastMeasure() const
      {
      auto i = std::find_if(ml.rbegin(), ml.rend(), [](MeasureBase* mb){return mb->isMeasure();});
      return i != ml.rend() ? static_cast<Measure*>(*i) : 0;
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
//   scanElements
//    collect all visible elements
//---------------------------------------------------------

void System::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (isVbox())
            return;
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
            if (all || (score()->staff(staffIdx)->show() && _staves[staffIdx]->show() && v) || (spanner->type() == Element::Type::VOLTA))
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
      return _staves[staffIdx]->y() + y();
      }

//---------------------------------------------------------
//   staffCanvasYpage
//    return canvas coordinates
//---------------------------------------------------------

qreal System::staffCanvasYpage(int staffIdx) const
      {
      return _staves[staffIdx]->y() + y() + page()->canvasPos().y();
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

void System::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
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

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(Score* s) : Symbol(s)
      {
      setSystemFlag(true);
      setTrack(0);
      // default value, but not valid until setDividerType()
      _dividerType = SystemDivider::Type::LEFT;
      _sym = SymId::systemDivider;
      }

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(const SystemDivider& sd) : Symbol(sd)
      {
      _dividerType = sd._dividerType;
      }

//---------------------------------------------------------
//   setDividerType
//---------------------------------------------------------

void SystemDivider::setDividerType(SystemDivider::Type v)
      {
      ScoreFont* sf = _score->scoreFont();
       _dividerType = v;
       if (v == SystemDivider::Type::LEFT) {
             SymId symLeft = Sym::name2id(_score->styleSt(StyleIdx::dividerLeftSym));
             if (!symIsValid(symLeft))
                   sf = sf->fallbackFont();
             setSym(symLeft, sf);
             setXoff(_score->styleD(StyleIdx::dividerLeftX));
             setYoff(_score->styleD(StyleIdx::dividerLeftY));
             setAlign(AlignmentFlags::LEFT | AlignmentFlags::VCENTER);
             }
       else {
             SymId symRight = Sym::name2id(_score->styleSt(StyleIdx::dividerRightSym));
             if (!symIsValid(symRight))
                   sf = sf->fallbackFont();
             setSym(symRight, sf);
             setXoff(_score->styleD(StyleIdx::dividerRightX));
             setYoff(_score->styleD(StyleIdx::dividerRightY));
             setAlign(AlignmentFlags::RIGHT | AlignmentFlags::VCENTER);
             }
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF SystemDivider::drag(EditData* ed)
      {
      setGenerated(false);
      return Symbol::drag(ed);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SystemDivider::layout()
      {
      Symbol::layout();
      // center vertically between systems
      rypos() -= bbox().y();
      Measure* m = measure();
      if (m) {
            System* s1 = m->system();
            if (!s1)
                  return;
            // rypos() += s1->height() + (s1->distance() + s1->stretchDistance()) * 0.5;
            rypos() += s1->height();
            // center horizontally under left/right barline
            if (dividerType() == SystemDivider::Type::LEFT)
                  rxpos() -= width() * 0.5;
            else
                  rxpos() += m->width() + width() * 0.5;
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SystemDivider::write(Xml& xml) const
      {
      if (dividerType() == SystemDivider::Type::LEFT)
            xml.stag(QString("SystemDivider type=\"left\""));
      else
            xml.stag(QString("SystemDivider type=\"right\""));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SystemDivider::read(XmlReader& e)
      {
      Align a = align() & AlignmentFlags::VMASK;
      ScoreFont* sf = score()->scoreFont();
      if (e.attribute("type") == "left") {
            _dividerType = SystemDivider::Type::LEFT;
            SymId sym = Sym::name2id(score()->styleSt(StyleIdx::dividerLeftSym));
            if (!symIsValid(sym))
                  sf = sf->fallbackFont();
            setSym(sym, sf);
            setAlign(a | AlignmentFlags::LEFT);
            setXoff(score()->styleB(StyleIdx::dividerLeftX));
            setYoff(score()->styleB(StyleIdx::dividerLeftY));
            }
      else {
            _dividerType = SystemDivider::Type::RIGHT;
            SymId sym = Sym::name2id(score()->styleSt(StyleIdx::dividerRightSym));
            if (!symIsValid(sym))
                  sf = sf->fallbackFont();
            setSym(sym, sf);
            setAlign(a | AlignmentFlags::RIGHT);
            setXoff(score()->styleB(StyleIdx::dividerRightX));
            setYoff(score()->styleB(StyleIdx::dividerRightY));
            }
      Symbol::read(e);
      }

//---------------------------------------------------------
//   minDistance
//    Return the minimum distance between this system and s2
//    without any element collisions.
//
//    this - top system
//    s2   - bottom system
//---------------------------------------------------------

qreal System::minDistance(System* s2) const
      {
      const qreal _spatium            = spatium();
      const qreal systemFrameDistance = _score->styleS(StyleIdx::systemFrameDistance).val() * _spatium;
      const qreal frameSystemDistance = _score->styleS(StyleIdx::frameSystemDistance).val() * _spatium;

      if (isVbox() && !s2->isVbox())
            return qMax(frameSystemDistance, -s2->minTop());
      else if (!isVbox() && s2->isVbox())
            return qMax(systemFrameDistance, -minBottom());
      else if (isVbox() && s2->isVbox())
            return 0.0;

      qreal minVerticalDistance = score()->styleS(StyleIdx::minVerticalDistance).val() * _spatium;
      qreal dist                = score()->styleS(StyleIdx::minSystemDistance).val()   * _spatium;
      int lastStaff             = _staves.size() - 1;
      qreal lastStaffY          = _staves[lastStaff]->y();

      for (MeasureBase* mb1 : ml) {
            qreal bx1 = mb1->x();
            qreal bx2 = mb1->x() + mb1->width();
            for (MeasureBase* mb2 : s2->measures()) {
                  if (mb1->type() != Element::Type::MEASURE || mb2->type() != Element::Type::MEASURE)
                        continue;
                  Measure* m1 = static_cast<Measure*>(mb1);
                  Measure* m2 = static_cast<Measure*>(mb2);
                  qreal ax1 = mb2->x();
                  if (ax1 > bx2)
                        break;
                  qreal ax2 = mb2->x() + mb2->width();
                  if (ax2 < bx1)
                        continue;
                  if ((ax1 >= bx1 && ax1 < bx2) || (ax2 >= bx1 && ax2 < bx2) || (ax1 < bx1 && ax2 >= bx2)) {
                        Shape s1;
                        Shape s2;
                        for (Segment* s = m1->first(); s; s = s->next())
                              s1.add(s->shape(lastStaff).translated(s->pos()));
                        for (Segment* s = m2->first(); s; s = s->next())
                              s2.add(s->shape(0).translated(s->pos()));
                        s1.translate(QPointF(m1->x(), lastStaffY));
                        s2.translate(QPointF(m2->x(), 0.0));

                        s1.add(QRectF(0.0, height(), 100000.0, 0.0));   // bottom staff line
                        s2.add(QRectF(0.0, 0.0,      100000.0, 0.0));   // top staff line

                        qreal d = s1.minVerticalDistance(s2) + minVerticalDistance;
                        dist = qMax(dist, d);
                        }
                  }
            }
      return qMax(score()->styleS(StyleIdx::minSystemDistance).val() * _spatium, dist - height());
      }

//---------------------------------------------------------
//   minTop
//    Return the minimum top margin.
//---------------------------------------------------------

qreal System::minTop() const
      {
      qreal dist = 0.0;
      for (MeasureBase* mb : ml) {
            if (mb->type() != Element::Type::MEASURE)
                  continue;
            for (Segment* s = static_cast<Measure*>(mb)->first(); s; s = s->next())
                  dist = qMin(dist, s->shape(0).top());
            }
      return dist;
      }

//---------------------------------------------------------
//   minBottom
//    Return the minimum bottom margin.
//---------------------------------------------------------

qreal System::minBottom() const
      {
      qreal dist = 0.0;
      int staffIdx = score()->nstaves() - 1;
      for (MeasureBase* mb : ml) {
            if (mb->type() != Element::Type::MEASURE)
                  continue;
            for (Segment* s = static_cast<Measure*>(mb)->first(); s; s = s->next())
                  dist = qMax(dist, s->shape(staffIdx).bottom());
            }
      return dist - spatium() * 4;
      }

//-------------------------------------------------------------------
//   removeGeneratedElements (System Header + TimeSig Announce)
//    helper function
//-------------------------------------------------------------------

void System::removeGeneratedElements()
      {
      auto fm = std::find_if(ml.begin(), ml.end(), [](MeasureBase* mb){return mb->isMeasure();});
      auto lm = std::find_if(ml.rbegin(), ml.rend(), [](MeasureBase* mb){return mb->isMeasure();});

      for (auto im = fm; im != ml.end(); ++im) {
            if (!(*im)->isMeasure())
                  continue;
            Measure* m = (*im)->measure();
            if (m != *fm && m->hasSystemHeader())
                  m->removeSystemHeader();
            if (m != *lm && m->hasSystemTrailer())
                  m->removeSystemTrailer();
            }
      }

}

