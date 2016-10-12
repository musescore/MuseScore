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
#include "system.h"
#include "box.h"
#include "chordrest.h"
#include "iname.h"
#include "spanner.h"
#include "sym.h"
#include "spacer.h"
#include "systemdivider.h"
#include "textframe.h"

namespace Ms {

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
      }

//---------------------------------------------------------
//   ~System
//---------------------------------------------------------

System::~System()
      {
      qDeleteAll(_staves);
      qDeleteAll(_brackets);
      delete _systemDividerLeft;
      delete _systemDividerRight;
      }

//---------------------------------------------------------
///   clear
///   Clear layout of System
//---------------------------------------------------------

void System::clear()
      {
      ml.clear();
      for (SpannerSegment* ss : _spannerSegments) {
            if (ss->system() == this)
                  ss->setParent(0);       // assume parent() is System
            }
      _spannerSegments.clear();
      // _systemDividers are reused
      }

//---------------------------------------------------------
//   vbox
//---------------------------------------------------------

VBox* System::vbox() const
      {
      if (!ml.empty()) {
            if (ml[0]->isVBox() || ml[0]->isTBox())
                  return static_cast<VBox*>(ml[0]);
            }
      return 0;
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

SysStaff* System::insertStaff(int idx)
      {
      SysStaff* staff = new SysStaff;
      if (idx) {
            // HACK: guess position
            staff->bbox().setY(_staves[idx-1]->y() + 6 * spatium());
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
      if (_staves.empty())                 // ignore vbox
            return;

      static const Spatium instrumentNameOffset(1.0);       // TODO: make style value

      int nstaves  = _staves.size();

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

      QList<Bracket*> bl;
      bl.swap(_brackets);

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
                              b->setTrack(track);
                              b->setLevel(i);
                              }
                        add(b);
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
            delete b;

      //---------------------------------------------------
      //  layout  SysStaff and StaffLines
      //---------------------------------------------------

      // xoff2 += xo1;
      _leftMargin = xoff2;


      qreal bd = score()->styleP(StyleIdx::bracketDistance);
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
      VBox* b = vbox();
      if (b) {
            b->layout();
            setbbox(b->bbox());
            return;
            }

      setPos(0.0, 0.0);
      QList<std::pair<int,SysStaff*>> visibleStaves;

      int firstStaffIdx        = -1;
      int lastStaffIdx         = 0;
      int firstStaffInitialIdx = -1;
      int lastStaffInitialIdx  = 0;
      Measure* fm              = firstMeasure();

      for (int i = 0; i < _staves.size(); ++i) {
            Staff*    s  = score()->staff(i);
            SysStaff* ss = _staves[i];
            if (s->show() && ss->show()) {
                  visibleStaves.append(std::pair<int,SysStaff*>(i, ss));
                  if (firstStaffIdx == -1)
                        firstStaffIdx = i;
                  if (i > lastStaffIdx)
                        lastStaffIdx = i;
                  if (fm && fm->visible(i)) {
                        if (firstStaffInitialIdx == -1)
                              firstStaffInitialIdx = i;
                        lastStaffInitialIdx = i;
                        }
                  }
            else {
                  ss->setbbox(QRectF());  // already done in layout() ?
                  }
            }
      if (firstStaffIdx == -1)
            firstStaffIdx = 0;
      if (firstStaffInitialIdx == -1)
            firstStaffInitialIdx = 0;

      qreal _spatium            = spatium();
      qreal y                   = 0.0;
      qreal minVerticalDistance = score()->styleP(StyleIdx::minVerticalDistance);
      qreal staffDistance       = score()->styleP(StyleIdx::staffDistance);
      qreal akkoladeDistance    = score()->styleP(StyleIdx::akkoladeDistance);

      if (visibleStaves.empty()) {
            qDebug("====no visible staves, staves %d, score staves %d", _staves.size(), score()->nstaves());
            }

      for (auto i = visibleStaves.begin();; ++i) {
            SysStaff* ss  = i->second;
            int si1       = i->first;
            Staff* staff  = score()->staff(si1);
            auto ni       = i + 1;

            qreal h = staff->height();
            if (ni == visibleStaves.end()) {
                  ss->setYOff(staff->lines() == 1 ? _spatium * staff->mag() : 0.0);
                  ss->bbox().setRect(_leftMargin, y, width() - _leftMargin, h);
                  break;
                  }

            int si2    = ni->first;
            qreal dist = h;

            switch (staff->innerBracket()) {
                  case BracketType::BRACE:
                        dist += akkoladeDistance;
                        break;
                  case BracketType::NORMAL:
                  case BracketType::SQUARE:
                  case BracketType::LINE:
                  case BracketType::NO_BRACKET:
                        dist += staffDistance;
                        break;
                  }
            dist += score()->staff(si2)->userDist();

            for (MeasureBase* mb : ml) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  Shape& s1 = m->staffShape(si1);
                  Shape& s2 = m->staffShape(si2);

                  qreal d = s1.minVerticalDistance(s2) + minVerticalDistance;
                  dist    = qMax(dist, d);

                  Spacer* sp = m->mstaff(si1)->_vspacerDown;
                  if (sp) {
                        if (sp->spacerType() == SpacerType::FIXED) {
                              dist = staff->height() + sp->gap();
                              break;
                              }
                        else
                              dist = qMax(dist, staff->height() + sp->gap());
                        }
                  sp = m->mstaff(si2)->_vspacerUp;
                  if (sp)
                        dist = qMax(dist, sp->gap());
                  }

            ss->setYOff(staff->lines() == 1 ? _spatium * staff->mag() : 0.0);
            ss->bbox().setRect(_leftMargin, y, width() - _leftMargin, h);
            y += dist;
            }

      qreal systemHeight = staff(lastStaffIdx)->bbox().bottom();
      setHeight(systemHeight);

      for (MeasureBase* m : ml) {
            if (m->isMeasure()) {
                  // note that the factor 2 * _spatium must be corrected for when exporting
                  // system distance in MusicXML (issue #24733)
                  m->bbox().setRect(0.0, -_spatium, m->width(), systemHeight + 2.0 * _spatium);
                  }
            else if (m->isHBox()) {
                  m->bbox().setRect(0.0, 0.0, m->width(), systemHeight);
                  toHBox(m)->layout2();
                  }
            else if (m->isTBox()) {
//                  m->bbox().setRect(0.0, 0.0, m->width(), systemHeight);
                  toTBox(m)->layout();
                  }
            else
                  qDebug("unhandled measure type %s", m->name());
            }

      if (fm) {
            Segment* s        = fm->first();
            BarLine* _barLine = s->isBeginBarLineType() ? toBarLine(s->element(0)) : 0;

            if (_barLine) {
                  _barLine->setTrack(firstStaffInitialIdx * VOICES);
                  _barLine->setSpan(lastStaffInitialIdx - firstStaffInitialIdx + 1);
                  if (score()->staff(firstStaffInitialIdx)->lines() == 1)
                        _barLine->setSpanFrom(BARLINE_SPAN_1LINESTAFF_FROM);
                  else
                        _barLine->setSpanFrom(0);
                  int llines = score()->staff(lastStaffInitialIdx)->lines();
                  int spanTo = llines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (llines - 1) * 2;
                  _barLine->setSpanTo(spanTo);
                  _barLine->layout();
                  }
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
//            if (score()->staff(firstStaffInitialIdx)->lines() == 1)   // bbox of one line staff bad?
//                  b->rypos() -= _spatium;
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
//   setInstrumentNames
//---------------------------------------------------------

void System::setInstrumentNames(bool longName)
      {
      //
      // remark: add/remove instrument names is not undo/redoable
      //         as add/remove of systems is not undoable
      //
      if (vbox())                 // ignore vbox
            return;
      if (!score()->showInstrumentNames()
              || (score()->styleB(StyleIdx::hideInstrumentNameIfOneInstrument) && score()->parts().size() == 1)) {
            for (SysStaff* staff : _staves) {
                  foreach (InstrumentName* t, staff->instrumentNames)
                        score()->removeElement(t);
                  }
            return;
            }

      // TODO: ml is normally empty here, so we are unable to retrieve tick
      // thus, staff name does not reflect current instrument

      int tick = ml.empty() ? 0 : ml.front()->tick();
      int staffIdx = 0;
      for (SysStaff* staff : _staves) {
            Staff* s = score()->staff(staffIdx);
            if (!s->isTop() || !s->show()) {
                  foreach (InstrumentName* t, staff->instrumentNames)
                        score()->removeElement(t);
                  ++staffIdx;
                  continue;
                  }

            Part* part = s->part();
            const QList<StaffName>& names = longName? part->longNames(tick) : part->shortNames(tick);

            int idx = 0;
            foreach (const StaffName& sn, names) {
                  InstrumentName* iname = staff->instrumentNames.value(idx);
                  if (iname == 0) {
                        iname = new InstrumentName(score());
                        // iname->setGenerated(true);
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
            ++staffIdx;
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
                        for (const Bracket* bb : _brackets) {
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

            case Element::Type::SYSTEM_DIVIDER:
                  {
                  SystemDivider* sd = static_cast<SystemDivider*>(el);
                  if (sd->dividerType() == SystemDivider::Type::LEFT)
                        _systemDividerLeft = sd;
                  else
                        _systemDividerRight = sd;
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
            case Element::Type::SYSTEM_DIVIDER:
                  if (el == _systemDividerLeft)
                        _systemDividerLeft = 0;
                  else {
                        Q_ASSERT(_systemDividerRight == el);
                        _systemDividerRight = 0;
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
      remove(o);
      add(n);
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
      if (vbox())
            return;
      for (Bracket* b : _brackets)
            func(data, b);

      if (_systemDividerLeft)
            func(data, _systemDividerLeft);
      if (_systemDividerRight)
            func(data, _systemDividerRight);

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
            qDebug("staffY: staves %d: bad staffIdx %d",
               _staves.size(), staffIdx);
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
      if (_systemDividerLeft && _systemDividerLeft->isUserModified())
            _systemDividerLeft->write(xml);
      if (_systemDividerRight && _systemDividerRight->isUserModified())
            _systemDividerRight->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void System::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "SystemDivider") {
                  SystemDivider* sd = new SystemDivider(score());
                  sd->read(e);
                  add(sd);
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
            Segment* firstSeg = m->segments().first();
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
//   minDistance
//    Return the minimum distance between this system and s2
//    without any element collisions.
//
//    this - top system
//    s2   - bottom system
//---------------------------------------------------------

qreal System::minDistance(System* s2) const
      {
//      const qreal systemFrameDistance = score()->styleP(StyleIdx::systemFrameDistance);
//      const qreal frameSystemDistance = score()->styleP(StyleIdx::frameSystemDistance);

      if (vbox() && !s2->vbox())
//            return qMax(frameSystemDistance, -s2->minTop());
            return qMax(vbox()->bottomGap(), -s2->minTop());
      else if (!vbox() && s2->vbox())
//            return qMax(systemFrameDistance, -minBottom());
            return qMax(s2->vbox()->topGap(), -minBottom());
      else if (vbox() && s2->vbox())
            return 0.0;

      qreal minVerticalDistance = score()->styleP(StyleIdx::minVerticalDistance);
      qreal dist                = score()->styleP(StyleIdx::minSystemDistance);
      int lastStaff             = _staves.size() - 1;

      fixedDownDistance = false;

      for (MeasureBase* mb1 : ml) {
            if (mb1->isMeasure()) {
                  Measure* m = toMeasure(mb1);
                  Q_ASSERT(!m->mstaves().empty());
                  Spacer* sp = m->mstaves().back()->_vspacerDown;
                  if (sp) {
                        if (sp->spacerType() == SpacerType::FIXED) {
                              dist = sp->gap();
                              fixedDownDistance = true;
                              break;
                              }
                        else
                              dist = qMax(dist, sp->gap());
                        }
                  }
            }
      if (!fixedDownDistance) {
            for (MeasureBase* mb2 : s2->ml) {
                  if (mb2->isMeasure()) {
                        Measure* m = toMeasure(mb2);
                        Q_ASSERT(!m->mstaves().empty());
                        Spacer* sp = m->mstaves().front()->_vspacerUp;
                        if (sp)
                              dist = qMax(dist, sp->gap());
                        }
                  }

            for (MeasureBase* mb1 : ml) {
                  if (!mb1->isMeasure())
                        continue;
                  Measure* m1 = toMeasure(mb1);
                  qreal bx1 = m1->x();
                  qreal bx2 = m1->x() + m1->width();

                  for (MeasureBase* mb2 : s2->measures()) {
                        if (!mb2->isMeasure())
                              continue;
                        Measure* m2 = toMeasure(mb2);
                        qreal ax1 = mb2->x();
                        if (ax1 >= bx2)
                              break;
                        qreal ax2 = mb2->x() + mb2->width();
                        if (ax2 < bx1)
                              continue;
                        Shape s1 = m1->staffShape(lastStaff).translated(m1->pos());
                        Shape s2 = m2->staffShape(0).translated(m2->pos());
                        qreal d  = s1.minVerticalDistance(s2) + minVerticalDistance;
                        dist = qMax(dist, d - m1->mstaff(lastStaff)->lines->height());
                        }
                  }
            }
      return dist;
      }

//---------------------------------------------------------
//   topDistance
//    return minimum distance to the shape above
//---------------------------------------------------------

qreal System::topDistance(int staffIdx, const Shape& s) const
      {
      qreal dist = -1000000.0;
      for (MeasureBase* mb1 : ml) {
            if (!mb1->isMeasure())
                  continue;
            Measure* m1 = toMeasure(mb1);
            dist = qMax(dist, s.minVerticalDistance(m1->staffShape(staffIdx).translated(m1->pos())));
            }
      return dist;
      }

//---------------------------------------------------------
//   bottomDistance
//---------------------------------------------------------

qreal System::bottomDistance(int staffIdx, const Shape& s) const
      {
      qreal dist = -1000000.0;
      for (MeasureBase* mb1 : ml) {
            if (!mb1->isMeasure())
                  continue;
            Measure* m1 = toMeasure(mb1);
            dist = qMax(dist, m1->staffShape(staffIdx).translated(m1->pos()).minVerticalDistance(s));
            }
      return dist;
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
                  dist = qMin(dist, s->staffShape(0).top());
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
                  dist = qMax(dist, s->staffShape(staffIdx).bottom());
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
            Measure* m = toMeasure(*im);
            if (m != *fm && m->hasSystemHeader())
                  m->removeSystemHeader();
            if (m != *lm && m->hasSystemTrailer())
                  m->removeSystemTrailer();
            }
      }

//---------------------------------------------------------
//   moveBracket
//---------------------------------------------------------

void System::moveBracket(int staffIdx, int srcCol, int dstCol)
      {
      if (vbox())
            return;
      for (Bracket* b : _brackets) {
            if (b->staffIdx() == staffIdx && b->level() == srcCol)
                  b->setLevel(dstCol);
            }
      }

//---------------------------------------------------------
//   pageBreak
//---------------------------------------------------------

bool System::pageBreak() const
      {
      return  ml.back()->pageBreak();
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

int System::endTick() const
      {
      return measures().back()->endTick();
      }
}

