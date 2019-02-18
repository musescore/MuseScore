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
#include "stafflines.h"
#include "bracketItem.h"

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
      for (SpannerSegment* ss : spannerSegments()) {
            if (ss->system() == this)
                  ss->setParent(nullptr);
            }
      for (MeasureBase* mb : measures()) {
            if (mb->system() == this)
                  mb->setSystem(nullptr);
            }
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
      for (MeasureBase* mb : measures()) {
            if (mb->system() == this)
                  mb->setSystem(nullptr);
            }
      ml.clear();
      for (SpannerSegment* ss : _spannerSegments) {
            if (ss->system() == this)
                  ss->setParent(0);       // assume parent() is System
            }
      _spannerSegments.clear();
      // _systemDividers are reused
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

void System::appendMeasure(MeasureBase* mb)
      {
      mb->setSystem(this);
      ml.push_back(mb);
      }

//---------------------------------------------------------
//   removeMeasure
//---------------------------------------------------------

void System::removeMeasure(MeasureBase* mb)
      {
      ml.erase(std::remove(ml.begin(), ml.end(), mb), ml.end());
      if (mb->system() == this)
            mb->setSystem(nullptr);
      }

//---------------------------------------------------------
//   removeLastMeasure
//---------------------------------------------------------

void System::removeLastMeasure()
      {
      if (ml.empty())
            return;
      MeasureBase* mb = ml.back();
      ml.pop_back();
      if (mb->system() == this)
            mb->setSystem(nullptr);
      }

//---------------------------------------------------------
//   vbox
//    a system can only contain one vertical frame
//---------------------------------------------------------

Box* System::vbox() const
      {
      if (!ml.empty()) {
            if (ml[0]->isVBox() || ml[0]->isTBox())
                  return toBox(ml[0]);
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
//   layoutSystem
///   Layout the System
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

      int columns = 0;
      for (int idx = 0; idx < nstaves; ++idx) {
            int c = 0;
            for (auto bi : score()->staff(idx)->brackets())
                  c = qMax(c, bi->column()+1);
            columns = qMax(columns, c);
            }

#if (!defined (_MSCVER) && !defined (_MSC_VER))
      qreal bracketWidth[columns];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<qreal> bracketWidth(columns);
#endif
      for (int i = 0; i < columns; ++i)
            bracketWidth[i] = 0.0;

      QList<Bracket*> bl;
      bl.swap(_brackets);

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* s = score()->staff(staffIdx);
            for (int i = 0; i < columns; ++i) {
                  for (auto bi : s->brackets()) {
                        if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET)
                              continue;
                        int firstStaff = staffIdx;
                        int lastStaff  = staffIdx + bi->bracketSpan() - 1;
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
                        if ((span > 1) || (bi->bracketSpan() == span)) {
                              //
                              // this bracket is visible
                              //
                              Bracket* b = 0;
                              int track = staffIdx * VOICES;
                              for (int k = 0; k < bl.size(); ++k) {
                                    if (bl[k]->track() == track && bl[k]->column() == i && bl[k]->bracketType() == bi->bracketType()) {
                                          b = bl.takeAt(k);
                                          break;
                                          }
                                    }
                              if (b == 0) {
                                    b = new Bracket(score());
                                    b->setBracketItem(bi);
                                    b->setGenerated(true);
                                    b->setTrack(track);
                                    }
                              add(b);
//                              if (bi->selected() != b->selected()) {
//                                    bi->selected() ? score()->select(b) : score()->deselect(b);
//                                    }
                              b->setStaffSpan(firstStaff, lastStaff);
                              bracketWidth[i] = qMax(bracketWidth[i], b->width());
                              }
                        }
                  }
            if (!staff(staffIdx)->show())
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

      _leftMargin = xoff2;

      qreal bd = score()->styleP(Sid::bracketDistance);
      if (!_brackets.empty()) {
            for (int w : bracketWidth)
                  _leftMargin += w + bd;
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
            qreal staffMag = staff->mag(Fraction(0,1));     // ??? TODO
            int staffLines = staff->lines(Fraction(0,1));
            if (staffLines == 1) {
                  qreal h = staff->lineDistance(Fraction(0,1)) * staffMag * spatium();
                  s->bbox().setRect(_leftMargin + xo1, -h, 0.0, 2 * h);
                  }
            else {
                  qreal h = (staffLines - 1) * staff->lineDistance(Fraction(0,1));
                  h = h * staffMag * spatium();
                  s->bbox().setRect(_leftMargin + xo1, 0.0, 0.0, h);
                  }
            }

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      for (Bracket* b : _brackets) {
            qreal xo = -xo1;
            for (const Bracket* b2 : _brackets) {
                   if (b->column() > b2->column() &&
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
                        switch (int(t->align()) & int(Align::HMASK)) {
                              case int(Align::LEFT):
                                    t->rxpos() = 0;
                                    break;
                              case int(Align::HCENTER):
                                    t->rxpos() = (xoff2 - point(instrumentNameOffset) + xo1) * .5;
                                    break;
                              case int(Align::RIGHT):
                              default:
                                    t->rxpos() = xoff2 - point(instrumentNameOffset) + xo1;
                                    break;
                              }
//                        t->rxpos() += t->offset(t->spatium()).x();
                        }
                  }
            idx += p->nstaves();
            }
      }

