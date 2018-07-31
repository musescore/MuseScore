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
 Implementation of most part of class Measure.
*/

#include "measure.h"
#include "accidental.h"
#include "ambitus.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "clef.h"
#include "drumset.h"
#include "duration.h"
#include "dynamic.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "hook.h"
#include "icon.h"
#include "image.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "layout.h"
#include "lyrics.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "pedal.h"
#include "pitchspelling.h"
#include "repeat.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sig.h"
#include "slur.h"
#include "spacer.h"
#include "staff.h"
#include "stafftext.h"
#include "stafftype.h"
#include "stringdata.h"
#include "style.h"
#include "sym.h"
#include "system.h"
#include "tempotext.h"
#include "text.h"
#include "tie.h"
#include "tiemap.h"
#include "timesig.h"
#include "tremolo.h"
#include "trill.h"
#include "tuplet.h"
#include "tupletmap.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

MStaff::MStaff()
      {
      _noText      = 0;
      distanceUp   = .0;
      distanceDown = .0;
      lines        = 0;
      hasVoices    = false;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = true;
      _slashStyle  = false;
      }

MStaff::~MStaff()
      {
      delete _noText;
      delete lines;
      delete _vspacerUp;
      delete _vspacerDown;
      }

MStaff::MStaff(const MStaff& m)
      {
      _noText      = 0;
      distanceUp   = m.distanceUp;
      distanceDown = m.distanceDown;
      lines        = new StaffLines(*m.lines);
      hasVoices    = m.hasVoices;
      _vspacerUp   = m._vspacerUp ? new Spacer(*m._vspacerUp) : 0;
      _vspacerDown = m._vspacerDown ? new Spacer(*m._vspacerDown) : 0;
      _visible     = m._visible;
      _slashStyle  = m._slashStyle;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : MeasureBase(s),
     _timesig(4,4), _len(4,4)
      {
      _repeatCount           = 2;
      _repeatFlags           = Repeat::NONE;

      int n = _score->nstaves();
      staves.reserve(n);
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(staffIdx);
            s->lines     = new StaffLines(score());
            s->lines->setTrack(staffIdx * VOICES);
            s->lines->setParent(this);
            s->lines->setVisible(!staff->invisible());
            staves.push_back(s);
            }

      _minWidth1             = 0.0;
      _minWidth2             = 0.0;

      _no                    = 0;
      _noOffset              = 0;
      _noMode                = MeasureNumberMode::AUTO;
      _userStretch           = 1.0;     // ::style->measureSpacing;
      _irregular             = false;
      _breakMultiMeasureRest = false;
      _breakMMRest           = false;
      _endBarLineColor       = MScore::defaultColor;
      _endBarLineGenerated   = true;
      _endBarLineVisible     = true;
      _endBarLineType        = BarLineType::NORMAL;
      _systemInitialBarLineType = BarLineType::NORMAL;
      _mmRest                = 0;
      _mmRestCount           = 0;
      setFlag(ElementFlag::MOVABLE, true);
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure::Measure(const Measure& m)
   : MeasureBase(m)
      {
      _segments              = m._segments.clone();
      _timesig               = m._timesig;
      _len                   = m._len;
      _repeatCount           = m._repeatCount;
      _repeatFlags           = m._repeatFlags;

      staves.reserve(m.staves.size());
      foreach(MStaff* ms, m.staves)
            staves.append(new MStaff(*ms));

      _minWidth1             = m._minWidth1;
      _minWidth2             = m._minWidth2;

      _no                    = m._no;
      _noOffset              = m._noOffset;
      _userStretch           = m._userStretch;

      _irregular             = m._irregular;
      _breakMultiMeasureRest = m._breakMultiMeasureRest;
      _breakMMRest           = m._breakMMRest;
      _endBarLineGenerated   = m._endBarLineGenerated;
      _endBarLineVisible     = m._endBarLineVisible;
      _endBarLineType        = m._endBarLineType;
      _systemInitialBarLineType = m._systemInitialBarLineType;    // possibly should be reset to NORMAL?
      _mmRest                = m._mmRest;
      _mmRestCount           = m._mmRestCount;
      _playbackCount         = m._playbackCount;
      _endBarLineColor       = m._endBarLineColor;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Measure::setScore(Score* score)
      {
      MeasureBase::setScore(score);
      for (Segment* s = first(); s; s = s->next())
            s->setScore(score);
      }

//---------------------------------------------------------
//   MStaff::setScore
//---------------------------------------------------------

void MStaff::setScore(Score* score)
      {
      if (lines)
            lines->setScore(score);
      if (_vspacerUp)
            _vspacerUp->setScore(score);
      if (_vspacerDown)
            _vspacerDown->setScore(score);
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::~Measure()
      {
      for (Segment* s = first(); s;) {
            Segment* ns = s->next();
            delete s;
            s = ns;
            }
      qDeleteAll(staves);
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

/**
 Debug only.
*/

void Measure::dump() const
      {
      qDebug("dump measure:");
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Measure::remove(Segment* el)
      {
#ifndef NDEBUG
      if (score()->undoRedo()) {
            qFatal("remove segment <%s> in undo/redo", el->subTypeName());
            }

      // Q_ASSERT(!score()->undoRedo());
      Q_ASSERT(el->type() == Element::Type::SEGMENT);
      Q_ASSERT(el->score() == score());
      if (el->prev()) {
            Q_ASSERT(el->prev()->next() == el);
            }
      else {
            Q_ASSERT(el == _segments.first());
            if (el != _segments.first())
                  return;
            }

      if (el->next()) {
            Q_ASSERT(el->next()->prev() == el);
            }
      else {
            Q_ASSERT(el == _segments.last());
            }
#endif
#if 0
      int tracks = staves.size() * VOICES;
      for (int track = 0; track < tracks; track += VOICES) {
            if (!el->element(track))
                  continue;
            if (el->segmentType() == Segment::Type::KeySig)
                  score()->staff(track/VOICES)->setUpdateKeymap(true);
            }
#endif
      _segments.remove(el);
      setDirty();
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      qreal x;
      };

//---------------------------------------------------------
//   layoutCR0
//---------------------------------------------------------

void Measure::layoutCR0(ChordRest* cr, qreal mm, AccidentalState* as)
      {
      qreal m = mm;
      if (cr->small())
            m *= score()->styleD(StyleIdx::smallNoteMag);

      if (cr->type() == Element::Type::CHORD) {
            Chord* chord = static_cast<Chord*>(cr);
            for (Chord* c : chord->graceNotes())
                  layoutCR0(c, mm, as);
            if (!chord->isGrace())
                  chord->cmdUpdateNotes(as);          // TODO: merge cmdUpdateNotes()

            if (chord->noteType() != NoteType::NORMAL)
                  m *= score()->styleD(StyleIdx::graceNoteMag);
            const Drumset* drumset = 0;
            if (cr->part()->instrument()->useDrumset())
                  drumset = cr->part()->instrument()->drumset();
            if (drumset) {
                  for (Note* note : chord->notes()) {
                        int pitch = note->pitch();
                        if (!drumset->isValid(pitch)) {
                              // qDebug("unmapped drum note %d", pitch);
                              }
                        else if (!note->fixed()) {
                              note->undoChangeProperty(P_ID::HEAD_GROUP, int(drumset->noteHead(pitch)));
                              // note->setHeadGroup(drumset->noteHead(pitch));
                              note->setLine(drumset->line(pitch));
                              continue;
                              }
                        }
                  }
            chord->computeUp();
            chord->layoutStem1();
            }
      if (m != mag()) {
            cr->setMag(m);
            setDirty();
            }
      }

//---------------------------------------------------------
//   findAccidental
///   return current accidental value at note position
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Note* note) const
      {
      AccidentalState tversatz;  // state of already set accidentals for this measure
      tversatz.init(note->chord()->staff()->key(tick()));

      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            int startTrack = note->staffIdx() * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != Element::Type::CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  for (Chord* chord1 : chord->graceNotes()) {
                        for (Note* note1 : chord1->notes()) {
                              if (note1->tieBack())
                                    continue;
                              //
                              // compute accidental
                              //
                              int tpc  = note1->tpc();
                              int line = absStep(tpc, note1->epitch());

                              if (note == note1)
                                    return tversatz.accidentalVal(line);
                              tversatz.setAccidentalVal(line, tpc2alter(tpc));
                              }
                        }
                  for (Note* note1 : chord->notes()) {
                        if (note1->tieBack())
                              continue;
                        //
                        // compute accidental
                        //
                        int tpc  = note1->tpc();
                        int line = absStep(tpc, note1->epitch());

                        if (note == note1)
                              return tversatz.accidentalVal(line);
                        tversatz.setAccidentalVal(line, tpc2alter(tpc));
                        }
                  }
            }
      qDebug("Measure::findAccidental: note not found");
      return AccidentalVal::NATURAL;
      }

//---------------------------------------------------------
//   findAccidental
///   Compute accidental state at segment/staffIdx for
///   relative staff line.
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Segment* s, int staffIdx, int line, bool &error) const
      {
      AccidentalState tversatz;  // state of already set accidentals for this measure
      Staff* staff = score()->staff(staffIdx);
      tversatz.init(staff->key(tick()));

      Segment::Type st = Segment::Type::ChordRest;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            if (segment == s && staff->isPitchedStaff()) {
                  ClefType clef = staff->clef(s->tick());
                  int l = relStep(line, clef);
                  return tversatz.accidentalVal(l, error);
                  }
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != Element::Type::CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  for (Chord* chord1 : chord->graceNotes()) {
                        for (Note* note : chord1->notes()) {
                              if (note->tieBack())
                                    continue;
                              int tpc  = note->tpc();
                              int l    = absStep(tpc, note->epitch());
                              tversatz.setAccidentalVal(l, tpc2alter(tpc));
                              }
                        }

                  for (Note* note : chord->notes()) {
                        if (note->tieBack())
                              continue;
                        int tpc    = note->tpc();
                        int l      = absStep(tpc, note->epitch());
                        tversatz.setAccidentalVal(l, tpc2alter(tpc));
                        }
                  }
            }
      qDebug("segment not found");
      return AccidentalVal::NATURAL;
      }

//---------------------------------------------------------
//   Measure::layout
///   Layout measure; must fit into  \a width.
///
///   Note: minWidth = width - stretch
//---------------------------------------------------------

void Measure::layoutWidth(qreal width)
      {
      int nstaves = _score->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            staves[staffIdx]->distanceUp   = 0.0;
            staves[staffIdx]->distanceDown = 0.0;
            StaffLines* sl = staves[staffIdx]->lines;
            if (sl)
                  sl->setMag(score()->staff(staffIdx)->mag());
            staves[staffIdx]->lines->layout();
            }

      // height of boundingRect will be set in system->layout2()
      // keep old value for relayout

      bbox().setRect(0.0, 0.0, width, height());
      layoutX(width);
      }

//---------------------------------------------------------
//   tick2pos
//    return x position for tick relative to System
//---------------------------------------------------------

qreal Measure::tick2pos(int tck) const
      {
      if (isMMRest()) {
            Segment* s = first(Segment::Type::ChordRest);
            qreal x1 = s->x();
            qreal w  = width() - x1;
            return x1 + (tck * w) / (ticks() * mmRestCount());
            }

      Segment* s;
      qreal x1 = 0;
      qreal x2 = 0;
      int tick1 = tick();
      int tick2 = tick1;
      for (s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            x2    = s->x();
            tick2 = s->tick();
            if (tck == tick2)
                  return x2 + pos().x();
            if (tck <= tick2)
                  break;
            x1    = x2;
            tick1 = tick2;
            }
      if (s == 0) {
            x2    = width();
            tick2 = endTick();
            }
      qreal dx = x2 - x1;
      int dt   = tick2 - tick1;
      x1      += (dt == 0) ? 0.0 : (dx * (tck - tick1) / dt);
      return x1 + pos().x();
      }

//---------------------------------------------------------
//   layout2
//    called after layout of all pages
//---------------------------------------------------------