//---------------------------------------------------------
//   nextVisibleStaff
//---------------------------------------------------------

int System::nextVisibleStaff(int staffIdx) const
      {
      int i;
      for (i = staffIdx + 1; i < _staves.size(); ++i) {
            Staff*    s  = score()->staff(i);
            SysStaff* ss = _staves[i];
            if (s->show() && ss->show())
                  break;
            }
      return i;
      }

//---------------------------------------------------------
//   firstVisibleStaff
//---------------------------------------------------------

int System::firstVisibleStaff() const
      {
      return nextVisibleStaff(-1);
      }

//---------------------------------------------------------
//   layout2
//    called after measure layout
//    adjusts staff distance
//---------------------------------------------------------

void System::layout2()
      {
      Box* vb = vbox();
      if (vb) {
            vb->layout();
            setbbox(vb->bbox());
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
      qreal minVerticalDistance = score()->styleP(Sid::minVerticalDistance);
      qreal staffDistance       = score()->styleP(Sid::staffDistance);
      qreal akkoladeDistance    = score()->styleP(Sid::akkoladeDistance);

      if (visibleStaves.empty()) {
            qDebug("====no visible staves, staves %d, score staves %d", _staves.size(), score()->nstaves());
            return;
            }

      for (auto i = visibleStaves.begin();; ++i) {
            SysStaff* ss  = i->second;
            int si1       = i->first;
            Staff* staff  = score()->staff(si1);
            auto ni       = i + 1;

            qreal h = staff->height();
            if (ni == visibleStaves.end()) {
//                  ss->setYOff(staff->lines(0) == 1 ? _spatium * staff->mag(0) : 0.0);
                  ss->setYOff(0.0);
                  ss->bbox().setRect(_leftMargin, y, width() - _leftMargin, h);
                  break;
                  }

            int si2        = ni->first;
            Staff* staff2  = score()->staff(si2);
            qreal dist     = h;

            switch (staff2->innerBracket()) {
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
            dist += staff2->userDist();
#if 0
            for (MeasureBase* mb : ml) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  Shape& s1  = m->staffShape(si1);
                  Shape& s2  = m->staffShape(si2);

                  qreal d    = score()->lineMode() ? 0.0 : s1.minVerticalDistance(s2);
                  dist       = qMax(dist, d + minVerticalDistance);

                  Spacer* sp = m->vspacerDown(si1);
                  if (sp) {
                        if (sp->spacerType() == SpacerType::FIXED) {
                              dist = staff->height() + sp->gap();
                              break;
                              }
                        else
                              dist = qMax(dist, staff->height() + sp->gap());
                        }
                  sp = m->vspacerUp(si2);
                  if (sp)
                        dist = qMax(dist, sp->gap());
                  }
#else
            bool fixedSpace = false;
            for (MeasureBase* mb : ml) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  Spacer* sp = m->vspacerDown(si1);
                  if (sp) {
                        if (sp->spacerType() == SpacerType::FIXED) {
                              dist = staff->height() + sp->gap();
                              fixedSpace = true;
                              break;
                              }
                        else
                              dist = qMax(dist, staff->height() + sp->gap());
                        }
                  sp = m->vspacerUp(si2);
                  if (sp)
                        dist = qMax(dist, sp->gap());
                  }
            if (!fixedSpace) {
                  qreal d = score()->lineMode() ? 0.0 : ss->skyline().minDistance(System::staff(si2)->skyline());
                  dist = qMax(dist, d + minVerticalDistance);
                  }
#endif

//            ss->setYOff(staff->lines(0) == 1 ? _spatium * staff->mag(0) : 0.0);
            ss->setYOff(0.0);
            ss->bbox().setRect(_leftMargin, y, width() - _leftMargin, h);
            y += dist;
            }

      qreal systemHeight = staff(visibleStaves.back().first)->bbox().bottom();
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
                                          SysStaff* s3 = staff(i);
                                          if (s3->show()) {
                                                s2 = s3;
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
                        t->rypos() = y1 + (y2 - y1) * .5 + t->offset().y();
                        }
                  }
            staffIdx += nstaves;
            }
      }

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void System::setInstrumentNames(bool longName, Fraction tick)
      {
      //
      // remark: add/remove instrument names is not undo/redoable
      //         as add/remove of systems is not undoable
      //
      if (vbox())                 // ignore vbox
            return;
      if (!score()->showInstrumentNames()
              || (score()->styleB(Sid::hideInstrumentNameIfOneInstrument) && score()->parts().size() == 1)) {
            for (SysStaff* staff : _staves) {
                  foreach (InstrumentName* t, staff->instrumentNames)
                        score()->removeElement(t);
                  }
            return;
            }

      int staffIdx = 0;
      for (SysStaff* staff : _staves) {
            Staff* s = score()->staff(staffIdx);
            if (!s->isTop() || !s->show()) {
                  for (InstrumentName* t : staff->instrumentNames)
                        score()->removeElement(t);
                  ++staffIdx;
                  continue;
                  }

            Part* part = s->part();
            const QList<StaffName>& names = longName? part->longNames(tick) : part->shortNames(tick);

            int idx = 0;
            for (const StaffName& sn : names) {
                  InstrumentName* iname = staff->instrumentNames.value(idx);
                  if (iname == 0) {
                        iname = new InstrumentName(score());
                        // iname->setGenerated(true);
                        iname->setParent(this);
                        iname->setTrack(staffIdx * VOICES);
                        iname->setInstrumentNameType(longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT);
                        iname->setLayoutPos(sn.pos());
                        score()->addElement(iname);
                        }
                  iname->setXmlText(sn.name());
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
            case ElementType::INSTRUMENT_NAME:
// qDebug("  staffIdx %d, staves %d", el->staffIdx(), _staves.size());
                  _staves[el->staffIdx()]->instrumentNames.append(toInstrumentName(el));
                  break;

            case ElementType::BEAM:
                  score()->addElement(el);
                  break;

            case ElementType::BRACKET: {
                  Bracket* b   = toBracket(el);
#if 0
                  int staffIdx = b->staffIdx();
                  int column   = b->column();
                  if (column == -1) {
                        column = 0;
                        for (const Bracket* bb : _brackets) {
                              if (staffIdx >= bb->firstStaff() && staffIdx <= bb->lastStaff())
                                    ++column;
                              }
//                        b->setLevel(column);
//                        b->setSpan(1);
                        }
//                  b->staff()->setBracket(column,     b->bracketType());
//                  b->staff()->setBracketSpan(column, b->span());
#endif
                  _brackets.append(b);
                  }
                  break;

            case ElementType::MEASURE:
            case ElementType::HBOX:
            case ElementType::VBOX:
            case ElementType::TBOX:
            case ElementType::FBOX:
                  score()->addElement(el);
                  break;
            case ElementType::TEXTLINE_SEGMENT:
            case ElementType::HAIRPIN_SEGMENT:
            case ElementType::OTTAVA_SEGMENT:
            case ElementType::TRILL_SEGMENT:
            case ElementType::VIBRATO_SEGMENT:
            case ElementType::VOLTA_SEGMENT:
            case ElementType::SLUR_SEGMENT:
            case ElementType::TIE_SEGMENT:
            case ElementType::PEDAL_SEGMENT:
            case ElementType::LYRICSLINE_SEGMENT:
            case ElementType::GLISSANDO_SEGMENT:
            case ElementType::LET_RING_SEGMENT:
            case ElementType::PALM_MUTE_SEGMENT:
                  {
                  SpannerSegment* ss = toSpannerSegment(el);
#ifndef NDEBUG
                  if (_spannerSegments.contains(ss))
                        qDebug("System::add() %s %p already there", ss->name(), ss);
                  else
#endif
                  _spannerSegments.append(ss);
                  }
                  break;

            case ElementType::SYSTEM_DIVIDER:
                  {
                  SystemDivider* sd = toSystemDivider(el);
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
            case ElementType::INSTRUMENT_NAME:
                  _staves[el->staffIdx()]->instrumentNames.removeOne(toInstrumentName(el));
                  break;
            case ElementType::BEAM:
                  score()->removeElement(el);
                  break;
            case ElementType::BRACKET:
                  {
                  Bracket* b = toBracket(el);
                  if (!_brackets.removeOne(b))
                        qDebug("System::remove: bracket not found");
                  }
                  break;
            case ElementType::MEASURE:
            case ElementType::HBOX:
            case ElementType::VBOX:
            case ElementType::TBOX:
            case ElementType::FBOX:
                  score()->removeElement(el);
                  break;
            case ElementType::TEXTLINE_SEGMENT:
            case ElementType::HAIRPIN_SEGMENT:
            case ElementType::OTTAVA_SEGMENT:
            case ElementType::TRILL_SEGMENT:
            case ElementType::VIBRATO_SEGMENT:
            case ElementType::VOLTA_SEGMENT:
            case ElementType::SLUR_SEGMENT:
            case ElementType::TIE_SEGMENT:
            case ElementType::PEDAL_SEGMENT:
            case ElementType::LYRICSLINE_SEGMENT:
            case ElementType::GLISSANDO_SEGMENT:
                  if (!_spannerSegments.removeOne(toSpannerSegment(el))) {
                        qDebug("System::remove: %p(%s) not found, score %p", el, el->name(), score());
                        Q_ASSERT(score() == el->score());
                        }
                  break;
            case ElementType::SYSTEM_DIVIDER:
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

Fraction System::snap(const Fraction& tick, const QPointF p) const
      {
      for (const MeasureBase* m : ml) {
            if (p.x() < m->x() + m->width())
                  return toMeasure(m)->snap(tick, p - m->pos()); //TODO: MeasureBase
            }
      return toMeasure(ml.back())->snap(tick, p-pos());          //TODO: MeasureBase
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

Fraction System::snapNote(const Fraction& tick, const QPointF p, int staff) const
      {
      for (const MeasureBase* m : ml) {
            if (p.x() < m->x() + m->width())
                  return toMeasure(m)->snapNote(tick, p - m->pos(), staff);  //TODO: MeasureBase
            }
      return toMeasure(ml.back())->snap(tick, p-pos());          // TODO: MeasureBase
      }

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* System::firstMeasure() const
      {
      auto i = std::find_if(ml.begin(), ml.end(), [](MeasureBase* mb){ return mb->isMeasure(); });
      return i != ml.end() ? toMeasure(*i) : 0;
      }

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* System::lastMeasure() const
      {
      auto i = std::find_if(ml.rbegin(), ml.rend(), [](MeasureBase* mb){return mb->isMeasure();});
      return i != ml.rend() ? toMeasure(*i) : 0;
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
                  if (se && se->isChordRest()) {
                        ChordRest* cr = toChordRest(se);
                        Measure* m    = cr->measure();
                        v1            = m->visible(cr->staffIdx());
                        }
                  bool v2 = true;
                  if (!v1 && ee && ee->isChordRest()) {
                        ChordRest* cr = toChordRest(ee);
                        Measure* m    = cr->measure();
                        v2            = m->visible(cr->staffIdx());
                        }
                  v = v1 || v2; // hide spanner if both chords are hidden
                  }
            if (all || (score()->staff(staffIdx)->show() && _staves[staffIdx]->show() && v) || spanner->isVolta())
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
            qFatal("staffY: staves %d: bad staffIdx %d", _staves.size(), staffIdx);
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

void System::write(XmlWriter& xml) const
      {
      xml.stag(this);
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
//   nextSegmentElement
//---------------------------------------------------------

Element* System::nextSegmentElement()
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
//   prevSegmentElement
//---------------------------------------------------------

Element* System::prevSegmentElement()
      {
      Segment* seg = firstMeasure()->first();
      Element* re = 0;
      while (!re) {
            seg = seg->prev1MM();
            if (!seg)
                  return score()->lastElement();

            if (seg->segmentType() == SegmentType::EndBarLine)
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
      if (vbox() && !s2->vbox())
            return qMax(vbox()->bottomGap(), s2->minTop());
      else if (!vbox() && s2->vbox())
            return qMax(s2->vbox()->topGap(), minBottom());
      else if (vbox() && s2->vbox())
            return s2->vbox()->topGap() + vbox()->bottomGap();

      qreal minVerticalDistance = score()->styleP(Sid::minVerticalDistance);
      qreal dist                = score()->styleP(Sid::minSystemDistance);
      int firstStaff;
      int lastStaff;

      for (firstStaff = 0; firstStaff < _staves.size()-1; ++firstStaff) {
            if (s2->staff(firstStaff)->show())
                  break;
            }
      for (lastStaff = _staves.size() -1; lastStaff > 0; --lastStaff) {
            if (staff(lastStaff)->show())
                  break;
            }

      dist = qMax(dist, score()->staff(firstStaff)->userDist());
      fixedDownDistance = false;

      for (MeasureBase* mb1 : ml) {
            if (mb1->isMeasure()) {
                  Measure* m = toMeasure(mb1);
                  Spacer* sp = m->vspacerDown(lastStaff);
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
                        Spacer* sp = m->vspacerUp(firstStaff);
                        if (sp)
                              dist = qMax(dist, sp->gap());
                        }
                  }
            qreal sld = staff(lastStaff)->skyline().minDistance(s2->staff(firstStaff)->skyline());
            sld -=  staff(lastStaff)->bbox().height() - minVerticalDistance;
            dist = qMax(dist, sld);
//            dist = dist - staff(lastStaff)->bbox().height() + minVerticalDistance;
            }
      return dist;
      }

//---------------------------------------------------------
//   topDistance
//    return minimum distance to the above south skyline
//---------------------------------------------------------

qreal System::topDistance(int staffIdx, const SkylineLine& s) const
      {
      Q_ASSERT(!vbox());
      Q_ASSERT(!s.isNorth());
      if (score()->lineMode())
            return 0.0;
      return s.minDistance(staff(staffIdx)->skyline().north());
      }

//---------------------------------------------------------
//   bottomDistance
//---------------------------------------------------------

qreal System::bottomDistance(int staffIdx, const SkylineLine& s) const
      {
      Q_ASSERT(!vbox());
      Q_ASSERT(s.isNorth());
      if (score()->lineMode())
            return 0.0;
      return staff(staffIdx)->skyline().south().minDistance(s);
      }

//---------------------------------------------------------
//   firstVisibleSysStaff
//---------------------------------------------------------

SysStaff* System::firstVisibleSysStaff() const
      {
      for (SysStaff* s : _staves) {
            if (s->show())
                  return s;
            }
      qDebug("no sys staff");
      return 0;
      }

//---------------------------------------------------------
//   lastVisibleSysStaff
//---------------------------------------------------------

SysStaff* System::lastVisibleSysStaff() const
      {
      for (int i = _staves.size() - 1; i >= 0; --i) {
            if (_staves[i]->show())
                  return _staves[i];
            }
      qDebug("no sys staff");
      return 0;
      }

//---------------------------------------------------------
//   minTop
//    Return the minimum top margin.
//---------------------------------------------------------

qreal System::minTop() const
      {
      SysStaff* s = firstVisibleSysStaff();
      if (s)
            return -s->skyline().north().max();
      return 0.0;
      }

//---------------------------------------------------------
//   minBottom
//    Return the minimum bottom margin.
//---------------------------------------------------------

qreal System::minBottom() const
      {
      if (vbox())
            return vbox()->height() + vbox()->bottomGap();
      SysStaff* s = lastVisibleSysStaff();
      if (s)
            return s->skyline().south().max();
      return 0.0;
      }

//---------------------------------------------------------
//   moveBracket
//---------------------------------------------------------

void System::moveBracket(int /*staffIdx*/, int /*srcCol*/, int /*dstCol*/)
      {
#if 0
printf("System::moveBracket\n");
      if (vbox())
            return;
      for (Bracket* b : _brackets) {
            if (b->staffIdx() == staffIdx && b->column() == srcCol)
                  b->setLevel(dstCol);
            }
#endif
      }

//---------------------------------------------------------
//   pageBreak
//---------------------------------------------------------

bool System::pageBreak() const
      {
      return  ml.empty() ? false : ml.back()->pageBreak();
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction System::endTick() const
      {
      return measures().back()->endTick();
      }
}