void Measure::layout2()
      {
      if (parent() == 0)
            return;

      Q_ASSERT(score()->nstaves() == staves.size());

      int tracks = score()->nstaves() * VOICES;
      qreal _spatium = spatium();
      static const Segment::Type st { Segment::Type::ChordRest };
      for (int track = 0; track < tracks; ++track) {
            for (Segment* s = first(st); s; s = s->next(st)) {
                  ChordRest* cr = s->cr(track);
                  if (!cr)
                        continue;
/* Lyrics line segments are now Spanner's belonging to System's: System takes care of them
                   int n = cr->lyricsList().size();
                  for (int i = 0; i < n; ++i) {
                        Lyrics* lyrics = cr->lyricsList().at(i);
                        if (lyrics)
                              system()->layoutLyrics(lyrics, s, track/VOICES);
                        } */
                  }
            if (track % VOICES == 0) {
                  int staffIdx = track / VOICES;
                  qreal y = system()->staff(staffIdx)->y();
                  Spacer* sp = staves[staffIdx]->_vspacerDown;
                  if (sp) {
                        sp->layout();
                        int n = score()->staff(staffIdx)->lines() - 1;
                        sp->setPos(_spatium * .5, y + n * _spatium);
                        }
                  sp = staves[staffIdx]->_vspacerUp;
                  if (sp) {
                        sp->layout();
                        sp->setPos(_spatium * .5, y - sp->gap());
                        }
                  }
            }
      for (MStaff* ms : staves)
            ms->lines->setWidth(width());

      MeasureBase::layout();  // layout LAYOUT_BREAK elements

      //
      //   set measure number
      //
      bool smn = false;

      if (_noMode == MeasureNumberMode::SHOW)
            smn = true;
      else if (_noMode == MeasureNumberMode::HIDE)
            smn = false;
      else {
            if (score()->styleB(StyleIdx::showMeasureNumber)
               && !_irregular
               && (_no || score()->styleB(StyleIdx::showMeasureNumberOne))) {
                  if (score()->styleB(StyleIdx::measureNumberSystem))
                        smn = system()->firstMeasure() == this;
                  else {
                        smn = (_no == 0 && score()->styleB(StyleIdx::showMeasureNumberOne)) ||
                              ( ((_no + 1) % score()->style(StyleIdx::measureNumberInterval).toInt()) == (score()->styleB(StyleIdx::showMeasureNumberOne) ? 1 : 0) ) ||
                              (score()->style(StyleIdx::measureNumberInterval).toInt() == 1);
                        }
                  }
            }
      QString s;
      if (smn)
            s = QString("%1").arg(_no + 1);
      int nn = 1;
      bool nas = score()->styleB(StyleIdx::measureNumberAllStaffs);

      if (!nas) {
            //find first non invisible staff
            for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
                  MStaff* ms = staves.at(staffIdx);
                  SysStaff* s  = system()->staff(staffIdx);
                  Staff* staff = score()->staff(staffIdx);
                  if (ms->visible() && staff->show() && s->show()) {
                        nn = staffIdx;
                        break;
                        }
                  }
            }
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            MStaff* ms = staves.at(staffIdx);
            Text* t = ms->noText();
            if (t) {
                  Q_ASSERT(t->score() == score());
                  Q_ASSERT(t->parent() == this);
                  t->setTrack(staffIdx * VOICES);
                  }
            if (smn && ((staffIdx == nn) || nas)) {
                  if (t == 0) {
                        t = new Text(score());
                        t->setFlag(ElementFlag::ON_STAFF, true);
                        t->setTrack(staffIdx * VOICES);
                        t->setGenerated(true);
                        t->setTextStyleType(TextStyleType::MEASURE_NUMBER);
                        t->setParent(this);
                        score()->undo(new AddElement(t));
                        // score()->undoAddElement(t);
                        }
                  t->setXmlText(s);
                  t->layout();
                  }
            else {
                  if (t)
                        score()->undo(new RemoveElement(t));
                  }
            }

      //
      // slur layout needs articulation layout first
      //
      for (Segment* s = first(st); s; s = s->next(st)) {
            for (int track = 0; track < tracks; ++track) {
                  if (!score()->staff(track / VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  if (!cr)
                        continue;

                  if (cr->type() == Element::Type::CHORD) {
                        Chord* c = static_cast<Chord*>(cr);
                        for (const Note* note : c->notes()) {
                              Tie* tie = note->tieFor();
                              if (tie)
                                    tie->layout();
                              foreach (Spanner* sp, note->spannerFor())
                                    sp->layout();
                              }
                        }
                  DurationElement* de = cr;
                  while (de->tuplet() && de->tuplet()->elements().front() == de) {
                        de->tuplet()->layout();
                        de = de->tuplet();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   findChord
///   Search for chord at position \a tick in \a track
//---------------------------------------------------------

Chord* Measure::findChord(int tick, int track)
      {
      for (Segment* seg = last(); seg; seg = seg->prev()) {
            if (seg->tick() < tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(track);
                  if (el && el->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(el);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   findChordRest
///   Search for chord or rest at position \a tick at \a staff in \a voice.
//---------------------------------------------------------

ChordRest* Measure::findChordRest(int tick, int track)
      {
      for (Segment* seg = first(); seg; seg = seg->next()) {
            if (seg->tick() > tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(track);
                  if (el && (el->type() == Element::Type::CHORD || el->type() == Element::Type::REST)) {
                        return (ChordRest*)el;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Measure::tick2segment(int tick, Segment::Type st) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->tick() == tick) {
                  if ( (s->segmentType() & st) != 0)
                        return s;
                  }
            if (s->tick() > tick)
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
/// Search for a segment of type \a st at position \a t.
//---------------------------------------------------------

Segment* Measure::findSegment(Segment::Type st, int t)
      {
      Segment* s;
      for (s = first(); s && s->tick() < t; s = s->next())
            ;
      for (; s && s->tick() == t; s = s->next()) {
            if (s->segmentType() & st)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   undoGetSegment
//---------------------------------------------------------

Segment* Measure::undoGetSegment(Segment::Type type, int tick)
      {
      Segment* s = findSegment(type, tick);
      if (s == 0) {
            s = new Segment(this, type, tick);
            score()->undoAddElement(s);
            }
      return s;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Element* e, int tick)
      {
      return getSegment(Segment::segmentType(e->type()), tick);
      }

//---------------------------------------------------------
//   getSegment
///   Get a segment of type \a st at tick position \a t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegment(Segment::Type st, int tick)
      {
      Segment* s = findSegment(st, tick);
      if (!s) {
            s = new Segment(this, st, tick);
            add(s);
            }
      return s;
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to Measure.
//---------------------------------------------------------

void Measure::add(Element* el)
      {
      setDirty();
      el->setParent(this);
      Element::Type type = el->type();

//      if (MScore::debugMode)
//            qDebug("measure %p(%d): add %s %p", this, _no, el->name(), el);

      switch (type) {
            case Element::Type::TEXT:
                  {
                  if (el->staffIdx() < staves.size())
                        staves[el->staffIdx()]->setNoText(static_cast<Text*>(el));
                  }
                  break;

            case Element::Type::SPACER:
                  {
                  Spacer* sp = static_cast<Spacer*>(el);
                  if (sp->spacerType() == SpacerType::UP)
                        staves[el->staffIdx()]->_vspacerUp = sp;
                  else if (sp->spacerType() == SpacerType::DOWN)
                        staves[el->staffIdx()]->_vspacerDown = sp;
                  }
                  break;
            case Element::Type::SEGMENT:
                  {
                  Segment* seg = static_cast<Segment*>(el);

                  // insert segment at specific position
                  if (seg->next()) {
                        _segments.insert(seg, seg->next());
                        break;
                        }
                  int t  = seg->tick();
                  Segment::Type st = seg->segmentType();
                  Segment* s;
                  for (s = first(); s && s->tick() < t; s = s->next())
                        ;
                  if (s) {
                        if (st == Segment::Type::ChordRest) {
                              // add ChordRest segment after all other segments with same tick
                              // except EndBarLine
                              while (s && s->segmentType() != st && s->tick() == t) {
                                    if (s->segmentType() == Segment::Type::EndBarLine) {
                                          break;
                                          }
                                    s = s->next();
                                    }
                              }
                        else {
                              // use order of segments in segment.h
                              if (s && s->tick() == t) {
                                    while (s && s->segmentType() <= st) {
                                          s = s->next();
                                          if (s && s->tick() != t)
                                                break;
                                          }
                                    }
                              }
                        }
                  seg->setParent(this);

                  _segments.insert(seg, s);
                  if ((seg->segmentType() == Segment::Type::TimeSig) && seg->element(0)) {
                        score()->addLayoutFlags(LayoutFlag::FIX_TICKS);
                        }
                  }
                  break;

            case Element::Type::JUMP:
                  _repeatFlags = _repeatFlags | Repeat::JUMP;
                  // fall through

            case Element::Type::MARKER:
                  _el.push_back(el);
                  break;

            case Element::Type::HBOX:
                  if (el->staff())
                        el->setMag(el->staff()->mag());     // ?!
                  _el.push_back(el);
                  break;

            case Element::Type::MEASURE:
                  _mmRest = static_cast<Measure*>(el);
                  break;

            default:
                  MeasureBase::add(el);
                  break;
            }

      }

//---------------------------------------------------------
//   remove
///   Remove Element \a el from Measure.
//---------------------------------------------------------

void Measure::remove(Element* el)
      {
      Q_ASSERT(el->parent() == this);
      Q_ASSERT(el->score() == score());

      setDirty();
      switch(el->type()) {
            case Element::Type::TEXT:
                  staves[el->staffIdx()]->setNoText(static_cast<Text*>(0));
                  break;

            case Element::Type::SPACER:
                  if (static_cast<Spacer*>(el)->spacerType() == SpacerType::DOWN)
                        staves[el->staffIdx()]->_vspacerDown = 0;
                  else if (static_cast<Spacer*>(el)->spacerType() == SpacerType::UP)
                        staves[el->staffIdx()]->_vspacerUp = 0;
                  break;

            case Element::Type::SEGMENT:
                  remove(static_cast<Segment*>(el));
                  break;

            case Element::Type::JUMP:
                  resetRepeatFlag(Repeat::JUMP);
                  // fall through

            case Element::Type::MARKER:
            case Element::Type::HBOX:
                  if (!_el.remove(el)) {
                        qDebug("Measure(%p)::remove(%s,%p) not found",
                           this, el->name(), el);
                        }
                  break;

            case Element::Type::CLEF:
            case Element::Type::CHORD:
            case Element::Type::REST:
            case Element::Type::TIMESIG:
                  for (Segment* segment = first(); segment; segment = segment->next()) {
                        int staves = _score->nstaves();
                        int tracks = staves * VOICES;
                        for (int track = 0; track < tracks; ++track) {
                              Element* e = segment->element(track);
                              if (el == e) {
                                    segment->setElement(track, 0);
                                    return;
                                    }
                              }
                        }
                  qDebug("Measure::remove: %s %p not found", el->name(), el);
                  break;

            case Element::Type::MEASURE:
                  _mmRest = 0;
                  break;

            default:
                  MeasureBase::remove(el);
                  break;
            }
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Measure::change(Element* o, Element* n)
      {
      if (o->type() == Element::Type::TUPLET) {
            Tuplet* t = static_cast<Tuplet*>(n);
            foreach(DurationElement* e, t->elements()) {
                  e->setTuplet(t);
                  }
            }
      else {
            remove(o);
            add(n);
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Measure::spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/)
      {
      setDirty();
      }

//-------------------------------------------------------------------
//   moveTicks
//    Also adjust endBarLine if measure len has changed. For this
//    diff == 0 cannot be optimized away
//-------------------------------------------------------------------

void Measure::moveTicks(int diff)
      {
      std::set<Tuplet*> tuplets;
      setTick(tick() + diff);
      for (Segment* segment = first(); segment; segment = segment->next()) {
            if (segment->segmentType() & (Segment::Type::EndBarLine | Segment::Type::TimeSigAnnounce))
                  segment->setTick(tick() + ticks());
            else if (segment->isChordRest())
                  // Tuplet ticks are stored as absolute ticks, so they must be adjusted.
                  // But each tuplet must only be adjusted once.
                  for (Element* e : segment->elist())
                        if (e && e->isChordRest()) {
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              Tuplet* tuplet = cr->tuplet();
                              if (tuplet && tuplets.count(tuplet) == 0) {
                                    tuplet->setTick(tuplet->tick() + diff);
                                    tuplets.insert(tuplet);
                                    }
                              }
            }
      tuplets.clear();
      }

//---------------------------------------------------------
//   removeStaves
//---------------------------------------------------------

void Measure::removeStaves(int sStaff, int eStaff)
      {
      for (Segment* s = first(); s; s = s->next()) {
            for (int staff = eStaff-1; staff >= sStaff; --staff) {
                  s->removeStaff(staff);
                  }
            }
      foreach (Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int voice = e->voice();
            int staffIdx = e->staffIdx();
            if (staffIdx >= eStaff) {
                  staffIdx -= eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      }

//---------------------------------------------------------
//   insertStaves
//---------------------------------------------------------

void Measure::insertStaves(int sStaff, int eStaff)
      {
      foreach (Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && !e->systemFlag()) {
                  int voice = e->voice();
                  staffIdx += eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      for (Segment* s = first(); s; s = s->next()) {
            for (int staff = sStaff; staff < eStaff; ++staff) {
                  s->insertStaff(staff);
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemoveStaves
//---------------------------------------------------------

void Measure::cmdRemoveStaves(int sStaff, int eStaff)
      {
      int sTrack = sStaff * VOICES;
      int eTrack = eStaff * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int track = eTrack - 1; track >= sTrack; --track) {
                  Element* el = s->element(track);
                  if (el) {
                        el->undoUnlink();
                        _score->undo(new RemoveElement(el));
                        }
                  }
            foreach (Element* e, s->annotations()) {
                  int staffIdx = e->staffIdx();
                  if ((staffIdx >= sStaff) && (staffIdx < eStaff) && !e->systemFlag()) {
                        e->undoUnlink();
                        _score->undo(new RemoveElement(e));
                        }
                  }
            }
      foreach (Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && (staffIdx < eStaff) && !e->systemFlag()) {
                  e->undoUnlink();
                  _score->undo(new RemoveElement(e));
                  }
            }

      _score->undo(new RemoveStaves(this, sStaff, eStaff));

      for (int i = eStaff - 1; i >= sStaff; --i) {
            MStaff* ms = *(staves.begin()+i);
            Text* t = ms->noText();
            if (t) {
//                  t->undoUnlink();
//                  _score->undo(new RemoveElement(t));
                  }
            _score->undo(new RemoveMStaff(this, ms, i));
            }

      // barLine
      // TODO
      }

//---------------------------------------------------------
//   cmdAddStaves
//---------------------------------------------------------

void Measure::cmdAddStaves(int sStaff, int eStaff, bool createRest)
      {
      _score->undo(new InsertStaves(this, sStaff, eStaff));

      Segment* ts = findSegment(Segment::Type::TimeSig, tick());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = _score->staff(i);
            MStaff* ms   = new MStaff;
            ms->lines    = new StaffLines(score());
            ms->lines->setTrack(i * VOICES);
            // ms->lines->setLines(staff->lines());
            ms->lines->setParent(this);
            ms->lines->setVisible(!staff->invisible());
            _score->undo(new InsertMStaff(this, ms, i));
            }

      if (!createRest && !ts)
            return;


      // create list of unique staves (only one instance for linked staves):

      QList<int> sl;
      for (int staffIdx = sStaff; staffIdx < eStaff; ++staffIdx) {
            Staff* s = _score->staff(staffIdx);
            if (s->linkedStaves()) {
                  bool alreadyInList = false;
                  for (int idx : sl) {
                        if (s->linkedStaves()->staves().contains(_score->staff(idx))) {
                              alreadyInList = true;
                              break;
                              }
                        }
                  if (alreadyInList)
                        continue;
                  }
            sl.append(staffIdx);
            }

      for (int staffIdx : sl) {
            if (createRest)
                  score()->setRest(tick(), staffIdx * VOICES, len(), false, 0, _timesig == len());

            // replicate time signature
            if (ts) {
                  TimeSig* ots = 0;
                  bool constructed = false;
                  for (int track = 0; track < staves.size() * VOICES; ++track) {
                        if (ts->element(track)) {
                              ots = static_cast<TimeSig*>(ts->element(track));
                              break;
                              }
                        }
                  if (!ots) {
                        // no time signature found; use measure length to construct one
                        ots = new TimeSig(score());
                        ots->setSig(len());
                        constructed = true;
                        }
                  // do no replicate local time signatures
                  if (ots && !ots->isLocal()) {
                        TimeSig* timesig = new TimeSig(*ots);
                        timesig->setTrack(staffIdx * VOICES);
                        timesig->setParent(ts);
                        timesig->setSig(ots->sig(), ots->timeSigType());
                        timesig->setNeedLayout(true);
                        score()->undoAddElement(timesig);
                        if (constructed)
                              delete ots;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void MStaff::setTrack(int track)
      {
      if (lines)
            lines->setTrack(track);
      if (_vspacerUp)
            _vspacerUp->setTrack(track);
      if (_vspacerDown)
            _vspacerDown->setTrack(track);
      }

//---------------------------------------------------------
//   insertMStaff
//---------------------------------------------------------

void Measure::insertMStaff(MStaff* staff, int idx)
      {
      staves.insert(idx, staff);
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx)
            staves[staffIdx]->setTrack(staffIdx * VOICES);
      }

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff* /*staff*/, int idx)
      {
      staves.removeAt(idx);
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx)
            staves[staffIdx]->setTrack(staffIdx * VOICES);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Measure::insertStaff(Staff* staff, int staffIdx)
      {
      for (Segment* s = first(); s; s = s->next())
            s->insertStaff(staffIdx);

      MStaff* ms = new MStaff;
      ms->lines  = new StaffLines(score());
      ms->lines->setParent(this);
      ms->lines->setTrack(staffIdx * VOICES);
//      ms->distanceUp   = 0.0;
//      ms->distanceDown = 0.0; // TODO point(staffIdx == 0 ? score()->styleS(StyleIdx::minSystemDistance) : score()->styleS(StyleIdx::staffDistance));
      ms->lines->setVisible(!staff->invisible());
      insertMStaff(ms, staffIdx);
      }

//---------------------------------------------------------
//   staffabbox
//---------------------------------------------------------

QRectF Measure::staffabbox(int staffIdx) const
      {
      System* s = system();
      QRectF sb(s->staff(staffIdx)->bbox());
      QRectF rrr(sb.translated(s->pagePos()));
      QRectF rr(abbox());
      QRectF r(rr.x(), rrr.y(), rr.width(), rrr.height());
      return r;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

/**
 Return true if an Element of type \a type can be dropped on a Measure
*/

bool Measure::acceptDrop(const DropData& data) const
      {
      MuseScoreView* viewer = data.view;
      QPointF pos           = data.pos;
      Element* e            = data.element;

      int staffIdx;
      Segment* seg;
      if (_score->pos2measure(pos, &staffIdx, 0, &seg, 0) == nullptr)
            return false;

      QRectF staffR = system()->staff(staffIdx)->bbox().translated(system()->canvasPos());
      staffR &= canvasBoundingRect();

      switch (e->type()) {
            case Element::Type::MEASURE_LIST:
            case Element::Type::JUMP:
            case Element::Type::MARKER:
            case Element::Type::LAYOUT_BREAK:
            case Element::Type::STAFF_LIST:
                  viewer->setDropRectangle(canvasBoundingRect());
                  return true;

            case Element::Type::KEYSIG:
            case Element::Type::TIMESIG:
                  if (data.modifiers & Qt::ControlModifier)
                        viewer->setDropRectangle(staffR);
                  else
                        viewer->setDropRectangle(canvasBoundingRect());
                  return true;

            case Element::Type::BRACKET:
            case Element::Type::REPEAT_MEASURE:
            case Element::Type::MEASURE:
            case Element::Type::SPACER:
            case Element::Type::IMAGE:
            case Element::Type::BAR_LINE:
            case Element::Type::SYMBOL:
            case Element::Type::CLEF:
                  viewer->setDropRectangle(staffR);
                  return true;

            case Element::Type::ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case IconType::VFRAME:
                        case IconType::HFRAME:
                        case IconType::TFRAME:
                        case IconType::FFRAME:
                        case IconType::MEASURE:
                              viewer->setDropRectangle(canvasBoundingRect());
                              return true;
                        default:
                              break;
                        }
                  break;

            default:
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
///   Drop element.
///   Handle a dropped element at position \a pos of given
///   element \a type and \a subtype.
//---------------------------------------------------------

Element* Measure::drop(const DropData& data)
      {
      Element* e = data.element;
      int staffIdx = -1;
      Segment* seg;
      _score->pos2measure(data.pos, &staffIdx, 0, &seg, 0);

      if (e->systemFlag())
            staffIdx = 0;
      if (staffIdx < 0)
            return 0;
#if 0 // yet(?) unused
      QPointF mrp(data.pos - pagePos());
#endif
      Staff* staff = score()->staff(staffIdx);
      bool fromPalette = (e->track() == -1);

      switch(e->type()) {
            case Element::Type::MEASURE_LIST:
qDebug("drop measureList or StaffList");
                  delete e;
                  break;

            case Element::Type::STAFF_LIST:
qDebug("drop staffList");
//TODO                  score()->pasteStaff(e, this, staffIdx);
                  delete e;
                  break;

            case Element::Type::MARKER:
            case Element::Type::JUMP:
                  e->setParent(this);
                  e->setTrack(0);
                  {
                  // code borrowed from ChordRest::drop()
                  Text* t = static_cast<Text*>(e);
                  TextStyleType st = t->textStyleType();
                  // for palette items, we want to use current score text style settings
                  // except where the source element had explicitly overridden these via text properties
                  // palette text style will be relative to baseStyle, so rebase this to score
                  if (st >= TextStyleType::DEFAULT && fromPalette)
                        t->textStyle().restyle(MScore::baseStyle()->textStyle(st), score()->textStyle(st));
                  }
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::DYNAMIC:
            case Element::Type::FRET_DIAGRAM:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::IMAGE:
            case Element::Type::SYMBOL:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  e->layout();
                  {
                  QPointF uo(data.pos - e->canvasPos() - data.dragOffset);
                  e->setUserOff(uo);
                  }
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::BRACKET:
                  {
                  Bracket* b = static_cast<Bracket*>(e);
                  int level = 0;
                  int firstStaff = 0;
                  foreach (Staff* s, score()->staves()) {
                        foreach (const BracketItem& bi, s->brackets()) {
                              int lastStaff = firstStaff + bi._bracketSpan - 1;
                              if (staffIdx >= firstStaff && staffIdx <= lastStaff)
                                    ++level;
                              }
                        firstStaff++;
                        }
                  score()->undoAddBracket(staff, level, b->bracketType(), 1);
                  delete b;
                  }
                  return 0;

            case Element::Type::CLEF:
                  score()->undoChangeClef(staff, first(), static_cast<Clef*>(e)->clefType());
                  delete e;
                  break;

            case Element::Type::KEYSIG:
                  {
                  KeySig* ks    = static_cast<KeySig*>(e);
                  KeySigEvent k = ks->keySigEvent();
                  delete ks;

                  if (data.modifiers & Qt::ControlModifier) {
                        // apply only to this stave
                        score()->undoChangeKeySig(staff, tick(), k);
                        }
                  else {
                        // apply to all staves:
                        foreach(Staff* s, score()->staves())
                              score()->undoChangeKeySig(s, tick(), k);
                        }

                  break;
                  }

            case Element::Type::TIMESIG:
                  score()->cmdAddTimeSig(this, staffIdx, static_cast<TimeSig*>(e),
                     data.modifiers & Qt::ControlModifier);
                  return 0;

            case Element::Type::LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  Measure* measure = isMMRest() ? mmRestLast() : this;
                  if (!measure)
                        return 0;
                  if (
                        (lb->layoutBreakType() == LayoutBreak::Type::PAGE && measure->pageBreak())
                     || (lb->layoutBreakType() == LayoutBreak::Type::LINE && measure->lineBreak())
                     || (lb->layoutBreakType() == LayoutBreak::Type::SECTION && measure->sectionBreak())
                     ) {
                        //
                        // if break already set
                        //
                        delete lb;
                        break;
                        }
                  // make sure there is only LayoutBreak::Type::LINE or LayoutBreak::Type::PAGE
                  if ((lb->layoutBreakType() != LayoutBreak::Type::SECTION) && (measure->pageBreak() || measure->lineBreak())) {
                        foreach(Element* le, measure->el()) {
                              if (le->type() == Element::Type::LAYOUT_BREAK
                                 && (static_cast<LayoutBreak*>(le)->layoutBreakType() == LayoutBreak::Type::LINE
                                  || static_cast<LayoutBreak*>(le)->layoutBreakType() == LayoutBreak::Type::PAGE)) {
                                    score()->undoChangeElement(le, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // these are system elements
                  lb->setParent(measure);
                  score()->undoAddElement(lb);
                  return lb;
                  }

            case Element::Type::SPACER:
                  {
                  Spacer* spacer = static_cast<Spacer*>(e);
                  spacer->setTrack(staffIdx * VOICES);
                  spacer->setParent(this);
                  score()->undoAddElement(spacer);
                  return spacer;
                  }

            case Element::Type::BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);
                  // if dropped bar line refers to span rather than to subtype
                  // or if Ctrl key used
                  if ((bl->spanFrom() != 0 && bl->spanTo() != DEFAULT_BARLINE_TO) || (data.modifiers & Qt::ControlModifier)) {
                        // get existing bar line for this staff, and drop the change to it
                        Segment* seg = undoGetSegment(Segment::Type::EndBarLine, tick() + ticks());
                        BarLine* cbl = static_cast<BarLine*>(seg->element(staffIdx * VOICES));
                        if (cbl)
                              cbl->drop(data);
                        }
                  // if dropped bar line refers to line subtype
                  else {
                        score()->undoChangeBarLine(this, bl->barLineType());
                        delete e;
                        }
                  break;
                  }

            case Element::Type::REPEAT_MEASURE:
                  {
                  delete e;
                  return cmdInsertRepeatMeasure(staffIdx);
                  }
            case Element::Type::ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case IconType::VFRAME:
                              score()->insertMeasure(Element::Type::VBOX, this);
                              break;
                        case IconType::HFRAME:
                              score()->insertMeasure(Element::Type::HBOX, this);
                              break;
                        case IconType::TFRAME:
                              score()->insertMeasure(Element::Type::TBOX, this);
                              break;
                        case IconType::FFRAME:
                              score()->insertMeasure(Element::Type::FBOX, this);
                              break;
                        case IconType::MEASURE:
                              score()->insertMeasure(Element::Type::MEASURE, this);
                              break;
                        default:
                              break;
                        }
                  break;

            default:
                  qDebug("Measure: cannot drop %s here", e->name());
                  delete e;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   cmdInsertRepeatMeasure
//---------------------------------------------------------

RepeatMeasure* Measure::cmdInsertRepeatMeasure(int staffIdx)
      {
      //
      // see also cmdDeleteSelection()
      //
      _score->select(0, SelectType::SINGLE, 0);
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() & Segment::Type::ChordRest) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* el = s->element(track);
                        if (el)
                              _score->undoRemoveElement(el);
                        }
                  }
            }
      //
      // add repeat measure
      //
      Segment* seg = undoGetSegment(Segment::Type::ChordRest, tick());
      RepeatMeasure* rm = new RepeatMeasure(_score);
      rm->setTrack(staffIdx * VOICES);
      rm->setParent(seg);
      rm->setDurationType(TDuration::DurationType::V_MEASURE);
      rm->setDuration(stretchedLen(_score->staff(staffIdx)));
      _score->undoAddCR(rm, this, tick());
      foreach (Element* el, _el) {
            if (el->type() == Element::Type::SLUR && el->staffIdx() == staffIdx)
                  _score->undoRemoveElement(el);
            }
      return rm;
      }

//---------------------------------------------------------
//   adjustToLen
//    change actual measure len, adjust elements to
//    new len
//---------------------------------------------------------

void Measure::adjustToLen(Fraction nf, bool appendRestsIfNecessary)
      {
      int ol   = len().ticks();
      int nl   = nf.ticks();
      int diff = nl - ol;

      int startTick = endTick();
      if (diff < 0)
            startTick += diff;

      score()->undoInsertTime(startTick, diff);
      score()->undo(new InsertTime(score(), startTick, diff));

      for (Score* s : score()->scoreList()) {
            Measure* m = s->tick2measure(tick());
            s->undo(new ChangeMeasureLen(m, nf));
            if (nl > ol) {
                  // move EndBarLine, TimeSigAnnounce, KeySigAnnounce
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->segmentType() & (Segment::Type::EndBarLine|Segment::Type::TimeSigAnnounce|Segment::Type::KeySigAnnounce)) {
                              s->setTick(tick() + nl);
                              }
                        }
                  }
            }
      Score* s      = score()->rootScore();
      Measure* m    = s->tick2measure(tick());
      QList<int> sl = s->uniqueStaves();

      for (int staffIdx : sl) {
            int rests  = 0;
            int chords = 0;
            Rest* rest = 0;
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = segment->element(track);
                        if (e) {
                              if (e->type() == Element::Type::REST) {
                                    ++rests;
                                    rest = static_cast<Rest*>(e);
                                    }
                              else if (e->type() == Element::Type::CHORD)
                                    ++chords;
                              }
                        }
                  }
            Fraction stretch = score()->staff(staffIdx)->timeStretch(tick());
            // if just a single rest
            if (rests == 1 && chords == 0) {
                  // if measure value didn't change, stick to whole measure rest
                  if (_timesig == nf) {
                        rest->undoChangeProperty(P_ID::DURATION, QVariant::fromValue<Fraction>(nf * stretch));
                        rest->undoChangeProperty(P_ID::DURATION_TYPE, QVariant::fromValue<TDuration>(TDuration::DurationType::V_MEASURE));
                        }
                  else {      // if measure value did change, represent with rests actual measure value
#if 0
                        // any reason not to do this instead?
                        s->undoRemoveElement(rest);
                        s->setRest(tick(), staffIdx * VOICES, nf * stretch, false, 0, false);
#else
                        // convert the measure duration in a list of values (no dots for rests)
                        QList<TDuration> durList = toDurationList(nf * stretch, false, 0);

                        // set the existing rest to the first value of the duration list
                        for (ScoreElement* e : rest->linkList()) {
                              e->undoChangeProperty(P_ID::DURATION, QVariant::fromValue<Fraction>(durList[0].fraction()));
                              e->undoChangeProperty(P_ID::DURATION_TYPE, QVariant::fromValue<TDuration>(durList[0]));
                              }

                        // add rests for any other duration list value
                        int tickOffset = tick() + rest->actualTicks();
                        for (int i = 1; i < durList.count(); i++) {
                              Rest* newRest = new Rest(s);
                              newRest->setDurationType(durList.at(i));
                              newRest->setDuration(durList.at(i).fraction());
                              newRest->setTrack(rest->track());
                              score()->undoAddCR(newRest, this, tickOffset);
                              tickOffset += newRest->actualTicks();
                              }
#endif
                        }
                  continue;
                  }

            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;

            for (int trk = strack; trk < etrack; ++trk) {
                  int n = diff;
                  bool rFlag = false;
                  if (n < 0)  {
                        for (Segment* segment = m->last(); segment;) {
                              Segment* pseg = segment->prev();
                              if (segment->segmentType() == Segment::Type::ChordRest) {
                                    for (Element* a : segment->annotations())
                                          if (a->track() == trk)
                                                s->undoRemoveElement(a);
                                    Element* e = segment->element(trk);
                                    if (e && e->isChordRest()) {
                                          ChordRest* cr = static_cast<ChordRest*>(e);
                                          if (cr->durationType() == TDuration::DurationType::V_MEASURE) {
                                                int actualTicks = cr->actualTicks();
                                                n += actualTicks;
                                                cr->setDurationType(TDuration(actualTicks));
                                                }
                                          else
                                                n += cr->actualTicks();
                                          s->undoRemoveElement(e);
                                          if (n >= 0)
                                                break;
                                          }
                                    }
                              else if (segment->segmentType() == Segment::Type::Breath) {
                                    Element* e = segment->element(trk);
                                    if (e)
                                          s->undoRemoveElement(e);
                                    }
                              segment = pseg;
                              }
                        rFlag = true;
                        }
                  int voice = trk % VOICES;
                  if (appendRestsIfNecessary && (n > 0) && (rFlag || voice == 0)) {
                        // add rest to measure
                        int rtick = tick() + nl - n;
                        int track = staffIdx * VOICES + voice;
                        s->setRest(rtick, track, Fraction::fromTicks(n) * stretch, false, 0, false);
                        }
                  }
            }
      if (diff < 0) {
            //
            //  CHECK: do not remove all slurs
            //
            foreach(Element* e, m->el()) {
                  if (e->type() == Element::Type::SLUR)
                        s->undoRemoveElement(e);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(Xml& xml, int staff, bool writeSystemElements, bool forceTimeSig) const
      {
      int mno = _no + 1;
      if (_len != _timesig) {
            // this is an irregular measure
            xml.stag(QString("Measure number=\"%1\" len=\"%2/%3\"").arg(mno).arg(_len.numerator()).arg(_len.denominator()));
            }
      else
            xml.stag(QString("Measure number=\"%1\"").arg(mno));

      xml.curTick = tick();

      if (_mmRestCount > 0)
            xml.tag("multiMeasureRest", _mmRestCount);
      if (writeSystemElements) {
            if (_repeatFlags & Repeat::START)
                  xml.tagE("startRepeat");
            if (_repeatFlags & Repeat::END)
                  xml.tag("endRepeat", _repeatCount);
            if (_irregular)
                  xml.tagE("irregular");
            if (_breakMultiMeasureRest)
                  xml.tagE("breakMultiMeasureRest");
            writeProperty(xml, P_ID::USER_STRETCH);
            writeProperty(xml, P_ID::NO_OFFSET);
            writeProperty(xml, P_ID::MEASURE_NUMBER_MODE);
            writeProperty(xml, P_ID::SYSTEM_INITIAL_BARLINE_TYPE);
            }
      qreal _spatium = spatium();
      MStaff* mstaff = staves[staff];
      if (mstaff->noText() && !mstaff->noText()->generated()) {
            xml.stag("MeasureNumber");
            mstaff->noText()->writeProperties(xml);
            xml.etag();
            }

      if (mstaff->_vspacerUp)
            xml.tag("vspacerUp", mstaff->_vspacerUp->gap() / _spatium);
      if (mstaff->_vspacerDown)
            xml.tag("vspacerDown", mstaff->_vspacerDown->gap() / _spatium);
      if (!mstaff->_visible)
            xml.tag("visible", mstaff->_visible);
      if (mstaff->_slashStyle)
            xml.tag("slashStyle", mstaff->_slashStyle);

      int strack = staff * VOICES;
      int etrack = strack + VOICES;
      foreach (const Element* el, _el) {
            if (!el->generated() && ((el->staffIdx() == staff) || (el->systemFlag() && writeSystemElements))) {
                  el->write(xml);
                  }
            }
      Q_ASSERT(first());
      Q_ASSERT(last());
      score()->writeSegments(xml, strack, etrack, first(), last()->next1(), writeSystemElements, false, false, forceTimeSig);
      xml.etag();
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int Measure::ticks() const
      {
      return _len.ticks();
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

void Measure::read(XmlReader& e, int staffIdx)
      {
      Segment* segment = 0;
      qreal _spatium = spatium();

      QList<Chord*> graceNotes;

      //sort tuplet elements. needed for nested tuplets #22537
      if (score()->mscVersion() <= 114) {
            for (Tuplet* t : e.tuplets()) {
                  t->sortElements();
                  }
            }
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      for (int n = staves.size(); n <= staffIdx; ++n) {
            Staff* staff = score()->staff(n);
            MStaff* s    = new MStaff;
            s->lines     = new StaffLines(score());
            s->lines->setParent(this);
            s->lines->setTrack(n * VOICES);
            s->lines->setVisible(!staff->invisible());
            staves.append(s);
            }

      // tick is obsolete
      if (e.hasAttribute("tick"))
            e.initTick(score()->fileDivision(e.intAttribute("tick")));

      bool irregular;
      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  _len = Fraction(sl[0].toInt(), sl[1].toInt());
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            irregular = true;
            score()->sigmap()->add(tick(), SigEvent(_len, _timesig));
            score()->sigmap()->add(tick() + ticks(), SigEvent(_timesig));
            }
      else
            irregular = false;

      Staff* staff = score()->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(tick()));

      // keep track of tick of previous element
      // this allows markings that need to apply to previous element to do so
      // even though we may have already advanced to next tick position
      int lastTick = e.tick();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "tick") {
                  e.initTick(score()->fileDivision(e.readInt()));
                  lastTick = e.tick();
                  }
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(e.track());
                  barLine->read(e);
                  Segment::Type st;

                  //
                  //  SegStartRepeatBarLine: always at the beginning tick of a measure
                  //  SegBarLine:            in the middle of a measure, has no semantic
                  //  SegEndBarLine:         at the end tick of a measure

                  if ((e.tick() != tick()) && (e.tick() != endTick()))
                        st = Segment::Type::BarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && e.tick() == tick())
                        st = Segment::Type::StartRepeatBarLine;
                  else
                        st = Segment::Type::EndBarLine;

                  segment = getSegment(st, e.tick()); // let the bar line know it belongs to a Segment,
                  segment->add(barLine);              // before setting its flags
                  if (st == Segment::Type::EndBarLine) {
                        if (!barLine->customSubtype()) {
                              BarLineType blt = barLine->barLineType();
                              // Measure::_endBarLineGenerated is true if the bar line is of a type which can
                              // be reconstructed from measure flags
                              bool endBarLineGenerated = (blt == BarLineType::NORMAL || blt == BarLineType::END_REPEAT
                                    || blt == BarLineType::END_START_REPEAT || blt == BarLineType::START_REPEAT);
                              setEndBarLineType(blt, barLine->el()->empty() && endBarLineGenerated, true);
                              }
                        if (!barLine->customSpan()) {
                              Staff* staff = score()->staff(staffIdx);
                              barLine->setSpan(staff->barLineSpan());
                              barLine->setSpanFrom(staff->barLineFrom());
                              barLine->setSpanTo(staff->barLineTo());
                              }
                        }
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(e.track());
                  chord->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  if (chord->noteType() != NoteType::NORMAL) {
                        graceNotes.push_back(chord);
                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TremoloType::R8) {
                              // old style tremolo found
                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case TremoloType::OLD_R8:  st = TremoloType::R8;  break;
                                    case TremoloType::OLD_R16: st = TremoloType::R16; break;
                                    case TremoloType::OLD_R32: st = TremoloType::R32; break;
                                    case TremoloType::OLD_C8:  st = TremoloType::C8;  break;
                                    case TremoloType::OLD_C16: st = TremoloType::C16; break;
                                    case TremoloType::OLD_C32: st = TremoloType::C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              }
                        }
                  else {
                        segment->add(chord);
                        Q_ASSERT(segment->segmentType() == Segment::Type::ChordRest);

                        for (int i = 0; i < graceNotes.size(); ++i) {
                              Chord* gc = graceNotes[i];
                              gc->setGraceIndex(i);
                              chord->add(gc);
                              }
                        graceNotes.clear();
                        int crticks = chord->actualTicks();

                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TremoloType::R8) {
                              // old style tremolo found

                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case TremoloType::OLD_R8:  st = TremoloType::R8;  break;
                                    case TremoloType::OLD_R16: st = TremoloType::R16; break;
                                    case TremoloType::OLD_R32: st = TremoloType::R32; break;
                                    case TremoloType::OLD_C8:  st = TremoloType::C8;  break;
                                    case TremoloType::OLD_C16: st = TremoloType::C16; break;
                                    case TremoloType::OLD_C32: st = TremoloType::C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              if (tremolo->twoNotes()) {
                                    int track = chord->track();
                                    Segment* ss = 0;
                                    for (Segment* ps = first(Segment::Type::ChordRest); ps; ps = ps->next(Segment::Type::ChordRest)) {
                                          if (ps->tick() >= e.tick())
                                                break;
                                          if (ps->element(track))
                                                ss = ps;
                                          }
                                    Chord* pch = 0;       // previous chord
                                    if (ss) {
                                          ChordRest* cr = static_cast<ChordRest*>(ss->element(track));
                                          if (cr && cr->type() == Element::Type::CHORD)
                                                pch = static_cast<Chord*>(cr);
                                          }
                                    if (pch) {
                                          tremolo->setParent(pch);
                                          pch->setTremolo(tremolo);
                                          chord->setTremolo(0);
                                          // force duration to half
                                          Fraction pts(timeStretch * pch->globalDuration());
                                          int pcrticks = pts.ticks();
                                          pch->setDuration(Fraction::fromTicks(pcrticks / 2));
                                          chord->setDuration(Fraction::fromTicks(crticks / 2));
                                          }
                                    else {
                                          qDebug("tremolo: first note not found");
                                          }
                                    crticks /= 2;
                                    }
                              else {
                                    tremolo->setParent(chord);
                                    }
                              }
                        lastTick = e.tick();
                        e.incTick(crticks);
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(timesig()/timeStretch);
                  rest->setTrack(e.track());
                  rest->read(e);
                  segment = getSegment(rest, e.tick());
                  segment->add(rest);

                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(timesig()/timeStretch);

                  lastTick = e.tick();
                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTrack(e.track());
                  int tick = e.tick();
                  breath->read(e);
                  if (score()->mscVersion() < 205) {
                        // older scores placed the breath segment right after the chord to which it applies
                        // rather than before the next chordrest segment with an element for the staff
                        // result would be layout too far left if there are other segments due to notes in other staves
                        // we need to find tick of chord to which this applies, and add its duration
                        int prevTick;
                        if (e.tick() < tick)
                              prevTick = e.tick();    // use our own tick if we explicitly reset to earlier position
                        else
                              prevTick = lastTick;    // otherwise use tick of previous tick/chord/rest tag
                        // find segment
                        Segment* prev = findSegment(Segment::Type::ChordRest, prevTick);
                        if (prev) {
                              // find chordrest
                              ChordRest* lastCR = static_cast<ChordRest*>(prev->element(e.track()));
                              if (lastCR)
                                    tick = prevTick + lastCR->actualTicks();
                              }
                        }
                  segment = getSegment(Segment::Type::Breath, tick);
                  segment->add(breath);
                  }
            else if (tag == "endSpanner") {
                  int id = e.attribute("id").toInt();
                  Spanner* spanner = e.findSpanner(id);
                  if (spanner) {
                        spanner->setTicks(e.tick() - spanner->tick());
                        // if (spanner->track2() == -1)
                              // the absence of a track tag [?] means the
                              // track is the same as the beginning of the slur
                        if (spanner->track2() == -1)
                              spanner->setTrack2(spanner->track() ? spanner->track() : e.track());
                        }
                  else {
                        // remember "endSpanner" values
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = e.track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  e.readNext();
                  }
            else if (tag == "Slur") {
                  Slur *sl = new Slur(score());
                  sl->setTick(e.tick());
                  sl->read(e);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sl);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sl->setTick2(sv->tick2);
                        sl->setTrack2(sv->track2);
                        }
                  score()->addSpanner(sl);
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  // ?? sp->setAnchor(Spanner::Anchor::SEGMENT);
                  sp->read(e);
                  score()->addSpanner(sp);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sp);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sp->setTicks(sv->tick2 - sp->tick());
                        sp->setTrack2(sv->track2);
                        }
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTrack(e.track());
                  rm->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(rm);
                  if (rm->actualDuration().isZero()) { // might happen with 1.3 scores
                        rm->setDuration(len());
                        }
                  lastTick = e.tick();
                  e.incTick(ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);
                  // in some 1.3 scores, clefs can be in score but not in cleflist
                  // if (score()->mscVersion() > 114)
                  //      staff->setClef(e.tick(), clef->clefTypeList());

                  if (segment)
                        // not the first segment, so this is not a header clef
                        clef->setSmall(true);
                  segment = 0;
                  Segment* ns = 0;  // the place to insert a new segment
                  Segment* s = 0;   // the segment to try to reuse, if possible
                  if (clef->small()) {
                        // mid-measure clefs are inserted right before the ChordRest
                        ns = findSegment(Segment::Type::ChordRest, e.tick());
                        // there might already be a segment for mid-measure clefs
                        s = ns ? ns->prev() : 0;
                        }
                  else {
                        // header clefs are inserted at the front of the segments list
                        ns = first();
                        // there might already be a segment for header clefs
                        s = ns;
                        }
                  if (s && s->segmentType() == Segment::Type::Clef) {
                        // check for a clef in each staff
                        // stop when a clef is found
                        // if it is the same size as the new clef, then use this segment
                        // otherwise, create a new segment for the clef
                        Clef* c = 0;
                        for (int t = 0; !c && t < score()->ntracks(); t += VOICES)
                              c = static_cast<Clef*>(s->element(t));
                        if (c && c->small() == clef->small())
                              // use this segment for the clef
                              segment = s;
                        }
                  if (!segment) {
                        // create a new segment for the clef
                        segment = new Segment(this, Segment::Type::Clef, e.tick());
                        _segments.insert(segment, ns);
                        }
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTrack(e.track());
                  ts->read(e);
                  // if time sig not at begining of measure => courtesy time sig
                  int currTick = e.tick();
                  bool courtesySig = (currTick > tick());
                  if (courtesySig) {
                        // if courtesy sig., just add it without map processing
                        segment = getSegment(Segment::Type::TimeSigAnnounce, currTick);
                        segment->add(ts);
                        }
                  else {
                        // if 'real' time sig., do full process
                        segment = getSegment(Segment::Type::TimeSig, currTick);
                        segment->add(ts);

                        timeStretch = ts->stretch().reduced();
                        _timesig    = ts->sig() / timeStretch;

                        if (score()->mscVersion() > 114) {
                              if (irregular) {
                                    score()->sigmap()->add(tick(), SigEvent(_len, _timesig));
                                    score()->sigmap()->add(tick() + ticks(), SigEvent(_timesig));
                                    }
                              else {
                                    _len = _timesig;
                                    score()->sigmap()->add(tick(), SigEvent(_timesig));
                                    }
                              }
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTrack(e.track());
                  ks->read(e);
                  int curTick = e.tick();
                  if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick == 0) {
                        // ignore empty key signature
                        qDebug("remove keysig c at tick 0");
                        delete ks;
                        }
                  else {
                        // if key sig not at beginning of measure => courtesy key sig
                        bool courtesySig = (curTick > tick());
                        segment = getSegment(courtesySig ? Segment::Type::KeySigAnnounce : Segment::Type::KeySig, curTick);
                        segment->add(ks);
                        if (!courtesySig)
                              staff->setKey(curTick, ks->keySigEvent());
                        }
                  }
            else if (tag == "Lyrics") {       // obsolete, keep for compatibility with version 114
                  Element* element = Element::name2Element(tag, score());
                  element->setTrack(e.track());
                  element->read(e);
                  segment       = getSegment(Segment::Type::ChordRest, e.tick());
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(element->track()));
                  if (!cr)
                        cr = static_cast<ChordRest*>(segment->element(e.track())); // in case lyric itself has bad track info
                  if (!cr)
                        qDebug("Internal error: no chord/rest for lyrics");
                  else
                        cr->add(element);
                  }
            else if (tag == "Text") {
                  Text* t = new StaffText(score());
                  t->setTrack(e.track());
                  t->read(e);
                  // previous versions stored measure number, delete it
                  if ((score()->mscVersion() <= 114) && (t->textStyleType() == TextStyleType::MEASURE_NUMBER))
                        delete t;
                  else if (t->isEmpty()) {
                        qDebug("reading empty text: deleted");
                        delete t;
                        }
                  else if ((score()->mscVersion() <= 114) && t->textStyleType() == TextStyleType::REHEARSAL_MARK) {
                        RehearsalMark* rh = new RehearsalMark(score());
                        rh->setXmlText(t->xmlText());
                        rh->setTrack(t->track());
                        segment = getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(rh);
                        delete t;
                        }
                  else {
                        segment = getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  if (score()->mscVersion() <= 114)
                        dyn->setDynamicType(dyn->xmlText());
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "RehearsalMark"
               || tag == "InstrumentChange"
               || tag == "StaffState"
               || tag == "FiguredBass"
               ) {
                  Element* el = Element::name2Element(tag, score());
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  if (score()->mscVersion() <= 114 && el->type() == Element::Type::SYMBOL)
                        el->setParent(this);    // this will get reset when adding to segment
                  el->setTrack(e.track());
                  el->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(el);
                  }
            else if (tag == "Marker"
               || tag == "Jump"
               ) {
                  Element* el = Element::name2Element(tag, score());
                  el->setTrack(e.track());
                  el->read(e);
                  add(el);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Element* el = Element::name2Element(tag, score());
                        el->setTrack(e.track());
                        el->read(e);
                        segment = getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(el);
                        }
                  }
            //----------------------------------------------------
            else if (tag == "stretch") {
                  double val = e.readDouble();
                  if (val < 0.0)
                        val = 0;
                  setUserStretch(val);
                  }
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "noOffset")
                  _noOffset = e.readInt();
            else if (tag == "measureNumberMode")
                  setMeasureNumberMode(MeasureNumberMode(e.readInt()));
            else if (tag == "irregular")
                  _irregular = e.readBool();
            else if (tag == "breakMultiMeasureRest")
                  _breakMultiMeasureRest = e.readBool();
            else if (tag == "sysInitBarLineType") {
                  _systemInitialBarLineType = BarLineType(Ms::getProperty(P_ID::SYSTEM_INITIAL_BARLINE_TYPE, e).toInt());
                  }
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->setTrack(e.track());
                  tuplet->setTick(e.tick());
                  tuplet->setParent(this);
                  tuplet->read(e);
                  e.addTuplet(tuplet);
                  }
            else if (tag == "startRepeat") {
                  _repeatFlags = _repeatFlags | Repeat::START;
                  e.readNext();
                  }
            else if (tag == "endRepeat") {
                  _repeatCount = e.readInt();
                  _repeatFlags = _repeatFlags | Repeat::END;
                  }
            else if (tag == "vspacer" || tag == "vspacerDown") {
                  if (staves[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerDown->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (staves[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::UP);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerUp->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  staves[staffIdx]->_visible = e.readInt();
            else if (tag == "slashStyle")
                  staves[staffIdx]->_slashStyle = e.readInt();
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score());
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  e.addBeam(beam);
                  }
            else if (tag == "Segment")
                  segment->read(e);
            else if (tag == "MeasureNumber") {
                  Text* noText = new Text(score());
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
                  // noText->setFlag(ElementFlag::MOVABLE, false); ??
                  noText->setTrack(e.track());
                  noText->setParent(this);
                  staves[noText->staffIdx()]->setNoText(noText);
                  }
            else if (tag == "Ambitus") {
                  Ambitus* range = new Ambitus(score());
                  range->read(e);
                  segment = getSegment(Segment::Type::Ambitus, e.tick());
                  range->setParent(segment);          // a parent segment is needed for setTrack() to work
                  range->setTrack(trackZeroVoice(e.track()));
                  segment->add(range);
                  }
            else if (tag == "multiMeasureRest") {
                  _mmRestCount = e.readInt();
                  // set tick to previous measure
                  setTick(e.lastMeasure()->tick());
                  e.initTick(e.lastMeasure()->tick());
                  }
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      if (staffIdx == 0) {
            Segment* s = last();
            if (s && s->segmentType() == Segment::Type::BarLine) {
                  BarLine* b = static_cast<BarLine*>(s->element(0));
                  setEndBarLineType(b->barLineType(), false, b->visible(), b->color());
                  // s->remove(b);
                  // delete b;
                  }
            }
      //
      // for compatibility with 1.22:
      //
      if (score()->mscVersion() == 122) {
            int ticks1 = 0;
            for (Segment* s = last(); s; s = s->prev()) {
                  if (s->segmentType() == Segment::Type::ChordRest) {
                        if (s->element(0)) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(0));
                              if (cr->type() == Element::Type::REPEAT_MEASURE)
                                    ticks1 = ticks();
                              else
                                    ticks1 = s->rtick() + cr->actualTicks();
                              break;
                              }
                        }
                  }
            if (ticks() != ticks1) {
                  // this is a irregular measure
                  _len = Fraction::fromTicks(ticks1);
                  _len.reduce();
                  for (Segment* s = last(); s; s = s->prev()) {
                        if (s->tick() < tick() + ticks())
                              break;
                        if (s->segmentType() == Segment::Type::BarLine) {
                              qDebug("reduce BarLine to EndBarLine");
                              s->setSegmentType(Segment::Type::EndBarLine);
                              }
                        }

                  }
            }
      e.checkTuplets();
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool Measure::visible(int staffIdx) const
      {
      if (system() && (system()->staves()->isEmpty() || !system()->staff(staffIdx)->show()))
            return false;
      if (staffIdx >= score()->staves().size()) {
            qDebug("Measure::visible: bad staffIdx: %d", staffIdx);
            return false;
            }
      return score()->staff(staffIdx)->show() && staves[staffIdx]->_visible;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Measure::slashStyle(int staffIdx) const
      {
      return score()->staff(staffIdx)->slashStyle() || staves[staffIdx]->_slashStyle || score()->staff(staffIdx)->staffType()->slashStyle();
      }

//---------------------------------------------------------
//   isFinalMeasureOfSection
//    returns true if this measure is final actual measure of a section
//    takes into consideration fact that subsequent measures base objects may have section break before encountering next actual measure
//---------------------------------------------------------

bool Measure::isFinalMeasureOfSection() const
      {
      const MeasureBase* mb = static_cast<const MeasureBase*>(this);

      do {
            if (mb->sectionBreak())
                  return true;

            mb = mb->next();
            } while (mb && !mb->isMeasure());   // loop until reach next actual measure or end of score

      return false;
      }

//---------------------------------------------------------
//   isAnacrusis
//---------------------------------------------------------

bool Measure::isAnacrusis() const
      {
      TimeSigFrac timeSig = score()->sigmap()->timesig(tick()).nominal();
      return irregular() && ticks() < timeSig.ticksPerMeasure();
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Measure::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      MeasureBase::scanElements(data, func, all);

      int nstaves = score()->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!all && !(visible(staffIdx) && score()->staff(staffIdx)->show()))
                  continue;
            MStaff* ms = staves[staffIdx];
            if (ms->lines)
                  func(data, ms->lines);
            if (ms->_vspacerUp)
                  func(data, ms->_vspacerUp);
            if (ms->_vspacerDown)
                  func(data, ms->_vspacerDown);
            if (ms->noText())
                  func(data, ms->noText());
            }

      for (Segment* s = first(); s; s = s->next())
            s->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   createVoice
//    Create a voice on demand by filling the measure
//    with a whole measure rest.
//    Check if there are any chord/rests in track; if
//    not create a whole measure rest
//---------------------------------------------------------

void Measure::createVoice(int track)
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            if (s->element(track) == 0)
                  score()->setRest(s->tick(), track, len(), true, 0);
            break;
            }
      }

//---------------------------------------------------------
//   setStartRepeatBarLine
//    return true if bar line type changed
//---------------------------------------------------------

bool Measure::setStartRepeatBarLine(bool val)
      {
      bool changed    = false;
      Segment* s      = findSegment(Segment::Type::StartRepeatBarLine, tick());
      bool customSpan = false;
      int numStaves   = score()->nstaves();

      for (int staffIdx = 0; staffIdx < numStaves;) {
            int track    = staffIdx * VOICES;
            Staff* staff = score()->staff(staffIdx);
            BarLine* bl  = s ? static_cast<BarLine*>(s->element(track)) : nullptr;
            int span, spanFrom, spanTo;
            // if there is a bar line and has custom span, take span from it
            if (bl && bl->customSpan()) {
                  span        = bl->span();
                  spanFrom    = bl->spanFrom();
                  spanTo      = bl->spanTo();
                  customSpan  = bl->customSpan();
                  }
            else {
                  span        = staff->barLineSpan();
                  spanFrom    = staff->barLineFrom();
                  spanTo      = staff->barLineTo();
                  if (span == 0 && customSpan) {
                        // spanned staves have already been skipped by the loop at the end;
                        // if a staff with span 0 is found and the previous bar line had custom span
                        // this staff shall have an aditional bar line, because the previous staff bar
                        // line has been shortened
                        int staffLines = staff->lines();
                        span     = 1;
                        spanFrom = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0;
                        spanTo   = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (staffLines-1) * 2;
                        }
                  customSpan = false;
                  }
            // make sure we do not span more staves than actually exist
            if (staffIdx + span > numStaves)
                  span = numStaves - staffIdx;

            if (span && val && (bl == 0)) {
                  // no barline were we need one:
                  if (s == 0) {
                        if (score()->undoRedo()) {
                              return false;
                              }
                        s = undoGetSegment(Segment::Type::StartRepeatBarLine, tick());
                        }
                  bl = new BarLine(score());
                  bl->setTrack(track);
                  bl->setParent(s);             // let the bar line know it belongs to a StartRepeatBarLine segment
                  bl->setBarLineType(BarLineType::START_REPEAT);
                  score()->undoAddElement(bl);
                  changed = true;
                  }
            else if (bl && !val) {
                  // barline were we do not need one:
                  if (!score()->undoRedo())                       // DEBUG
                        score()->undoRemoveElement(bl);
                  changed = true;
                  }
            if (bl && val && span) {
                  bl->setSpan(span);
                  bl->setSpanFrom(spanFrom);
                  bl->setSpanTo(spanTo);
                  }

            ++staffIdx;
            //
            // remove any unwanted barlines:
            //
            // if spanning several staves but not entering INTO last staff,
            if (span > 1 && spanTo <= 0)
                  span--;                 // count one span less
            if (s) {
                  for (int i = 1; i < span; ++i) {
                        BarLine* bl  = static_cast<BarLine*>(s->element(staffIdx * VOICES));
                        if (bl) {
                              score()->undoRemoveElement(bl);
                              changed = true;
                              }
                        ++staffIdx;
                        }
                  }
            }
      return changed;
      }

//---------------------------------------------------------
//   createEndBarLines
//    actually creates or modifies barlines
//    returns true if layout changes
//---------------------------------------------------------

bool Measure::createEndBarLines()
      {
      bool changed = false;
      int nstaves  = score()->nstaves();
      Segment* seg = undoGetSegment(Segment::Type::EndBarLine, endTick());

      BarLine* bl = 0;
      int span    = 0;        // span counter
      int aspan   = 0;        // actual span
      bool mensur = false;    // keep note of Mensurstrich case
      int spanTot = 0;           // to keep track of the target span as we count down
      int spanFrom = 0;
      int spanTo = 0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* staff = score()->staff(staffIdx);
            int track    = staffIdx * VOICES;

            // get existing bar line for this staff, if any
            BarLine* cbl = static_cast<BarLine*>(seg->element(track));

            // if span counter has been counted off, get new span values
            // and forget about any previous bar line

            if (span == 0) {
                  if (cbl && cbl->customSpan()) {      // if there is a bar line and has custom span,
                        span        = cbl->span();    // get span values from it
                        spanFrom    = cbl->spanFrom();
                        spanTo      = cbl->spanTo();
                        // if bar span values == staff span values, set bar as not custom
                        if(span == staff->barLineSpan() && spanFrom == staff->barLineFrom()
                           && spanTo == staff->barLineTo())
                              cbl->setCustomSpan(false);
                        }
                  else {                              // otherwise, get from staff
                        span        = staff->barLineSpan();
                        // if some span OR last staff (span=0) of a Mensurstrich case, get From/To from staff
                        if (span || mensur) {
                              spanFrom    = staff->barLineFrom();
                              spanTo      = staff->barLineTo();
                              mensur      = false;
                              }
                        // but if staff is set to no span, a multi-staff spanning bar line
                        // has been shortened to span less staves and following staves left without bars;
                        // set bar line span values to default
                        else {
                              span        = 1;
                              spanFrom    = 0;
                              spanTo      = (staff->lines()-1)*2;
                              }
                        }
                  if ((staffIdx + span) > nstaves)
                        span = nstaves - staffIdx;
                  spanTot     = span;
                  bl = 0;
                  }
            if (staff->show() && span) {
                  //
                  // there should be a barline in this staff
                  //
                  // if we already have a bar line, keep extending this bar line down until span exhausted;
                  // if no barline yet, re-use the bar line existing in this staff if any,
                  // restarting actual span
                  if (!bl) {
                        bl = cbl;
                        aspan = 0;
                        }
                  if (!bl) {
                        // no suitable bar line: create a new one
                        bl = new BarLine(score());
                        bl->setVisible(_endBarLineVisible);
                        bl->setColor(_endBarLineColor);
                        bl->setParent(seg);           // let the bar line know the segment and track it belongs to
                        bl->setTrack(track);
                        bl->setBarLineType(_endBarLineType);
                        bl->setGenerated(_endBarLineGenerated);
                        score()->undoAddElement(bl);
                        changed = true;
                        }
                  else {
                        // a bar line is there (either existing or newly created):
                        // adjust subtype, if not fitting
                        if (bl->barLineType() != _endBarLineType && !bl->customSubtype()) {
                              score()->undoChangeProperty(bl, P_ID::SUBTYPE, int(_endBarLineType));
                              bl->setGenerated(bl->el()->empty() && _endBarLineGenerated);
                              changed = true;
                              }
                        // or clear custom subtype flag if same type as measure
                        if (bl->barLineType() == _endBarLineType && bl->customSubtype()) {
                              bl->setCustomSubtype(false);
                              bl->setGenerated(bl->el()->empty() && _endBarLineGenerated);
                              }

                        // if a bar line exists for this staff (cbl) but
                        // it is not the bar line we are dealing with (bl),
                        // we are extending down the bar line of a staff above (bl)
                        // and the bar line for this staff (cbl) is not needed:
                        // DELETE it
                        if (cbl && cbl != bl) {
                              // Mensurstrich special case:
                              // if span arrives inside the end staff (spanTo>0) OR
                              //          span is not multi-staff (spanTot<=1) OR
                              //          current staff is not the last spanned staff (span!=1) OR
                              //          staff is the last score staff
                              //    remove bar line for this staff
                              // If NONE of the above conditions holds, the staff is the last staff of
                              // a Mensurstrich(-like) span: keep its bar line, as it may span to next staff
                              if (spanTo > 0 || spanTot <= 1 || span != 1 || staffIdx == nstaves-1) {
                                    score()->undoRemoveElement(cbl);
                                    changed = true;
                                    }
                              }
                        }
                  }
            else {
                  //
                  // there should be no barline in this staff
                  //
                  if (cbl) {
                        score()->undoRemoveElement(cbl);
                        changed = true;
                        }
                  }
            // if span not counted off AND we have a bar line AND this staff is shown,
            // set bar line span values (this may result in extending down a bar line
            // for a previous staff, if we are counting off a span > 1)
            if (span) {
                  if (bl) {
                        ++aspan;
                        if (staff->show()) {          // update only if visible
                              bl->setSpan(aspan);
                              bl->setSpanFrom(spanFrom);
                              // if current actual span < target span, set spanTo to full staff height
                              if(aspan < spanTot)
                                    bl->setSpanTo((staff->lines()-1)*2);
                              // if we reached target span, set spanTo to intended value
                              else
                                    bl->setSpanTo(spanTo);
                              }
                        }
                  --span;
                  }
            // if just finished (span==0) a multi-staff span (spanTot>1) ending at the top of a staff (spanTo<=0)
            // scan this staff again, as it may have its own bar lines (Mensurstrich(-like) span)
            if (spanTot > 1 && spanTo <= 0 && span == 0) {
                  mensur = true;
                  staffIdx--;
                  }
            }
      return changed;
      }

//---------------------------------------------------------
//   setEndBarLineType
//---------------------------------------------------------

void Measure::setEndBarLineType(BarLineType val, bool g, bool visible, QColor color)
      {
      _endBarLineType      = val;
      _endBarLineGenerated = g;
      _endBarLineVisible   = visible;
      if (color.isValid())
            _endBarLineColor = color;
      else
            _endBarLineColor = curColor();
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(QList<int>& dst)
      {
      QList<MStaff*> ms;
      foreach (int idx, dst)
            ms.push_back(staves[idx]);
      staves = ms;

      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            if (staves[staffIdx]->lines)
                  staves[staffIdx]->lines->setTrack(staffIdx * VOICES);
            }
      for (Segment* s = first(); s; s = s->next())
            s->sortStaves(dst);

      foreach (Element* e, _el) {
            if (e->track() == -1 || e->systemFlag())
                  continue;
            int voice    = e->voice();
            int staffIdx = e->staffIdx();
            int idx = dst.indexOf(staffIdx);
            e->setTrack(idx * VOICES + voice);
            }
      }

//---------------------------------------------------------
//   exchangeVoice
//---------------------------------------------------------

void Measure::exchangeVoice(int v1, int v2, int staffIdx)
      {
      int strack = staffIdx * VOICES + v1;
      int dtrack = staffIdx * VOICES + v2;

      for (Segment* s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            s->swapElements(strack, dtrack);
            }

      auto spanners = score()->spannerMap().findOverlapping(tick(), endTick()-1);
      int start = tick();
      int end = start + ticks();
      for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp = i->value;
            int spStart = sp->tick();
            int spEnd = spStart + sp->ticks();
            qDebug("Start %d End %d Diff %d \n Measure Start %d End %d", spStart, spEnd, spEnd-spStart, start, end);
            if (sp->type() == Element::Type::SLUR && (spStart >= start || spEnd < end)) {
                  if (sp->track() == strack && spStart >= start){
                        sp->setTrack(dtrack);
                        }
                  else if (sp->track() == dtrack && spStart >= start){
                        sp->setTrack(strack);
                        }
                  if (sp->track2() == strack && spEnd < end){
                        sp->setTrack2(dtrack);
                        }
                  else if (sp->track2() == dtrack && spEnd < end){
                        sp->setTrack2(strack);
                        }
                  }
            }
      // MStaff* ms = mstaff(staffIdx);
      // ms->hasVoices = true;
      checkMultiVoices(staffIdx);   // probably true, but check for invisible notes & rests
      }

//---------------------------------------------------------
//   checkMultiVoices
///   Check for more than on voice in this measure and staff and
///   set MStaff->hasVoices
//---------------------------------------------------------

void Measure::checkMultiVoices(int staffIdx)
      {
      int strack = staffIdx * VOICES + 1;
      int etrack = staffIdx * VOICES + VOICES;
      staves[staffIdx]->hasVoices = false;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e) {
                        bool v;
                        if (e->type() == Element::Type::CHORD) {
                              v = false;
                              // consider chord visible if any note is visible
                              Chord* c = static_cast<Chord*>(e);
                              for (Note* n : c->notes()) {
                                    if (n->visible()) {
                                          v = true;
                                          break;
                                          }
                                    }
                              }
                        else
                              v = e->visible();
                        if (v) {
                              staves[staffIdx]->hasVoices = true;
                              return;
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hasVoice
//---------------------------------------------------------

bool Measure::hasVoice(int track) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            if (s->element(track))
                  return true;
            }
      return false;
      }

//-------------------------------------------------------------------
//   isMeasureRest
///   Check if the measure is filled by a full-measure rest or full
///   of rests on this staff. If staff is -1, then check for
///   all staves.
//-------------------------------------------------------------------

bool Measure::isMeasureRest(int staffIdx)
      {
      int strack;
      int etrack;
      if (staffIdx < 0) {
            strack = 0;
            etrack = score()->nstaves() * VOICES;
            }
      else {
            strack = staffIdx * VOICES;
            etrack = strack + VOICES;
            }
      for (Segment* s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->type() != Element::Type::REST)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isFullMeasureRest
//    Check for an empty measure, filled with full measure
//    rests.
//---------------------------------------------------------

bool Measure::isFullMeasureRest()
      {
      int strack = 0;
      int etrack = score()->nstaves() * VOICES;

      Segment* s = first(Segment::Type::ChordRest);
      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e) {
                  if (e->type() != Element::Type::REST)
                        return false;
                  Rest* rest = static_cast<Rest*>(e);
                  if (rest->durationType().type() != TDuration::DurationType::V_MEASURE)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isRepeatMeasure
//---------------------------------------------------------

bool Measure::isRepeatMeasure(Staff* staff)
      {
      int staffIdx = score()->staffIdx(staff);
      int strack        = staffIdx * VOICES;
      int etrack        = (staffIdx + 1) * VOICES;
      Segment* s        = first(Segment::Type::ChordRest);

      if (s == 0)
            return false;

      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e && e->type() == Element::Type::REPEAT_MEASURE)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   distanceDown
//---------------------------------------------------------

qreal Measure::distanceDown(int i) const
      {
      if (staves[i]->_vspacerDown)
            return qMax(staves[i]->distanceDown, staves[i]->_vspacerDown->gap());
      return staves[i]->distanceDown;
      }

//---------------------------------------------------------
//   distanceUp
//---------------------------------------------------------

qreal Measure::distanceUp(int i) const
      {
      if (staves[i]->_vspacerUp)
            return qMax(staves[i]->distanceUp, staves[i]->_vspacerUp->gap());
      return staves[i]->distanceUp;
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Measure::isEmpty() const
      {
      if (_irregular)
            return false;
      int n = 0;
      int tracks = staves.size() * VOICES;
      static const Segment::Type st { Segment::Type::ChordRest };
      for (const Segment* s = first(st); s; s = s->next(st)) {
            bool restFound = false;
            for (int track = 0; track < tracks; ++track) {
                  if ((track % VOICES) == 0 && !score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  if (s->element(track))  {
                        if (s->element(track)->type() != Element::Type::REST)
                              return false;
                        Rest* rest = static_cast<Rest*>(s->element(track));
                        if (rest->articulations().size() > 0) // break on fermata
                              return false;
                        restFound = true;
                        }
                  }
            if (restFound)
                  ++n;
            // measure is not empty if there is more than one rest
            if (n > 1)
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   isOnlyRests
//---------------------------------------------------------

bool Measure::isOnlyRests(int track) const
      {
      static const Segment::Type st { Segment::Type::ChordRest };
      for (const Segment* s = first(st); s; s = s->next(st)) {
            if (s->segmentType() != Segment::Type::ChordRest || !s->element(track))
                  continue;
            if (s->element(track)->type() != Element::Type::REST)
                  return false;
            }
      return true;
      }


//---------------------------------------------------------
//   Space::max
//---------------------------------------------------------

void Space::max(const Space& s)
      {
      if (s._lw > _lw)
            _lw = s._lw;
      if (s._rw > _rw)
            _rw = s._rw;
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void Measure::setDirty()
      {
      _minWidth1 = 0.0;
      _minWidth2 = 0.0;
      }

//---------------------------------------------------------
//   systemHeader
///   return true if the measure contains a system header
//    The system header is identified by a generated Clef in
//    the first segment.
//---------------------------------------------------------

bool Measure::systemHeader() const
      {
      Segment* s = first();
      return s && (s->segmentType() == Segment::Type::Clef) && s->element(0) && s->element(0)->generated();
      }

//---------------------------------------------------------
//   minWidth1
///   return minimum width of measure excluding system
///   header
//---------------------------------------------------------

qreal Measure::minWidth1() const
      {
      if (_minWidth1 == 0.0) {
            int nstaves = score()->nstaves();
            Segment* s = first();
            Segment::Type st = Segment::Type::Clef | Segment::Type::KeySig | Segment::Type::StartRepeatBarLine;
            while ((s->segmentType() & st) && s->next()) {
                  // found a segment that we might be able to skip
                  // we can do so only if it contains no non-generated elements
                  // note that it is possible for the same segment to contain both generated and non-generated elements
                  // consider, a keysig segment at the start of a system in which one staff has a local key change
                  bool generated = true;
                  for (int i = 0; i < nstaves; ++i) {
                        Element* e = s->element(i * VOICES);
                        if (e && !e->generated()) {
                              generated = false;
                              break;
                              }
                        }
                  if (!generated)
                        break;
                  s = s->next();
                  }
            _minWidth1 = score()->computeMinWidth(s, false);
            if (MScore::debugMode)
                  qDebug("Measure::minWidth1: %d %f", no(), _minWidth1);
            }
      return _minWidth1;
      }

//---------------------------------------------------------
//   minWidth2
///   return minimum width of measure including system
///  header if present
//---------------------------------------------------------

qreal Measure::minWidth2() const
      {
      if (_minWidth2 == 0.0) {
            _minWidth2 = score()->computeMinWidth(first(), system()->firstMeasure() == this);
            if (MScore::debugMode)
                  qDebug("Measure::minWidth2: %d %f", no(), _minWidth2 / DPMM);
            }
      return _minWidth2;
      }

//-----------------------------------------------------------------------------
//    computeStretch
///   \brief distribute stretch across a range of segments
//-----------------------------------------------------------------------------

void computeStretch(int minTick, qreal minimum, qreal stretch, int first, int last, int ticksList[], qreal xpos[], qreal width[])
      {
      SpringMap springs;
      for (int i = first; i < last; ++i) {
            qreal str = 1.0;
            qreal d;
            qreal w = width[i];

            int t = ticksList[i];
            if (t) {
                  if (minTick > 0)
                        // str += .6 * log(qreal(t) / qreal(minTick)) / log(2.0);
                        str = 1.0 + 0.865617 * log(qreal(t) / qreal(minTick));
                  d = w / str;
                  }
            else {
                  str = 0.0;              // dont stretch timeSig and key
                  d   = 100000000.0;      // CHECK
                  }
            springs.insert(std::pair<qreal, Spring>(d, Spring(i, str, w)));
            minimum += w;
            }

      //---------------------------------------------------
      //    distribute stretch to segments
      //---------------------------------------------------

      qreal force = sff(stretch, minimum, springs);

      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            qreal stretch = force * i->second.stretch;
            if (stretch < i->second.fix)
                  stretch = i->second.fix;
            width[i->second.seg] = stretch;
            }
      qreal x = xpos[first];
      for (int i = first; i < last; ++i) {
            x += width[i];
            xpos[i+1] = x;
            }
      }

//-----------------------------------------------------------------------------
//    layoutX
///   \brief main layout routine for note spacing
///   Return width of measure (in MeasureWidth), taking into account \a stretch.
//-----------------------------------------------------------------------------

void Measure::layoutX(qreal stretch)
      {
      int nstaves = _score->nstaves();

      int segs = 0;
      for (const Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() == Segment::Type::Clef && (s != first()))
                  continue;
            ++segs;
            }

      if (nstaves == 0 || segs == 0)
            return;

      qreal _spatium              = spatium();
      int tracks                  = nstaves * VOICES;
      qreal clefKeyRightMargin    = score()->styleS(StyleIdx::clefKeyRightMargin).val() * _spatium;
      qreal minHarmonyDistance    = score()->styleS(StyleIdx::minHarmonyDistance).val() * _spatium;
      qreal maxHarmonyBarDistance = score()->styleS(StyleIdx::maxHarmonyBarDistance).val() * _spatium;

      qreal rest[nstaves];    // fixed space needed from previous segment
      memset(rest, 0, nstaves * sizeof(qreal));

      qreal hRest[nstaves];    // fixed space needed from previous harmony
      memset(hRest, 0, nstaves * sizeof(qreal));

      //--------tick table for segments
      int ticksList[segs];
      memset(ticksList, 0, segs * sizeof(int));

      qreal xpos[segs+1];
      qreal width[segs];

      int segmentIdx = 0;
      qreal x        = 0.0;
      qreal lastx    = 0.0;
      int minTick    = 100000;
      int hMinTick   = 100000;
      int hLastIdx   = -1;
      int ntick      = ticks();   // position of next measure

      if (system() && system()->firstMeasure() == this && system()->barLine()) {
            BarLine* bl = system()->barLine();
            x += BarLine::layoutWidth(score(), bl->barLineType(), bl->mag());
            }

      qreal minNoteDistance = score()->styleS(StyleIdx::minNoteDistance).val() * _spatium;

      qreal clefWidth[nstaves];
      memset(clefWidth, 0, nstaves * sizeof(qreal));

      std::vector<QRectF> hLastBbox(nstaves);    // bbox of previous harmony to test vertical separation

      const Segment* s = first();
      const Segment* pSeg = 0;
      for (; s; s = s->next(), ++segmentIdx) {
            qreal elsp = s->extraLeadingSpace().val()  * _spatium;
            qreal etsp = s->extraTrailingSpace().val() * _spatium;
            if ((s->segmentType() == Segment::Type::Clef) && (s != first())) {
                  --segmentIdx;
                  for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                        if (!score()->staff(staffIdx)->show())
                              continue;
                        int track  = staffIdx * VOICES;
                        Element* e = s->element(track);
                        if (e) {
                              clefWidth[staffIdx] = e->width() + _spatium + elsp;
                              }
                        }
                  pSeg = s;
                  continue;
                  }
            bool rest2[nstaves];
            bool hRest2[nstaves];
            bool spaceHarmony     = false;
            Segment::Type segType = s->segmentType();
            qreal segmentWidth    = 0.0;
            qreal harmonyWidth    = 0.0;
            qreal stretchDistance = 0.0;
            Segment::Type pt      = pSeg ? pSeg->segmentType() : Segment::Type::BarLine;
#if 0
            qreal firstHarmonyDistance = 0.0;
#endif

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  qreal minDistance = 0.0;
                  Space space;
                  Space hSpace;
                  QRectF hBbox;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  bool hFound = false;
                  bool eFound = false;
#if 0
                  qreal harmonyCarryOver = system()->firstMeasure() == this ? 0.0 : // calculate value for this staff; but how to duplicate in Score::computeMinWidth?
#endif

                  if (segType & (Segment::Type::ChordRest)) {
                        qreal llw = 0.0;
                        qreal rrw = 0.0;
                        Lyrics* lyrics = 0;
                        bool accidentalStaff = false;

                        bool accidental = false;
                        bool grace = false;
                        qreal accidentalX = 0.0;
                        qreal noteX = 0.0;
                        if (pt & (Segment::Type::StartRepeatBarLine | Segment::Type::BarLine | Segment::Type::TimeSig) && !accidentalStaff) {
                              for (int voice = 0; voice < VOICES; ++voice) {
                                    ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                                    if (!cr)
                                          continue;
                                    // check for accidentals in chord
                                    if (cr->type() == Element::Type::CHORD) {
                                          Chord* c = static_cast<Chord*>(cr);
                                          if (c->graceNotesBefore().size())
                                                grace = true;
                                          else {
                                                for (Note* note : c->notes()) {
                                                      if (note->accidental() && !note->fixed()) {
                                                            accidental = true;
                                                            accidentalX = qMin(accidentalX, note->accidental()->x() + note->x() + c->x());
                                                            }
                                                      else
                                                            noteX = qMin(noteX, note->x() + c->x());
                                                      }
                                                }
                                          }
                                    }
                              }
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (pt & (Segment::Type::StartRepeatBarLine | Segment::Type::BarLine | Segment::Type::TimeSig) && !accidentalStaff) {
                                    // no distance to full measure rest
                                    if (!(cr->type() == Element::Type::REST && static_cast<Rest*>(cr)->durationType() == TDuration::DurationType::V_MEASURE)) {
                                          accidentalStaff = true;
                                          qreal sp;
                                          qreal bnd = score()->styleS(StyleIdx::barNoteDistance).val() * _spatium;
                                          if (accidental) {
                                                qreal bad = score()->styleS(StyleIdx::barAccidentalDistance).val() * _spatium;
                                                qreal diff = qMax(noteX - accidentalX, 0.0);
                                                sp = qMax(bad, bnd - diff);
                                                }
                                          else if (grace)
                                                sp = score()->styleS(StyleIdx::barGraceDistance).val() * _spatium;
                                          else
                                                sp = bnd;
                                          if (pt & Segment::Type::TimeSig)
                                                sp += clefKeyRightMargin - bnd;
                                          minDistance = qMax(minDistance, sp);
                                          }
                                    else if (pt & Segment::Type::TimeSig)
                                          minDistance = qMax(minDistance, clefKeyRightMargin);
                                    }
                              else if (pt & (Segment::Type::ChordRest)) {
                                    minDistance = qMax(minDistance, minNoteDistance);
                                    }
                              else {
                                    bool firstClef = (pt == Segment::Type::Clef) && (pSeg && pSeg->rtick() == 0);
                                    if ((pt & Segment::Type::KeySig) || firstClef)
                                          minDistance = qMax(minDistance, clefKeyRightMargin);
                                    }

                              // special case:
                              // make extra space for ties or glissandi continued from previous system

                              if (cr->type() == Element::Type::CHORD) {
                                    Chord* c = static_cast<Chord*>(cr);
                                    if (system() && system()->firstMeasure() == this && c->tick() == tick()) {
                                          if (c->endsGlissando()) {
                                                minDistance = qMax(minDistance, _spatium * GLISS_STARTOFSYSTEM_WIDTH);
                                                }
                                          else {
                                                for (Note* note : c->notes()) {
                                                      if (note->tieBack()) {
                                                            minDistance = qMax(minDistance, _spatium * 2);
                                                            break;
                                                            }
                                                      }
                                                }
                                          }
                                    }

                              // calculate space needed for segment
                              // take cr position into account
                              // by converting to segment-relative space
                              // chord space itself already has ipos offset built in
                              // but lyrics do not
                              // and neither have user offsets
                              qreal cx = cr->ipos().x();
                              qreal cxu = cr->userOff().x();
                              qreal lx = qMax(cxu, 0.0); // nudge left shouldn't require more leading space
                              qreal rx = qMin(cxu, 0.0); // nudge right shouldn't require more trailing space
                              // Score::computeMinWidth already allocated enough space for full measure rests
                              // but do not account for them when spacing within the measure
                              // because they do not need to align with the other elements in segment
                              if (cr->durationType() != TDuration::DurationType::V_MEASURE) {
                                    Space crSpace = cr->space();
                                    Space segRelSpace(crSpace.lw()-lx, crSpace.rw()+rx);
                                    space.max(segRelSpace);
                                    }

                              // lyrics
                              int n = cr->lyricsList().size();
                              for (int i = 0; i < n; ++i) {
                                    Lyrics* l = cr->lyricsList().at(i);
                                    if (!l || l->isEmpty())
                                          continue;
                                    if (!lyrics || l->no() > lyrics->no())
                                          lyrics = l;
                                    QRectF b(l->bbox().translated(l->pos()));
                                    llw = qMax(llw, -(b.left()+lx+cx));
                                    rrw = qMax(rrw, b.right()+rx+cx);
                                    }
                              }
                        if (lyrics) {
                              qreal y = lyrics->ipos().y() + score()->styleS(StyleIdx::lyricsMinBottomDistance).val() * _spatium;
                              y -= score()->staff(staffIdx)->height();
                              if (y > staves[staffIdx]->distanceDown)
                                 staves[staffIdx]->distanceDown = y;
                              space.max(Space(llw, rrw));
                              }

                        // add spacing for chord symbols
                        foreach (const Element* e, s->annotations()) {
                              if (e->type() != Element::Type::HARMONY || e->track() < track || e->track() >= track+VOICES)
                                    continue;
                              const Harmony* h = static_cast<const Harmony*>(e);
                              QRectF b(h->bboxtight().translated(h->pos()));
                              if (hFound)
                                    hBbox |= b;
                              else
                                    hBbox = b;
                              hFound = true;
                              spaceHarmony = true;
                              // allow chord to be dragged
                              qreal xoff = h->pos().x();
                              qreal bl = -b.left() + qMin(xoff, 0.0);
                              qreal br = b.right() - qMax(xoff, 0.0);
                              hSpace.max(Space(bl, br));
#if 0
                              hSpace.max(Space(s->rtick()?-b.left():0.0, b.right()));
                              // account for carryover from last measure
                              if (harmonyCarryOver > 0.0) {
                                    if (!s->rtick()) {
                                          // first ChordRest of measure
                                          // use minDistance to clear carryover harmony
                                          minDistance = qMax(minDistance, harmonyCarryOver - x);
                                          }
                                    else {
                                          // otherwise, use stretch
                                          firstHarmonyDistance = qMax(firstHarmonyDistance, harmonyCarryOver + minHarmonyDistance);
                                          }
                                    harmonyCarryOver = 0.0;
                                    }
#endif
                              }
                        }
                  else {
                        // current segment (s) is not a ChordRest
                        Element* e = s->element(track);
                        if (segType == Segment::Type::StartRepeatBarLine && !e) {
                              for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                                    Element* ee = s->element(staffIdx * VOICES);
                                    if (ee) {
                                          BarLine* bl = static_cast<BarLine*>(ee);
                                          int strack = staffIdx * VOICES;
                                          int etrack = (staffIdx + bl->span()) * VOICES;
                                          if (track >= strack && track < etrack) {
                                                e = ee;
                                                break;
                                                }
                                          }
                                    }
                              }
                        if ((segType == Segment::Type::Clef) && (pt != Segment::Type::ChordRest))
                              minDistance = score()->styleS(StyleIdx::clefLeftMargin).val() * _spatium;
                        else if (segType == Segment::Type::StartRepeatBarLine && pSeg)
                              minDistance = .5 * _spatium;
                        else if (segType == Segment::Type::TimeSig && pt == Segment::Type::Clef) {
                              // missing key signature, but allocate default margin anyhow
                              minDistance = score()->styleS(StyleIdx::keysigLeftMargin).val() * _spatium;
                              }
                        else if ((segType == Segment::Type::EndBarLine) && segmentIdx) {
                              if (pt == Segment::Type::Clef)
                                    minDistance = score()->styleS(StyleIdx::clefBarlineDistance).val() * _spatium;
                              else
                                    stretchDistance = score()->styleS(StyleIdx::noteBarDistance).val() * _spatium;
                              if (e == 0) {
                                    // look for barline
                                    for (int i = track - VOICES; i >= 0; i -= VOICES) {
                                          e = s->element(i);
                                          if (e)
                                                break;
                                          }
                                    }
                              }
                        if (e) {
                              eFound = true;
                              if (!s->next())               // segType & Segment::Type::EndBarLine
                                    spaceHarmony = true;    // to space last Harmony to end of measure
                              space.max(e->space());
                              }
                        }
                  space += Space(elsp, etsp);
                  // if (isMMRest())            //?? minDistance needed for clef-repeatbarline distance
                        // minDistance = 0;

                  if (found || eFound) {
                        space.addL(clefWidth[staffIdx]);
                        qreal sp     = minDistance + rest[staffIdx] + qMax(space.lw(), stretchDistance);
                        rest[staffIdx]  = space.rw();
                        rest2[staffIdx] = false;
                        segmentWidth    = qMax(segmentWidth, sp);
                        }
                  else
                        rest2[staffIdx] = true;

                  // space chord symbols separately from segments
                  if (hFound || spaceHarmony) {
                        qreal sp = 0.0;

                        // space chord symbols unless they miss each other vertically
                        if (hFound && hBbox.top() < hLastBbox[staffIdx].bottom() && hBbox.bottom() > hLastBbox[staffIdx].top())
                              sp = hRest[staffIdx] + minHarmonyDistance + hSpace.lw();
                        // barline: limit space to maxHarmonyBarDistance
                        else if (spaceHarmony)
                              sp = qMin(hRest[staffIdx], maxHarmonyBarDistance);

                        hLastBbox[staffIdx] = hBbox;
                        hRest[staffIdx] = hSpace.rw();
                        hRest2[staffIdx] = false;
                        harmonyWidth = qMax(harmonyWidth, sp);
                        }
                  else
                        hRest2[staffIdx] = true;

                  clefWidth[staffIdx] = 0.0;
                  }

            // set previous seg width before adding in harmony, to allow stretching
            if (segmentIdx) {
                  width[segmentIdx-1] = segmentWidth;
                  if (pSeg)
                        pSeg->setbbox(QRectF(0.0, 0.0, segmentWidth, _spatium * 5));  //??
                  }

            // make room for harmony if needed
            segmentWidth = qMax(segmentWidth, harmonyWidth);

            x += segmentWidth;
            xpos[segmentIdx]  = x;

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  if (rest2[staffIdx])
                        rest[staffIdx] -= qMin(rest[staffIdx], segmentWidth);
                  if (hRest2[staffIdx])
                        hRest[staffIdx] -= qMin(hRest[staffIdx], segmentWidth);
                  }

            if ((s->segmentType() == Segment::Type::ChordRest)) {
                  const Segment* nseg = s;
                  for (;;) {
                        nseg = nseg->next();
                        if (nseg == 0 || nseg->segmentType() == Segment::Type::ChordRest)
                              break;
                        }
                  int nticks = (nseg ? nseg->rtick() : ntick) - s->rtick();
                  if (nticks == 0) {
                        // this happens for tremolo notes
                        qDebug("layoutX: empty segment(%p)%s: measure: tick %d ticks %d",
                           s, s->subTypeName(), tick(), ticks());
                        qDebug("         nticks==0 segmente %d, segmentIdx: %d, segTick: %d nsegTick(%p) %d",
                           size(), segmentIdx-1, s->tick(), nseg, ntick
                           );
                        }
                  else {
                        if (nticks < minTick)
                              minTick = nticks;
                        if (nticks < hMinTick)
                              hMinTick = nticks;
                        }
                  ticksList[segmentIdx] = nticks;
                  }
            else
                  ticksList[segmentIdx] = 0;

            // if we are on a chord symbol, stretch the notes below it if necessary
            if (spaceHarmony) {
                  if (hLastIdx >= 0) {
                        computeStretch(hMinTick, 0.0, x-lastx, hLastIdx, segmentIdx, ticksList, xpos, width);
                        }
#if 0
                  else if (s->rtick() && firstHarmonyDistance > 0.0) {
                        // account for carryover from previous measure
                        qDebug("measure %d, initial %d segments: stretching to %f", _no, segmentIdx, firstHarmonyDistance);
                        computeStretch(0, 0.0, firstHarmonyDistance, 0, segmentIdx, ticksList, xpos, width);
                        firstHarmonyDistance = 0.0;
                        }
#endif
                  hMinTick = 10000;
                  lastx = x;
                  hLastIdx = segmentIdx;
                  }

            //
            // set pSeg only to used segments
            //
            for (int voice = 0; voice < nstaves * VOICES; ++voice) {
                  if (!score()->staff(voice/VOICES)->show()) {
                        voice += VOICES-1;
                        continue;
                        }
                  if (s->element(voice)) {
                        pSeg = s;
                        break;
                        }
                  }
            }

      //---------------------------------------------------
      // TAB: compute distance above and below
      //---------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!score()->staff(staffIdx)->show())
                  continue;
            qreal distAbove = 0.0;
            qreal distBelow = 0.0;
            Staff * staff = _score->staff(staffIdx);
            if (staff->isTabStaff()) {
                  StaffType* stt = staff->staffType();
                  if (stt->slashStyle())        // if no stems
                        distAbove = stt->genDurations() ? -stt->durationBoxY() : 0.0;
                  else {                        // if stems
                        if (mstaff(staffIdx)->hasVoices) {  // reserve space both above and below
                              distBelow = (STAFFTYPE_TAB_DEFAULTSTEMLEN_UP + STAFFTYPE_TAB_DEFAULTSTEMDIST_UP)*_spatium;
                              distAbove = (STAFFTYPE_TAB_DEFAULTSTEMLEN_DN + STAFFTYPE_TAB_DEFAULTSTEMDIST_DN)*_spatium;
                              }
                        else {                              // if no voices, reserve space on stem side
                              if (stt->stemsDown())
                                    distBelow = (STAFFTYPE_TAB_DEFAULTSTEMLEN_UP + STAFFTYPE_TAB_DEFAULTSTEMDIST_UP)*_spatium;
                              else
                                    distAbove = (STAFFTYPE_TAB_DEFAULTSTEMLEN_DN + STAFFTYPE_TAB_DEFAULTSTEMDIST_DN)*_spatium;
                              }
                        }
                  if (distAbove > staves[staffIdx]->distanceUp)
                     staves[staffIdx]->distanceUp = distAbove;
                  if (distBelow > staves[staffIdx]->distanceDown)
                     staves[staffIdx]->distanceDown = distBelow;
                  }
            }

      qreal segmentWidth = 0.0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!score()->staff(staffIdx)->show())
                  continue;
            segmentWidth = qMax(segmentWidth, rest[staffIdx]);
            segmentWidth = qMax(segmentWidth, hRest[staffIdx]);
            }
      if (segmentIdx)
            width[segmentIdx-1] = segmentWidth;
      xpos[segmentIdx]    = x + segmentWidth;

      //---------------------------------------------------
      // compute stretches for whole measure
      //---------------------------------------------------

      computeStretch(minTick, xpos[0], stretch, 0, segs, ticksList, xpos, width);

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      int seg = 0;
      for (Segment* s = first(); s; s = s->next(), ++seg) {
            if ((s->segmentType() == Segment::Type::Clef) && (s != first())) {
                  //
                  // clefs are not in xpos[] table
                  //
                  s->setPos(xpos[seg], 0.0);
                  for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                        if (!score()->staff(staffIdx)->show())
                              continue;
                        int track  = staffIdx * VOICES;
                        Element* e = s->element(track);
                        if (e) {
                              qreal lm = 0.0;
                              if (s->next()) {
                                    for (int track = staffIdx * VOICES; track < staffIdx*VOICES+VOICES; ++track) {
                                          if (s->next()->element(track)) {
                                                qreal clm = s->next()->element(track)->space().lw();
                                                lm = qMax(lm, clm);
                                                }
                                          }
                                    }
                              e->setPos(-e->width() - lm - _spatium*.5, 0.0);
                              e->adjustReadPos();
                              }
                        }
                  --seg;
                  continue;
                  }
            s->setPos(xpos[seg], 0.0);
            }

      for (Segment* s = first(); s; s = s->next(), ++seg) {
            qreal barLineWidth = 0.0;
            for (int track = 0; track < tracks; ++track) {
                  if (!score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  Element::Type t = e->type();
                  Rest* rest = static_cast<Rest*>(e);
                  if (t == Element::Type::REPEAT_MEASURE || (t == Element::Type::REST && (isMMRest() || rest->isFullMeasureRest()))) {
                        //
                        // element has to be centered in free space
                        //    x1 - left measure position of free space
                        //    x2 - right measure position of free space

                        if (isMMRest()) {
                              //
                              // center multi measure rest
                              //
                              qreal x1 = 0.0, x2;
                              Segment* ss = first();
                              Segment* ps = nullptr;
                              for (; ss->segmentType() != Segment::Type::ChordRest; ss = ss->next()) {
                                    if (!ss->isEmpty())
                                          ps = ss;
                                    }
                              if (ps) {
                                    ss = ps;
                                    for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                                          int track = staffIdx * VOICES;
                                          Element* e = ss->element(track);
                                          if (e)
                                                x1 = qMax(x1, ss->x() + e->x() + e->width());
                                          }
                                    }
                              Segment* ns = s->next();
                              x2 = this->width();
                              if (ns) {
                                    x2 = ns->x();
                                    if (ns->segmentType() != Segment::Type::EndBarLine) {
                                          for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                                                int track = staffIdx * VOICES;
                                                Element* e = ns->element(track);
                                                if (e)
                                                      x2 = qMin(x2, ns->x() + e->x());
                                                }
                                          }
                                    }

                              qreal d  = point(score()->styleS(StyleIdx::multiMeasureRestMargin));
                              qreal w = x2 - x1 - 2 * d;

                              rest->setMMWidth(w);
                              StaffLines* sl = staves[track/VOICES]->lines;
                              qreal x = x1 - s->pos().x() + d;
                              e->setPos(x, sl->staffHeight() * .5);   // center vertically in measure
                              }
                        else { // if (rest->isFullMeasureRest()) {
                              //
                              // center full measure rest
                              //
                              qreal x1 = 0.0;
                              qreal x2 = this->width();
                              Segment* ss = first();
                              Segment* ps = nullptr;
                              for (; ss->segmentType() != Segment::Type::ChordRest; ss = ss->next()) {
                                    if (!ss->isEmpty())
                                          ps = ss;
                                    }
                              if (ps) {
                                    ss = ps;
                                    for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                                          if (!visible(staffIdx))
                                                continue;
                                          int track = staffIdx * VOICES;
                                          Element* e = ss->element(track);
                                          if (e)
                                                x1 = qMax(x1, ss->x() + e->x() + e->width());
                                          }
                                    }

                              Segment* ns = s->next();
                              while (ns && ns->segmentType() != Segment::Type::EndBarLine) {
                                    ns = ns->next();
                                    }
                              if (ns)
                                    x2 = ns->x();

                              rest->rxpos() = (x2 - x1 - e->width()) * .5 + x1 - s->x() - e->bbox().x();
                              rest->adjustReadPos();
                              }
                        }
                  else if (t == Element::Type::REST)
                        e->rxpos() = 0;
                  else if (t == Element::Type::CHORD)
                        static_cast<Chord*>(e)->layout2();
                  else if (t == Element::Type::CLEF) {
                        if (s == first()) {
                              // clef at the beginning of measure
                              qreal gap = 0.0;
                              Segment* ps = s->prev();
                              if (ps)
                                    gap = s->x() - (ps->x() + ps->width());
                              e->rxpos() = -gap * .5;
                              e->adjustReadPos();
                              }
                        }
                  else if (t == Element::Type::BAR_LINE) {
                        e->setPos(QPointF());
                        barLineWidth = qMax(barLineWidth, e->width());
                        e->adjustReadPos();
                        }
                  else {
                        if (t != Element::Type::AMBITUS)
                              e->setPos(-e->bbox().x(), 0.0);
                        e->adjustReadPos();
                        }
                  }
            // align bar lines
            if (s->segmentType() == Segment::Type::EndBarLine)
                  for (int track = 0; track < tracks; ++track)
                        if (s->element(track)) {
                              BarLine*    bl    = static_cast<BarLine*>(s->element(track));
                              qreal       delta = barLineWidth - bl->width();
                              // right-align end bar lines, end-repeat bar lines and bar lines in last system measure
                              if (bl->barLineType() == BarLineType::END || bl->barLineType() == BarLineType::END_REPEAT
                                    || (system() && this == system()->lastMeasure()))
                                    bl->rxpos() = delta;
                              // centre-align all other bar lines except start-repeat
                              else if (bl->barLineType() != BarLineType::START_REPEAT)
                                    bl->rxpos() = delta * 0.5;
                              // leave start-repeat left aligned
                              }
            }
      }

//---------------------------------------------------------
//   layoutStage1
//    compute multi measure rest break
//    call layoutCR0 for every chord/rest
//---------------------------------------------------------

void Measure::layoutStage1()
      {
      setDirty();

      bool mmrests = score()->styleB(StyleIdx::createMultiMeasureRests);
      setBreakMMRest(false);

      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            AccidentalState as;      // list of already set accidentals for this measure
            Staff* staff = score()->staff(staffIdx);
            as.init(staff->key(tick()));

            if (mmrests) {
                  if (
                        (repeatFlags() & Repeat::START)
                     || (prevMeasure() && (prevMeasure()->repeatFlags() & Repeat::END))
                     || (prevMeasure() && (prevMeasure()->sectionBreak()))
                     )
                        setBreakMMRest(true);
                  else if (!breakMultiMeasureRest()) {
                        for (Segment* s = first(); s; s = s->next()) {
                              for (Element* e : s->annotations()) {
                                    if (score()->staff(e->staffIdx())->show() || e->systemFlag()) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              if (breakMultiMeasureRest())      // optimize
                                    break;
                              }
                        }
                  }

            int track    = staffIdx * VOICES;
            int endTrack = track + VOICES;

            for (Segment* segment = first(); segment; segment = segment->next()) {
                  if (score()->staff(staffIdx)->show()) {
                        Element* e = segment->element(track);
                        if (e && !e->generated()) {
                              if (segment->segmentType() & (Segment::Type::StartRepeatBarLine))
                                    setBreakMMRest(true);
                              if (segment->segmentType() & (Segment::Type::KeySig | Segment::Type::TimeSig) && tick())
                                    setBreakMMRest(true);
                              else if (segment->segmentType() == Segment::Type::Clef) {
                                    if (segment->tick() != endTick() && tick())
                                          setBreakMMRest(true);
                                    }
                              }
                        }

                  if (segment->segmentType() == Segment::Type::ChordRest) {
                        Staff* staff     = score()->staff(staffIdx);
                        qreal staffMag  = staff->mag();

                        for (int t = track; t < endTrack; ++t) {
                              ChordRest* cr = static_cast<ChordRest*>(segment->element(t));
                              if (!cr)
                                    continue;
                              layoutCR0(cr, staffMag, &as);
                              }
                        }
                  }
            }

      if (!mmrests || breakMultiMeasureRest())
            return;

      Measure* pm = prevMeasure();
      if (pm) {
            if (pm->endBarLineType() != BarLineType::NORMAL
               && pm->endBarLineType() != BarLineType::BROKEN
               && pm->endBarLineType() != BarLineType::DOTTED) {
                  setBreakMMRest(true);
                  return;
                  }
            if (pm->findSegment(Segment::Type::Clef, tick()))
                  setBreakMMRest(true);
            }

      }

//---------------------------------------------------------
//   stretchedLen
//---------------------------------------------------------

Fraction Measure::stretchedLen(Staff* staff) const
      {
      return len() * staff->timeStretch(tick());
      }

//---------------------------------------------------------
//   cloneMeasure
//---------------------------------------------------------

Measure* Measure::cloneMeasure(Score* sc, TieMap* tieMap)
      {
      Measure* m      = new Measure(sc);
      m->_timesig     = _timesig;
      m->_len         = _len;
      m->_repeatCount = _repeatCount;
      m->_repeatFlags = _repeatFlags;

      m->staves.clear();
      for (MStaff* ms : staves)
            m->staves.append(new MStaff(*ms));

      m->_no                    = _no;
      m->_noOffset              = _noOffset;
      m->_userStretch           = _userStretch;
      m->_irregular             = _irregular;
      m->_breakMultiMeasureRest = _breakMultiMeasureRest;
      m->_breakMMRest           = _breakMMRest;
      m->_endBarLineGenerated   = _endBarLineGenerated;
      m->_endBarLineVisible     = _endBarLineVisible;
      m->_endBarLineType        = _endBarLineType;
      m->_playbackCount         = _playbackCount;
      m->_endBarLineColor       = _endBarLineColor;

      m->_minWidth1             = _minWidth1;
      m->_minWidth2             = _minWidth2;

      m->setTick(tick());
      m->setLineBreak(lineBreak());
      m->setPageBreak(pageBreak());
      m->setSectionBreak(sectionBreak() ? new LayoutBreak(*sectionBreak()) : 0);

      int tracks = sc->nstaves() * VOICES;
      TupletMap tupletMap;

      for (Segment* oseg = first(); oseg; oseg = oseg->next()) {
            Segment* s = new Segment(m);
            s->setSegmentType(oseg->segmentType());
            s->setRtick(oseg->rtick());
            m->_segments.push_back(s);
            for (int track = 0; track < tracks; ++track) {
                  Element* oe = oseg->element(track);
                  foreach (Element* e, oseg->annotations()) {
                        if (e->generated() || e->track() != track)
                              continue;
                        Element* ne = e->clone();
                        ne->setTrack(track);
                        ne->setUserOff(e->userOff());
                        ne->setScore(sc);
                        s->add(ne);
                        }
                  if (!oe)
                        continue;
                  Element* ne = oe->clone();
                  if (oe->isChordRest()) {
                        ChordRest* ocr = static_cast<ChordRest*>(oe);
                        ChordRest* ncr = static_cast<ChordRest*>(ne);
                        Tuplet* ot     = ocr->tuplet();
                        if (ot) {
                              Tuplet* nt = tupletMap.findNew(ot);
                              if (nt == 0) {
                                    nt = new Tuplet(*ot);
                                    nt->clear();
                                    nt->setTrack(track);
                                    nt->setScore(sc);
                                    tupletMap.add(ot, nt);
                                    }
                              ncr->setTuplet(nt);
                              nt->add(ncr);
                              }
                        if (oe->type() == Element::Type::CHORD) {
                              Chord* och = static_cast<Chord*>(ocr);
                              Chord* nch = static_cast<Chord*>(ncr);
                              int n = och->notes().size();
                              for (int i = 0; i < n; ++i) {
                                    Note* on = och->notes().at(i);
                                    Note* nn = nch->notes().at(i);
                                    if (on->tieFor()) {
                                          Tie* tie = on->tieFor()->clone();
                                          tie->setScore(sc);
                                          nn->setTieFor(tie);
                                          tie->setStartNote(nn);
                                          tieMap->add(on->tieFor(), tie);
                                          }
                                    if (on->tieBack()) {
                                          Tie* tie = tieMap->findNew(on->tieBack());
                                          if (tie) {
                                                nn->setTieBack(tie);
                                                tie->setEndNote(nn);
                                                }
                                          else {
                                                qDebug("cloneMeasure: cannot find tie, track %d", track);
                                                }
                                          }
                                    }
                              }
                        }
                  ne->setUserOff(oe->userOff());
                  ne->setScore(sc);
                  s->add(ne);
                  }
            }
      foreach(Element* e, el()) {
            Element* ne = e->clone();
            ne->setScore(sc);
            ne->setUserOff(e->userOff());
            m->add(ne);
            }
      return m;
      }

//---------------------------------------------------------
//   pos2sel
//---------------------------------------------------------

int Measure::snap(int tick, const QPointF p) const
      {
      Segment* s = first();
      for (; s->next(); s = s->next()) {
            qreal x  = s->x();
            qreal dx = s->next()->x() - x;
            if (s->tick() == tick)
                  x += dx / 3.0 * 2.0;
            else  if (s->next()->tick() == tick)
                  x += dx / 3.0;
            else
                  x += dx * .5;
            if (p.x() < x)
                  break;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   snapNote
//---------------------------------------------------------

int Measure::snapNote(int /*tick*/, const QPointF p, int staff) const
      {
      Segment* s = first();
      for (;;) {
            Segment* ns = s->next();
            while (ns && ns->element(staff) == 0)
                  ns = ns->next();
            if (ns == 0)
                  break;
            qreal x  = s->x();
            qreal nx = x + (ns->x() - x) * .5;
            if (p.x() < nx)
                  break;
            s = ns;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Measure::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TIMESIG_NOMINAL:
                  return QVariant::fromValue(_timesig);
            case P_ID::TIMESIG_ACTUAL:
                  return QVariant::fromValue(_len);
            case P_ID::REPEAT_FLAGS:
                  return int(repeatFlags());
            case P_ID::MEASURE_NUMBER_MODE:
                  return int(measureNumberMode());
            case P_ID::BREAK_MMR:
                  return getBreakMultiMeasureRest();
            case P_ID::REPEAT_COUNT:
                  return repeatCount();
            case P_ID::USER_STRETCH:
                  return userStretch();
            case P_ID::NO_OFFSET:
                  return noOffset();
            case P_ID::IRREGULAR:
                  return irregular();
            case P_ID::SYSTEM_INITIAL_BARLINE_TYPE:
                  return int(systemInitialBarLineType());
            default:
                  return MeasureBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Measure::setProperty(P_ID propertyId, const QVariant& value)
      {
      switch(propertyId) {
            case P_ID::TIMESIG_NOMINAL:
                  _timesig = value.value<Fraction>();
                  break;
            case P_ID::TIMESIG_ACTUAL:
                  _len = value.value<Fraction>();
                  break;
            case P_ID::REPEAT_FLAGS:
                  setRepeatFlags(Repeat(value.toInt()));
                  break;
            case P_ID::MEASURE_NUMBER_MODE:
                  setMeasureNumberMode(MeasureNumberMode(value.toInt()));
                  break;
            case P_ID::BREAK_MMR:
                  setBreakMultiMeasureRest(value.toBool());
                  break;
            case P_ID::REPEAT_COUNT:
                  setRepeatCount(value.toInt());
                  break;
            case P_ID::USER_STRETCH:
                  setUserStretch(value.toDouble());
                  break;
            case P_ID::NO_OFFSET:
                  setNoOffset(value.toInt());
                  break;
            case P_ID::IRREGULAR:
                  setIrregular(value.toBool());
                  break;
            case P_ID::SYSTEM_INITIAL_BARLINE_TYPE:
                  setSystemInitialBarLineType(BarLineType(value.toInt()));
                  break;
            default:
                  return MeasureBase::setProperty(propertyId, value);
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Measure::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TIMESIG_NOMINAL:
            case P_ID::TIMESIG_ACTUAL:
                  return QVariant();
            case P_ID::REPEAT_FLAGS:
                  return 0;
            case P_ID::MEASURE_NUMBER_MODE:
                  return int(MeasureNumberMode::AUTO);
            case P_ID::BREAK_MMR:
                  return false;
            case P_ID::REPEAT_COUNT:
                  return 2;
            case P_ID::USER_STRETCH:
                  return 1.0;
            case P_ID::NO_OFFSET:
                  return 0;
            case P_ID::IRREGULAR:
                  return false;
            case P_ID::SYSTEM_INITIAL_BARLINE_TYPE:
                  return int(BarLineType::NORMAL);
            default:
                  break;
            }
      return MeasureBase::getProperty(propertyId);
      }

//-------------------------------------------------------------------
//   mmRestFirst
//    this is a multi measure rest
//    returns first measure of replaced sequence of empty measures
//-------------------------------------------------------------------

Measure* Measure::mmRestFirst() const
      {
      Q_ASSERT(isMMRest());
      if (prev())
            return static_cast<Measure*>(prev()->next());
      return score()->firstMeasure();
      }

//-------------------------------------------------------------------
//   mmRestLast
//    this is a multi measure rest
//    returns last measure of replaced sequence of empty measures
//-------------------------------------------------------------------

Measure* Measure::mmRestLast() const
      {
      Q_ASSERT(isMMRest());
      if (next())
            return static_cast<Measure*>(next()->prev());
      return score()->lastMeasure();
      }

//---------------------------------------------------------
//   mmRest1
//    return the multi measure rest this measure is covered
//    by
//---------------------------------------------------------

const Measure* Measure::mmRest1() const
      {
      if (_mmRest)
            return _mmRest;
      if (_mmRestCount != -1)
            // return const_cast<Measure*>(this);
            return this;
      const Measure* m = this;
      while (m && !m->_mmRest)
            m = m->prevMeasure();
      if (m)
            return const_cast<Measure*>(m->_mmRest);
      return 0;
      }

//-------------------------------------------------------------------
//   userStretch
//-------------------------------------------------------------------

qreal Measure::userStretch() const
      {
      return (score()->layoutMode() == LayoutMode::FLOAT ? 1.0 : _userStretch);
      }

Element* Measure::nextElementStaff(int staff)
      {
      Segment* firstSeg = segments()->first();
      if (firstSeg)
            return firstSeg->firstElement(staff);
      return score()->firstElement();
      }

Element* Measure::prevElementStaff(int staff)
      {
      Measure* prevM = prevMeasureMM();
      if (prevM) {
            Segment* seg = prevM->last();
            if (seg)
                  seg->lastElement(staff);
            }
      return score()->lastElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Measure::accessibleInfo()
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(QString::number(no() + 1));
      }

}

