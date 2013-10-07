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

#include <fenv.h>
#include "page.h"
#include "sig.h"
#include "key.h"
#include "clef.h"
#include "score.h"
#include "segment.h"
#include "text.h"
#include "staff.h"
#include "style.h"
#include "timesig.h"
#include "chord.h"
#include "note.h"
#include "slur.h"
#include "tie.h"
#include "keysig.h"
#include "barline.h"
#include "repeat.h"
#include "box.h"
#include "system.h"
#include "part.h"
#include "utils.h"
#include "measure.h"
#include "volta.h"
#include "beam.h"
#include "tuplet.h"
#include "sym.h"
#include "fingering.h"
#include "stem.h"
#include "layoutbreak.h"
#include "mscore.h"
#include "accidental.h"
#include "undo.h"
#include "layout.h"
#include "lyrics.h"
#include "harmony.h"
#include "ottava.h"

namespace Ms {

//---------------------------------------------------------
//   rebuildBspTree
//---------------------------------------------------------

void Score::rebuildBspTree()
      {
      int n = _pages.size();
      for (int i = 0; i < n; ++i)
            _pages.at(i)->rebuildBspTree();
      }

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

ChordRest* Score::searchNote(int tick, int track) const
      {
      ChordRest* ipe = 0;
      Segment::SegmentTypes st = Segment::SegChordRest;
      for (Segment* segment = firstSegment(st); segment; segment = segment->next(st)) {
            ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
            if (!cr)
                  continue;
            if (cr->tick() == tick)
                  return cr;
            if (cr->tick() >  tick)
                  return ipe ? ipe : cr;
            ipe = cr;
            }
      return 0;
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      qreal x;
      };

//---------------------------------------------------------
//   layoutChords1
//    - calculate displaced note heads
//---------------------------------------------------------

void Score::layoutChords1(Segment* segment, int staffIdx)
      {
      Staff* staff = Score::staff(staffIdx);

      // if (staff->isDrumStaff() || staff->isTabStaff())
      if (staff->isTabStaff())
            return;

      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      int voices     = 0;
      QList<Note*> notes;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (e && (e->type() == Element::CHORD)) {
                  ++voices;
                  Chord* chord = static_cast<Chord*>(e);
                  for (Chord* c : chord->graceNotes())
                        layoutChords1(c->notes(), 1, staff, 0);
                  notes.append(chord->notes());
                  }
            }
      if (notes.isEmpty())
            return;
      layoutChords1(notes, voices, staff, segment);
      }

void Score::layoutChords1(QList<Note*>& notes, int voices, Staff* staff, Segment* segment)
      {
      int startIdx, endIdx, incIdx;

      if (notes[0]->chord()->up() || (voices > 1)) {
            startIdx = 0;
            incIdx   = 1;
            endIdx   = notes.size();
            for (int i = 0; i < endIdx-1; ++i) {
                  if ((notes[i]->line() == notes[i+1]->line())
                     && (notes[i]->track() != notes[i+1]->track())
                     && (!notes[i]->chord()->up() && notes[i+1]->chord()->up())
                     ) {
                        Note* n = notes[i];
                        notes[i] = notes[i+1];
                        notes[i+1] = n;
                        }
                  }
            }
      else {
            startIdx = notes.size() - 1;
            incIdx   = -1;
            endIdx   = -1;
            }

      int ll        = 1000;      // line distance to previous note head
      bool isLeft   = notes[startIdx]->chord()->up();
      int move1     = notes[startIdx]->chord()->staffMove();
      bool mirror   = false;
      int lastHeadGroup  = -1;
      int lastHeadType  = -1;

      for (int idx = startIdx; idx != endIdx; idx += incIdx) {
            Note* note    = notes[idx];
            Chord* chord  = note->chord();
            int move      = chord->staffMove();
            int line      = note->line();
            int ticks     = chord->actualTicks();
            int headGroup = note->headGroup();
            int headType  =  (note->headType() == Note::HEAD_AUTO) ? note->chord()->durationType().headType() : note->headType() - 1;

            bool conflict = (qAbs(ll - line) < 2) && (move1 == move);
            bool sameHead = (ll == line) && (headGroup == lastHeadGroup) && (headType == lastHeadType);
            if ((chord->up() != isLeft) || conflict)
                  isLeft = !isLeft;
            bool nmirror  = (chord->up() != isLeft) && !sameHead;

            note->setHidden(false);
            chord->rxpos() = 0.0;

            if (conflict && (nmirror == mirror) && idx) {
                  if (sameHead) {
                        Note* pnote = notes[idx-1];
                        if (!(pnote->parent()->isNudged() || note->parent()->isNudged())) {
                              if (ticks > pnote->chord()->actualTicks()) {
                                    pnote->setHidden(true);
                                    // TODO: pnote->setAccidentalType(ACC_NONE);
                                    note->setHidden(false);
                                    }
                              else {
                                    // TODO: note->setAccidentalType(ACC_NONE);
                                    note->setHidden(true);
                                    }
                              }
                        else
                              note->setHidden(false);
                        }
                  else {
                        qreal x = note->headWidth() - note->point(styleS(ST_stemWidth));
                        if ((line > ll) || !chord->up())
                              note->chord()->rxpos() = x;
                        else
                              notes[idx-incIdx]->chord()->rxpos() = x;
                        note->rxpos() = 0.0;
                        }
                  }
            if (note->userMirror() == MScore::DH_AUTO) {
                  mirror = nmirror;
                  }
            else {
                  mirror = note->chord()->up();
                  if (note->userMirror() == MScore::DH_LEFT)
                        mirror = !mirror;
                  }
            note->setMirror(mirror);
            move1         = move;
            ll            = line;
            lastHeadGroup = headGroup;
            lastHeadType  = headType;
            }

      //---------------------------------------------------
      //    layout accidentals
      //    find column for dots
      //---------------------------------------------------

      std::vector<AcEl> aclist;

      qreal stepDistance = staff->spatium() * .5;
      int stepOffset     = staff->staffType()->stepOffset();

      qreal lx       = 10000.0;     // leftmost note head position
      qreal dotPosX  = 0.0;

      int nNotes = notes.size();
      for (int i = nNotes-1; i >= 0; --i) {
            Note* note     = notes[i];
            Accidental* ac = note->accidental();
            if (ac) {
                  ac->layout();
                  AcEl acel;
                  acel.note = note;
                  acel.x    = 0.0;
                  aclist.push_back(acel);
                  }
            qreal hw = note->headWidth();

            Chord* chord = note->chord();
            bool _up     = chord->up();
            qreal stemX  = chord->stemPosX();

            qreal stemWidth5;
            if (chord->stem()) {
                  stemWidth5 = chord->stem()->lineWidth() * .5;
                  chord->stem()->rxpos() = _up ? stemX - stemWidth5 : stemWidth5;
                  }
            else
                  stemWidth5 = 0.0;

            qreal x;
            if (note->mirror()) {
                  if (_up)
                        x = stemX - stemWidth5 * 2;
                  else
                        x = stemX - hw + stemWidth5 * 2;
                  }
            else {
                  int ht = chord->durationType().headType();
                  if (ht == Note::HEAD_WHOLE || ht == Note::HEAD_BREVIS) {
                        // center whole note
                        qreal xd = (hw - noteHeadWidth() * chord->mag()) * .5;
                        if (_up)
                              x = stemX - hw + xd;
                        else
                              x = -xd;

                        if(chord->stem())
                              chord->stem()->rxpos() +=  (_up ?  xd  : -xd);
                        }
                  else {
                        if (_up)
                              x = stemX - hw;
                        else
                              x = 0.0;
                        }
                  }
            note->rypos()  = (note->line() + stepOffset) * stepDistance;
            note->rxpos()  = x;
            if (x < lx)
                  lx = x;

            //if (chord->stem())
            //      chord->stem()->rxpos() = _up ? x + hw - stemWidth5 : x + stemWidth5;

            qreal xx = x + hw + note->chord()->pos().x();
            if (xx > dotPosX)
                  dotPosX = xx;
            }

      if (segment)
            segment->setDotPosX(staff->idx(), dotPosX);

      int nAcc = aclist.size();
      if (nAcc == 0)
            return;
      qreal pd  = point(styleS(ST_accidentalDistance));
      qreal pnd = point(styleS(ST_accidentalNoteDistance));

      //
      // layout top accidental
      //
      Note* note      = aclist[0].note;
      Accidental* acc = note->accidental();
      qreal x         = -pnd * acc->mag() - acc->width() - acc->bbox().x();
      aclist[0].x     = x;

      //
      // layout bottom accidental
      //
      if (nAcc > 1) {
            note   = aclist[nAcc-1].note;         // last note
            acc    = note->accidental();
            int l1 = aclist[0].note->line();
            int l2 = note->line();

            int st1   = aclist[0].note->accidental()->accidentalType();
            int ldiff = st1 == Accidental::ACC_FLAT ? 4 : 5;

            if (qAbs(l1-l2) > ldiff) {
                  aclist[nAcc-1].x = -pnd * acc->mag() - acc->width() - acc->bbox().x();
                  }
            else {
                  int st2   = acc->accidentalType();
                  if ((st1 == Accidental::ACC_FLAT) && (st2 == Accidental::ACC_FLAT) && (qAbs(l1-l2) > 2))
                        aclist[nAcc-1].x = aclist[0].x - acc->width() * .5;
                  else
                        aclist[nAcc-1].x = aclist[0].x - acc->width() - pd;
                  }
            }

      //
      // layout middle accidentals
      //
      if (nAcc > 2) {
            int n = nAcc - 1;
            for (int i = 1; i < n; ++i) {
                  note = aclist[i].note;
                  acc  = note->accidental();
                  int l1 = aclist[i-1].note->line();
                  int l2 = note->line();
                  int l3 = aclist[n].note->line();
                  qreal x = 0.0;

                  int st1 = aclist[i-1].note->accidental()->accidentalType();
                  int st2 = acc->accidentalType();

                  int ldiff = st1 == Accidental::ACC_FLAT ? 4 : 5;
                  if (qAbs(l1-l2) <= ldiff) {   // overlap accidental above
                        if ((st1 == Accidental::ACC_FLAT) && (st2 == Accidental::ACC_FLAT) && (qAbs(l1-l2) > 2))
                              x = aclist[i-1].x + acc->width() * .5;    // undercut flats
                        else
                              x = aclist[i-1].x;
                        }

                  ldiff = acc->accidentalType() == Accidental::ACC_FLAT ? 4 : 5;
                  if (qAbs(l2-l3) <= ldiff) {       // overlap accidental below
                        if (aclist[n].x < x)
                              x = aclist[n].x;
                        }
                  if (x == 0.0 || x > acc->width())
                        x = -pnd * acc->mag() - acc->bbox().x();
                  else
                        x -= pd * acc->mag();   // accidental distance
                  aclist[i].x = x - acc->width() - acc->bbox().x();
                  }
            }

      for (const AcEl& e : aclist) {
            Note* note = e.note;
            qreal x    = e.x + lx - note->x();
            note->accidental()->setPos(x, 0);
            note->accidental()->adjustReadPos();
            }
      }

//---------------------------------------------------------
//   beamGraceNotes
//---------------------------------------------------------

void Score::beamGraceNotes(Chord* mainNote)
      {
      // TODO: support beam mode
      bool beam = false;
      foreach (ChordRest* cr, mainNote->graceNotes()) {
            if (cr->durationType().hooks() > 0)
                  beam = true;
            cr->removeDeleteBeam();
            }
      if (mainNote->graceNotes().size() < 2 || !beam)
            return;
      Beam* b = new Beam(this);
      b->setTrack(mainNote->track());
      b->setGenerated(true);
      foreach (ChordRest* cr, mainNote->graceNotes())
            b->add(cr);

      b->layoutGraceNotes();
      }

//---------------------------------------------------------
//   layoutStage2
//    auto - beamer
//---------------------------------------------------------

void Score::layoutStage2()
      {
      int tracks = nstaves() * VOICES;
      bool crossMeasure = styleB(ST_crossMeasureValues);

      for (int track = 0; track < tracks; ++track) {
            ChordRest* a1    = 0;      // start of (potential) beam
            Beam* beam       = 0;      // current beam
            Measure* measure = 0;

            BeamMode bm = BeamMode::AUTO;
            Segment::SegmentTypes st = Segment::SegChordRest;
            for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st)) {
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                  if (cr == 0)
                        continue;
                  if (cr->type() == Element::CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        beamGraceNotes(chord);

                        // set up for cross-measure values as soon as possible
                        // to have all computations (stems, hooks, ...) consistent with it
                        if (!chord->isGrace())
                              chord->crossMeasureSetup(crossMeasure);
                        }

                  bm = Groups::endBeam(cr);

                  // if chord has hooks and is 2nd element of a cross-measure value
                  // set beam mode to NONE (do not combine with following chord beam/hook, if any)

                  if (cr->durationType().hooks() > 0 && cr->crossMeasure() == CROSSMEASURE_SECOND)
                        bm = BeamMode::NONE;
                  if (cr->measure() != measure) {
                        if (measure && !beamModeMid(bm)) {
                              if (beam) {
                                    beam->layout1();
                                    beam = 0;
                                    }
                              else if (a1) {
                                    a1->removeDeleteBeam();
                                    a1 = 0;
                                    }
                              }
                        measure = cr->measure();
                        if (!beamModeMid(bm)) {
                              a1      = 0;
                              beam    = 0;
                              }
                        }
                  if ((cr->durationType().type() <= TDuration::V_QUARTER) || (bm == BeamMode::NONE)) {
                        if (beam) {
                              beam->layout1();
                              beam = 0;
                              }
                        if (a1) {
                              a1->removeDeleteBeam();
                              a1 = 0;
                              }
                        cr->removeDeleteBeam();
                        continue;
                        }

                  if (beam) {
                        bool beamEnd = bm == BeamMode::BEGIN;
                        if (!beamEnd) {
                              cr->removeDeleteBeam(true);
                              beam->add(cr);
                              cr = 0;
                              beamEnd = (bm == BeamMode::END);
                              }
                        if (beamEnd) {
                              beam->layout1();
                              beam = 0;
                              }
                        }
                  if (!cr)
                        continue;

                  if (a1 == 0)
                        a1 = cr;
                  else {
                        if (!beamModeMid(bm)
                             &&
                             (bm == BeamMode::BEGIN
                             || (a1->segment()->segmentType() != cr->segment()->segmentType())
                             || (a1->tick() + a1->actualTicks() < cr->tick())
                             )
                           ) {
                              a1->removeDeleteBeam();
                              a1 = cr;
                              }
                        else {
                              beam = a1->beam();
                              if (beam == 0 || beam->elements().front() != a1) {
                                    beam = new Beam(this);
                                    beam->setGenerated(true);
                                    beam->setTrack(track);
                                    a1->removeDeleteBeam(true);
                                    beam->add(a1);
                                    }
                              cr->removeDeleteBeam(true);
                              beam->add(cr);
                              a1 = 0;
                              }
                        }
                  }
            if (beam)
                  beam->layout1();
            else if (a1)
                  a1->removeDeleteBeam();
            }
      }

//---------------------------------------------------------
//   layoutStage3
//---------------------------------------------------------

void Score::layoutStage3()
      {
      Segment::SegmentTypes st = Segment::SegChordRest;
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st)) {
                  layoutChords1(segment, staffIdx);
                  }
            }
      }

//---------------------------------------------------------
//   layout
//    - measures are akkumulated into systems
//    - systems are akkumulated into pages
//   already existent systems and pages are reused
//---------------------------------------------------------

void Score::doLayout()
      {
//      printf("======doLayout\n");

      int idx = styleSt(ST_MusicalSymbolFont) == "Gonville" ? 1 : 0;

      initSymbols(idx);

      if (idx != _symIdx) {
            _symIdx = idx;
            _noteHeadWidth  = symbols[_symIdx][quartheadSym].width(spatium() / (MScore::DPI * SPATIUM20));
            }

      if (layoutFlags & LAYOUT_FIX_TICKS)
            fixTicks();
      if (layoutFlags & LAYOUT_FIX_PITCH_VELO)
            updateVelo();
      if (layoutFlags & LAYOUT_PLAY_EVENTS)
            createPlayEvents();

      int measureNo = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            Measure* measure = static_cast<Measure*>(m);
            measureNo += measure->noOffset();
            measure->setNo(measureNo);
            if (measure->sectionBreak() && measure->sectionBreak()->startWithMeasureOne())
                  measureNo = 0;
            else if (measure->irregular())      // dont count measure
                  ;
            else
                  ++measureNo;
            measure->setBreakMMRest(false);
            }

      for (MeasureBase* m = first(); m; m = m->next())
            m->layout0();

      layoutFlags = 0;

      int nstaves = _staves.size();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* st = _staves[staffIdx];
            if (!st->updateKeymap())
                  continue;
            int track = staffIdx * VOICES;
            st->keymap()->clear();
            KeySig* key1 = 0;
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        Element* e = s->element(track);
                        if (e == 0 || e->generated())
                              continue;
                        if ((s->segmentType() == Segment::SegKeySig)) {
                              KeySig* ks = static_cast<KeySig*>(e);
                              int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                              ks->setOldSig(naturals);
                              st->setKey(s->tick(), ks->keySigEvent());
                              key1 = ks;
                              }
                        }
                  if (m->sectionBreak() && (_layoutMode != LayoutFloat))
                        key1 = 0;
                  }
            st->setUpdateKeymap(false);
            }
      if (_staves.isEmpty() || first() == 0) {
            // score is empty
            qDeleteAll(_pages);
            _pages.clear();

            Page* page = addPage();
            page->layout();
            page->setNo(0);
            page->setPos(0.0, 0.0);
            page->rebuildBspTree();
            return;
            }

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->layoutStage1();
      if (styleB(ST_createMultiMeasureRests))
            createMMRests();

      layoutStage2();   // beam notes, finally decide if chord is up/down
      layoutStage3();   // compute note head horizontal positions

      if (layoutMode() == LayoutLine)
            layoutLinear();
      else
            layoutSystems();  // create list of systems

      //---------------------------------------------------
      //   place Spanner & beams
      //---------------------------------------------------

      int tracks = nstaves * VOICES;
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = firstSegmentMM(); segment; segment = segment->next1MM()) {
                  if (track == tracks-1) {
                        for (Element* e : segment->annotations())
                              e->layout();
                        }
                  Element* e = segment->element(track);
                  if (!e)
                        continue;
                  if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr->beam() && cr->beam()->elements().front() == cr)
                              cr->beam()->layout();

                        if (cr->type() == Element::CHORD) {
                              Chord* c = static_cast<Chord*>(cr);
                              for (Chord* cc : c->graceNotes()) {
                                    if (cc->beam() && cc->beam()->elements().front() == cc)
                                          cc->beam()->layout();
                                    for (Element* e : cc->el()) {
                                          if (e->type() == Element::SLUR)
                                                e->layout();
                                          }
                                    }
                              c->layoutStem();
                              c->layoutArpeggio2();
                              for (Note* n : c->notes()) {
                                    Tie* tie = n->tieFor();
                                    if (tie)
                                          tie->layout();
                                    for (Spanner* sp : n->spannerFor())
                                          sp->layout();
                                    }
                              }
                        cr->layoutArticulations();
                        }
                  else if (e->type() == Element::BAR_LINE)
                        e->layout();
                  }
            }

      for (const std::pair<int,Spanner*>& s : _spanner.map()) {
            Spanner* sp = s.second;
            if (sp->type() == Element::OTTAVA && sp->tick2() == -1) {
                  sp->setTick2(lastMeasure()->endTick());
                  sp->staff()->updateOttava(static_cast<Ottava*>(sp));
                  }
            // 1.3 scores can have ties in this list
            if (sp->type() != Element::TIE) {
                  if (sp->tick() == -1) {
                        qDebug("bad spanner id %d %s %d - %d", sp->id(), sp->name(), sp->tick(), sp->tick2());
                        }
                  else
                        sp->layout();
                  }
            }

      if (layoutMode() != LayoutLine) {
            layoutSystems2();
            layoutPages();    // create list of pages
            }

      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM())
            m->layout2();

      rebuildBspTree();

      int n = viewer.size();
      for (int i = 0; i < n; ++i) {
            viewer.at(i)->layoutChanged();
            viewer.at(i)->updateLoopCursors();
      }

      _layoutAll = false;
      }

//---------------------------------------------------------
//   layoutSpanner
//    called after dragging a staff
//---------------------------------------------------------

void Score::layoutSpanner()
      {
      int tracks = ntracks();
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                  if (track == tracks-1) {
                        int n = segment->annotations().size();
                        for (int i = 0; i < n; ++i)
                              segment->annotations().at(i)->layout();
                        }
                  Element* e = segment->element(track);
                  if (!e)
                        continue;
                  if (e->isChordRest()) {
                        Chord* c = static_cast<Chord*>(e);
                        if (c->type() == Element::CHORD) {
                              for (Chord* cc : c->graceNotes()) {
                                    for (Element* e : cc->el()) {
                                          if (e->type() == Element::SLUR)
                                                e->layout();
                                          }
                                    }
                              c->layoutStem();
                              for (Note* n : c->notes()) {
                                    Tie* tie = n->tieFor();
                                    if (tie)
                                          tie->layout();
                                    for (Spanner* sp : n->spannerFor())
                                          sp->layout();
                                    }
                              }
                        }
                  }
            }
      rebuildBspTree();
      }

//-------------------------------------------------------------------
//   addSystemHeader
///   Add elements to make this measure suitable as the first measure
///   of a system.
//    The system header can contain a Clef, a KeySig and a
//    RepeatBarLine.
//-------------------------------------------------------------------

void Score::addSystemHeader(Measure* m, bool isFirstSystem)
      {
      if (undoRedo())   // no change possible in this state
            return;

      int tick = m->tick();
      int i    = 0;
      foreach (Staff* staff, _staves) {
            if (!m->system()->staff(i)->show()) {
                  ++i;
                  continue;
                  }

            KeySig* keysig = 0;
            Clef*   clef   = 0;
            int strack     = i * VOICES;

            // we assume that keysigs and clefs are only in the first
            // track (voice 0) of a staff

            const KeySigEvent& keyIdx = staff->key(tick);
            const KeySigEvent& oKeySigBefore = staff->key(tick-1);

            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  // search only up to the first ChordRest
                  if (seg->segmentType() == Segment::SegChordRest)
                        break;
                  Element* el = seg->element(strack);
                  if (!el)
                        continue;
                  switch (el->type()) {
                        case Element::KEYSIG:
                              keysig = static_cast<KeySig*>(el);
                              keysig->changeKeySigEvent(keyIdx);
                              if (!keysig->isCustom() && oKeySigBefore.accidentalType() == keysig->keySignature())
                                    keysig->setOldSig(0);
                              break;
                        case Element::CLEF:
                              clef = static_cast<Clef*>(el);
                              clef->setSmall(false);
                              break;
                        default:
                              break;
                        }
                  }
            bool needKeysig = /* !staff->isTabStaff()       // keep key sigs in TABs: TABs themselves should hide them
               && */ keyIdx.isValid()
               && (isFirstSystem || styleB(ST_genKeysig));

            if (needKeysig && !keysig) {
                  //
                  // create missing key signature
                  //
                  keysig = keySigFactory(keyIdx);
                  if (keysig) {
                        // if signature is not custom or prev. signature has same accid. as
                        // this one, reset naturals
                        if (!keysig->isCustom() && oKeySigBefore.accidentalType() == keysig->keySignature())
                              keysig->setOldSig(0);
                        keysig->setTrack(i * VOICES);
                        keysig->setGenerated(true);
                        Segment* seg = m->undoGetSegment(Segment::SegKeySig, tick);
                        keysig->setParent(seg);
                        keysig->layout();
                        undoAddElement(keysig);
                        }
                  }
            else if (!needKeysig && keysig)
                  undoRemoveElement(keysig);
            else if (keysig && keysig->keySigEvent() != keyIdx)
                  undo(new ChangeKeySig(keysig, keyIdx, keysig->showCourtesy(), keysig->showNaturals()));

            bool needClef = isFirstSystem || styleB(ST_genClef);
            if (needClef) {
                  if (!clef) {
                        //
                        // create missing clef
                        //
                        clef = new Clef(this);
                        clef->setTrack(i * VOICES);
                        clef->setSmall(false);
                        clef->setGenerated(staff->clef(tick) == staff->clef(tick-1));

                        Segment* s = m->undoGetSegment(Segment::SegClef, tick);
                        clef->setParent(s);
                        clef->layout();
                        clef->setClefType(staff->clefTypeList(tick));  // set before add !
                        undoAddElement(clef);
                        }
                  else if (clef->generated()) {
                        ClefTypeList cl = staff->clefTypeList(tick);
                        if (cl != clef->clefTypeList())
                              undo(new ChangeClefType(clef, cl._concertClef, cl._transposingClef));
                        }
                  }
            else {
                  if (clef && clef->generated())
                        undoRemoveElement(clef);
                  }
            ++i;
            }
      m->setStartRepeatBarLine(m->repeatFlags() & RepeatStart);
      }

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* Score::getNextSystem(bool isFirstSystem, bool isVbox)
      {
      System* system;
      if (curSystem >= _systems.size()) {
            system = new System(this);
            _systems.append(system);
            }
      else {
            system = _systems[curSystem];
            system->clear();   // remove measures from system
            }
      system->setFirstSystem(isFirstSystem);
      system->setVbox(isVbox);
      if (!isVbox) {
            int nstaves = Score::nstaves();
            for (int i = system->staves()->size(); i < nstaves; ++i)
                  system->insertStaff(i);
            int dn = system->staves()->size() - nstaves;
            for (int i = 0; i < dn; ++i)
                  system->removeStaff(system->staves()->size()-1);
            }
      return system;
      }

//---------------------------------------------------------
//   createMMRests
//---------------------------------------------------------

void Score::createMMRests()
      {
      //
      //  create mm rest measures
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            Measure* nm = m;
            Measure* lm = nm;
            int n = 0;
            Fraction len;
            while (nm->isEmpty()) {
                  m->setMMRestCount(0);
                  MeasureBase* mb = _showVBox ? nm->next() : nm->nextMeasure();
                  if (nm->breakMultiMeasureRest() && n)
                        break;
                  ++n;
                  len += nm->len();
                  lm = nm;
                  nm = static_cast<Measure*>(mb);
                  if (!mb || (mb->type() != Element::MEASURE))
                        break;
                  }

            if (n >= styleI(ST_minEmptyMeasures)) {
                  //
                  // create a multi measure rest from m to lm (inclusive)
                  // attach the measure to m
                  //
                  for (Measure* mm = m->nextMeasure(); mm; mm = mm->nextMeasure()) {
                        mm->setMMRestCount(-1);
                        if (mm == lm)
                              break;
                        }
                  Measure* mmr;
                  if (m->mmRest()) {
                        mmr = m->mmRest();
                        if (m->len() != len) {
                              Segment* s = mmr->findSegment(Segment::SegEndBarLine, mmr->endTick());
                              mmr->setLen(len);
                              if (s)
                                    s->setTick(mmr->endTick());
                              }
                        }
                  else {
                        mmr = new Measure(this);
                        mmr->setLen(len);
                        }

                  mmr->setMMRestCount(n);
                  mmr->setTick(m->tick());
                  mmr->setNo(m->no());
                  mmr->setPageBreak(lm->pageBreak());
                  mmr->setLineBreak(lm->lineBreak());
                  mmr->setSectionBreak(lm->sectionBreak());
                  mmr->setEndBarLineType(lm->endBarLineType(), false, lm->endBarLineVisible(), lm->endBarLineColor());
                  qDeleteAll(*mmr->el());
                  mmr->el()->clear();
                  for (Element* e : *lm->el())
                        mmr->add(e->clone());

                  Segment* s = mmr->getSegment(Segment::SegChordRest, m->tick());
                  for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                        int track = staffIdx * VOICES;
                        if (s->element(track) == 0) {
                              Rest* r = new Rest(this);
                              r->setDurationType(TDuration::V_MEASURE);
                              r->setTrack(track);
                              s->add(r);
                              }
                        }
                  //
                  // check for clefs
                  //
                  Segment* cs = lm->findSegment(Segment::SegClef, lm->endTick());
                  if (cs) {
                        Segment* ns = mmr->getSegment(Segment::SegClef, lm->endTick());
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Clef* clef = static_cast<Clef*>(cs->element(track));
                              if (clef) {
                                    if (ns->element(track) == 0)
                                          ns->add(clef->clone());
                                    else {
                                          //TODO: check if same clef
                                          }
                                    }
                              }
                        }
                  //
                  // check for time signature
                  //
                  cs = m->findSegment(Segment::SegTimeSig, m->tick());
                  if (cs) {
                        Segment* ns = mmr->getSegment(Segment::SegTimeSig, m->tick());
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              TimeSig* ts = static_cast<TimeSig*>(cs->element(track));
                              if (ts) {
                                    if (ns->element(track) == 0)
                                          ns->add(ts->clone());
                                    else {
                                          //TODO: check if same time signature
                                          }
                                    }
                              }
                        }
                  //
                  // check for key signature
                  //
                  cs = m->findSegment(Segment::SegKeySig, m->tick());
                  if (cs) {
                        Segment* ns = mmr->getSegment(Segment::SegKeySig, m->tick());
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              KeySig* ts = static_cast<KeySig*>(cs->element(track));
                              if (ts) {
                                    if (ns->element(track) == 0)
                                          ns->add(ts->clone());
                                    else {
                                          //TODO: check if same key signature
                                          }
                                    }
                              }
                        }
                  mmr->setNext(nm);
                  mmr->setPrev(m->prev());
                  m->setMMRest(mmr);
                  m = lm;
                  }
            else if (m->mmRest()) {
                  delete m->mmRest();
                  m->setMMRest(0);
                  }
            }
      }

//---------------------------------------------------------
//   cautionaryWidth
//    Compute the width difference of actual measure m
//    and the width of m if it were the last measure in a
//    staff. The reason for any difference are courtesy
//    time signatures and key signatures.
//---------------------------------------------------------

qreal Score::cautionaryWidth(Measure* m)
      {
      qreal w = 0.0;
      if (m == 0)
            return w;
      Measure* nm = m ? m->nextMeasure() : 0;
      if (nm == 0 || (m->sectionBreak() && _layoutMode != LayoutFloat))
            return w;

      int tick = m->tick() + m->ticks();

      // locate a time sig. in the next measure and, if found,
      // check if it has caut. sig. turned off

      Segment* ns       = nm->findSegment(Segment::SegTimeSig, tick);
      TimeSig* ts       = 0;
      bool showCourtesy = styleB(ST_genCourtesyTimesig) && ns;
      if (showCourtesy) {
            ts = static_cast<TimeSig*>(ns->element(0));
            if (ts && !ts->showCourtesySig())
                  showCourtesy = false;     // this key change has court. sig turned off
            }
      Segment* s = m->findSegment(Segment::SegTimeSigAnnounce, tick);

      if (showCourtesy && !s)
            w += ts->space().width();
      else if (!showCourtesy && s && s->element(0))
            w -= static_cast<TimeSig*>(s->element(0))->space().width();

      // courtesy key signatures
      qreal wwMin = 0.0;
      qreal wwMax = 0.0;
      int n = _staves.size();
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            int track        = staffIdx * VOICES;
            ns               = nm->findSegment(Segment::SegKeySig, tick);
            KeySig* ks       = 0;

            showCourtesy = styleB(ST_genCourtesyKeysig) && ns;
            if (showCourtesy) {
                  ks = static_cast<KeySig*>(ns->element(track));
                  if (ks && !ks->showCourtesy())
                        showCourtesy = false;
                  }
            Segment* s = m->findSegment(Segment::SegKeySigAnnounce, tick);

            if (showCourtesy && !s && ks)
                  wwMax = qMax(wwMax, ks->space().width());
            else if (!showCourtesy && s && s->element(track))
                  wwMin = qMin(wwMin, -static_cast<KeySig*>(s->element(track))->space().width());
            }
      if (wwMax > 0.0)
            w += wwMax;
      else
            w += wwMin;
      return w;
      }

//---------------------------------------------------------
//   layoutSystem
//    return true if line continues
//---------------------------------------------------------

bool Score::layoutSystem(qreal& minWidth, qreal w, bool isFirstSystem, bool longName)
      {
      if (undoRedo())   // no change possible in this state
            return layoutSystem1(minWidth, isFirstSystem, longName);

      System* system = getNextSystem(isFirstSystem, false);

      qreal xo = 0;
      if (curMeasure->type() == Element::HBOX)
            xo = point(static_cast<Box*>(curMeasure)->boxWidth());

      system->setInstrumentNames(longName);
      system->layout(xo);

      qreal minMeasureWidth = point(styleS(ST_minMeasureWidth));
      minWidth              = system->leftMargin();
      qreal systemWidth     = w;
      bool continueFlag     = false;
      bool isFirstMeasure   = true;
      Measure* firstMeasure = 0;
      Measure* lastMeasure  = 0;

      qreal measureSpacing = styleD(ST_measureSpacing);

      for (; curMeasure;) {
            MeasureBase* nextMeasure;
            if (curMeasure->type() == Element::MEASURE && !_showVBox)
                  nextMeasure = curMeasure->nextMeasureMM();
            else
                  nextMeasure = curMeasure->nextMM();

            System* oldSystem = curMeasure->system();
            curMeasure->setSystem(system);
            qreal ww = 0.0;

            qreal cautionaryW = 0.0;

            if (curMeasure->type() == Element::HBOX) {
                  ww = point(static_cast<Box*>(curMeasure)->boxWidth());
                  if (!isFirstMeasure) {
                        // try to put another system on current row
                        // if not a line break
                        switch(_layoutMode) {
                              case LayoutFloat:
                                    break;
                              case LayoutLine:
                              case LayoutPage:
                              case LayoutSystem:
                                    continueFlag = !(curMeasure->lineBreak() || curMeasure->pageBreak());
                                    break;
                              }
                        }
                  }
            else if (curMeasure->type() == Element::MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  m->createEndBarLines();       // TODO: type not set right here
                  if (isFirstMeasure) {
                        firstMeasure = m;
                        addSystemHeader(m, isFirstSystem);
                        ww = m->minWidth2();
                        }
                  else
                        ww = m->minWidth1();

                  Segment* s = m->last();
                  if ((s->segmentType() == Segment::SegEndBarLine) && s->element(0)) {
                        BarLine*    bl = static_cast<BarLine*>(s->element(0));
                        BarLineType ot = bl->barLineType();
                        BarLineType nt = m->endBarLineType();

                        if (m->repeatFlags() & RepeatEnd)
                              nt = END_REPEAT;
                        else {
                              Measure* nm = m->nextMeasureMM();
                              if (nm && (nm->repeatFlags() & RepeatStart))
                                    nt = START_REPEAT;
                              }
                        if (ot != nt) {
                              qreal mag = bl->magS();
                              ww += BarLine::layoutWidth(this, nt, mag)
                                    - BarLine::layoutWidth(this, ot, mag);
                              }
                        }
                  qreal stretch = m->userStretch() * measureSpacing;
                  cautionaryW   = 0.0; // TODO: cautionaryWidth(m) * stretch;
                  ww           *= stretch;

                  if (ww < minMeasureWidth)
                        ww = minMeasureWidth;
                  isFirstMeasure = false;
                  }

            // collect at least one measure
            bool empty = system->measures().isEmpty();
            if (!empty && (minWidth + ww + cautionaryW  > systemWidth)) {
                  curMeasure->setSystem(oldSystem);
                  break;
                  }
            if (curMeasure->type() == Element::MEASURE)
                  lastMeasure = static_cast<Measure*>(curMeasure);

            system->measures().append(curMeasure);

            Element::ElementType nt;
            if (_showVBox)
                  nt = curMeasure->nextMM() ? curMeasure->nextMM()->type() : Element::INVALID;
            else
                  nt = curMeasure->nextMeasureMM() ? curMeasure->nextMeasureMM()->type() : Element::INVALID;
            int n = styleI(ST_FixMeasureNumbers);
            bool pbreak;
            switch (_layoutMode) {
                  case LayoutPage:
                  case LayoutSystem:
                        pbreak = curMeasure->pageBreak() || curMeasure->lineBreak();
                        break;
                  case LayoutFloat:
                  case LayoutLine:
                        pbreak = false;
                        break;
                  }
            if ((n && system->measures().size() >= n)
               || continueFlag
               || pbreak
               || (nt == Element::VBOX || nt == Element::TBOX || nt == Element::FBOX)
               ) {
                  if (_layoutMode != LayoutSystem)
                        system->setPageBreak(curMeasure->pageBreak());
                  curMeasure = nextMeasure;
                  break;
                  }
            curMeasure = nextMeasure;
            if (minWidth + minMeasureWidth > systemWidth)
                  break;      // next measure will not fit

            minWidth += ww;
            }

      //
      // remember line breaks in list of measures
      //
      int n = system->measures().size() - 1;
      if (n >= 0) {
            for (int i = 0; i < n; ++i)
                  undoChangeProperty(system->measure(i), P_BREAK_HINT, false);
            undoChangeProperty(system->measures().last(), P_BREAK_HINT, true);
            }

      if (!undoRedo() && firstMeasure && lastMeasure && firstMeasure != lastMeasure)
            removeGeneratedElements(firstMeasure, lastMeasure);

      hideEmptyStaves(system, isFirstSystem);

      return continueFlag && curMeasure;
      }


void Score::hideEmptyStaves(System* system, bool isFirstSystem)
      {
       //
      //    hide empty staves
      //
      int staves = _staves.size();
      int staffIdx = 0;
      foreach (Staff* staff, _staves) {
            SysStaff* s  = system->staff(staffIdx);
            bool oldShow = s->show();
            if (styleB(ST_hideEmptyStaves)
               && (staves > 1)
               && !(isFirstSystem && styleB(ST_dontHideStavesInFirstSystem))
               ) {
                  bool hideStaff = true;
                  foreach(MeasureBase* m, system->measures()) {
                        if (m->type() != Element::MEASURE)
                              continue;
                        Measure* measure = static_cast<Measure*>(m);
                        if (!measure->isMeasureRest(staffIdx)) {
                              hideStaff = false;
                              break;
                              }
                        }
                  // check if notes moved into this staff
                  Part* part = staff->part();
                  int n = part->nstaves();
                  if (hideStaff && (n > 1)) {
                        int idx = part->staves()->front()->idx();
                        for (int i = 0; i < part->nstaves(); ++i) {
                              int st = idx + i;

                              foreach(MeasureBase* mb, system->measures()) {
                                    if (mb->type() != Element::MEASURE)
                                          continue;
                                    Measure* m = static_cast<Measure*>(mb);
                                    for (Segment* s = m->first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
                                          for (int voice = 0; voice < VOICES; ++voice) {
                                                ChordRest* cr = static_cast<ChordRest*>(s->element(st * VOICES + voice));
                                                if (cr == 0 || cr->type() == Element::REST)
                                                      continue;
                                                int staffMove = cr->staffMove();
                                                if (staffIdx == st + staffMove) {
                                                      hideStaff = false;
                                                      break;
                                                      }
                                                }
                                          }
                                    if (!hideStaff)
                                          break;
                                    }
                              if (!hideStaff)
                                    break;
                              }
                        }
                  s->setShow(hideStaff ? false : staff->show());
                  }
            else {
                  s->setShow(true);
                  }

            if (oldShow != s->show()) {
                  foreach (MeasureBase* mb, system->measures()) {
                        if (mb->type() != Element::MEASURE)
                              continue;
                        static_cast<Measure*>(mb)->createEndBarLines();
                        }
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   layoutSystem1
//    used in undoRedo state
//    return true if line continues
//---------------------------------------------------------

bool Score::layoutSystem1(qreal& minWidth, bool isFirstSystem, bool longName)
      {
      System* system = getNextSystem(isFirstSystem, false);

      qreal xo = 0;
      if (curMeasure->type() == Element::HBOX)
            xo = point(static_cast<Box*>(curMeasure)->boxWidth());

      system->setInstrumentNames(longName);
      system->layout(xo);

      qreal minMeasureWidth = point(styleS(ST_minMeasureWidth));
      minWidth              = system->leftMargin();
      bool continueFlag     = false;
      bool isFirstMeasure   = true;

      for (; curMeasure;) {
            MeasureBase* nextMeasure;
            if (curMeasure->type() == Element::MEASURE && !_showVBox)
                  nextMeasure = curMeasure->nextMeasureMM();
            else
                  nextMeasure = curMeasure->nextMM();

            // System* oldSystem = curMeasure->system();
            curMeasure->setSystem(system);
            qreal ww = 0.0;

            if (curMeasure->type() == Element::HBOX) {
                  ww = point(static_cast<Box*>(curMeasure)->boxWidth());
                  if (!isFirstMeasure) {
                        // try to put another system on current row
                        // if not a line break
                        switch(_layoutMode) {
                              case LayoutFloat:
                                    break;
                              case LayoutLine:
                              case LayoutPage:
                              case LayoutSystem:
                                    continueFlag = !(curMeasure->lineBreak() || curMeasure->pageBreak());
                                    break;
                              }
                        }
                  }
            else if (curMeasure->type() == Element::MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  m->createEndBarLines();       // TODO: type not set right here
                  if (isFirstMeasure) {
                        addSystemHeader(m, isFirstSystem);
                        ww = m->minWidth2();
                        }
                  else
                        ww = m->minWidth1();

                  ww *= m->userStretch() * styleD(ST_measureSpacing);
                  if (ww < minMeasureWidth)
                        ww = minMeasureWidth;
                  isFirstMeasure = false;
                  }

            minWidth += ww;

            system->measures().append(curMeasure);
            Element::ElementType nt = curMeasure->next() ? curMeasure->next()->type() : Element::INVALID;
            int n = styleI(ST_FixMeasureNumbers);
            bool pbreak;
            switch (_layoutMode) {
                  case LayoutPage:
                  case LayoutSystem:
                        pbreak = curMeasure->pageBreak() || curMeasure->lineBreak();
                        break;
                  case LayoutFloat:
                  case LayoutLine:
                        pbreak = false;
                        break;
                  }
            if ((n && system->measures().size() >= n)
               || continueFlag || pbreak || (nt == Element::VBOX || nt == Element::TBOX || nt == Element::FBOX)) {
                  if (_layoutMode != LayoutSystem)
                        system->setPageBreak(curMeasure->pageBreak());
                  curMeasure = nextMeasure;
                  break;
                  }
            // do not change line break
            if (curMeasure->breakHint()) {
                  curMeasure = nextMeasure;
                  break;
                  }
            curMeasure = nextMeasure;
            }

      hideEmptyStaves(system,isFirstSystem);

      return continueFlag && curMeasure;
      }

//---------------------------------------------------------
//   removeGeneratedElements (System Header + TimeSig Announce)
//    helper function
//---------------------------------------------------------

void Score::removeGeneratedElements(Measure* sm, Measure* em)
      {
      for (Measure* m = sm; m; m = m->nextMeasureMM()) {
            //
            // remove generated elements from all measures in [sm;em]
            //    assume: generated elements are only living in voice 0
            //    - do not remove end bar lines
            //    - set size of clefs to small
            //
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  Segment::SegmentType st = seg->segmentType();
                  if (st == Segment::SegEndBarLine)
                        continue;
                  if (st == Segment::SegStartRepeatBarLine && m != sm) {
                        undoRemoveElement(seg);
                        }
                  for (int staffIdx = 0;  staffIdx < nstaves(); ++staffIdx) {
                        Element* el = seg->element(staffIdx * VOICES);
                        if (el == 0)
                              continue;

                        if (el->generated() && ((st == Segment::SegTimeSigAnnounce && m != em)
                            || (el->type() == Element::CLEF   && seg->tick() != sm->tick())
                            || (el->type() == Element::KEYSIG && seg->tick() != sm->tick())))
                              {
                              undoRemoveElement(el);
                              }
                        else if (el->type() == Element::CLEF) {
                              Clef* clef = static_cast<Clef*>(el);
                              System* s = m->system();
                              bool small = seg != m->first() || s->firstMeasure() != m;
                              if (clef->small() != small) {
                                    clef->setSmall(small);
                                    m->setDirty();
                                    }
                              //
                              // if measure is not the first in the system, the clef at
                              // measure start has to be moved to the end of the previous measure
                              //
                              if (s->firstMeasure() != m && seg->tick() == m->tick()) {
                                    undoRemoveElement(el);
                                    Measure* pm = m->prevMeasure();
                                    Segment* s = pm->undoGetSegment(Segment::SegClef, m->tick());
                                    Clef* nc = clef->clone();
                                    nc->setParent(s);
                                    undoAddElement(nc);
                                    m->setDirty();
                                    pm->setDirty();
                                    }
                              }
                        }
                  }
            if (m == em)
                  break;
            }
      }

//---------------------------------------------------------
//   addPage
//---------------------------------------------------------

Page* Score::addPage()
      {
      Page* page = new Page(this);
      page->setNo(_pages.size());
      _pages.push_back(page);
      return page;
      }

//---------------------------------------------------------
//   connectTies
///   Rebuild tie connections.
//---------------------------------------------------------

void Score::connectTies()
      {
      int tracks = nstaves() * VOICES;
      Measure* m = firstMeasure();
      if (!m)
            return;
      Segment::SegmentTypes st = Segment::SegChordRest;
      for (Segment* s = m->first(st); s; s = s->next1(st)) {
            for (int i = 0; i < tracks; ++i) {
                  Chord* c = static_cast<Chord*>(s->element(i));
                  if (c == 0 || c->type() != Element::CHORD)
                        continue;
                  for (Note* n : c->notes()) {
                        Tie* tie = n->tieFor();
                        if (!tie || tie->endNote())
                              continue;
                        Note* nnote;
                        if (_mscVersion <= 114)
                              nnote = searchTieNote114(n);
                        else
                              nnote = searchTieNote(n);
                        if (nnote == 0) {
                              qDebug("next note at %d track %d for tie not found",
                                 s->tick(), i);
                              // n->setTieFor(0);  show short bow
                              // delete tie;
                              }
                        else {
                              tie->setEndNote(nnote);
                              nnote->setTieBack(tie);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Score::add(Element* el)
      {
      switch(el->type()) {
            case Element::MEASURE:
            case Element::HBOX:
            case Element::VBOX:
            case Element::TBOX:
            case Element::FBOX:
                  measures()->add(static_cast<MeasureBase*>(el));
                  break;
            case Element::BEAM:
                  {
                  Beam* b = static_cast<Beam*>(el);
                  int n = b->elements().size();
                  for (int i = 0; i < n; ++i)
                        b->elements().at(i)->setBeam(b);
                  }
                  break;
            case Element::TEXTLINE:
                  if (static_cast<Spanner*>(el)->anchor() == Spanner::ANCHOR_NOTE)
                        break;
                  // fall through

            case Element::SLUR:
            case Element::VOLTA:
            case Element::TRILL:
            case Element::PEDAL:
            case Element::HAIRPIN:
            case Element::OTTAVA:
                  addSpanner(static_cast<Spanner*>(el));
                  break;

            default:
                  qDebug("Score::add() invalid element <%s>", el->name());
                  // abort();
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Score::remove(Element* el)
      {
      switch(el->type()) {
            case Element::MEASURE:
            case Element::HBOX:
            case Element::VBOX:
            case Element::TBOX:
            case Element::FBOX:
                  measures()->remove(static_cast<MeasureBase*>(el));
                  break;
            case Element::BEAM:
                  {
                  Beam* b = static_cast<Beam*>(el);
                  foreach(ChordRest* cr, b->elements())
                        cr->setBeam(0);
                  }
                  break;
            case Element::SLUR:
            case Element::TEXTLINE:
            case Element::VOLTA:
            case Element::TRILL:
            case Element::PEDAL:
            case Element::HAIRPIN:
            case Element::OTTAVA:
                  removeSpanner(static_cast<Spanner*>(el));
                  break;
            default:
                  qDebug("Score::remove(): invalid element %s", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   layoutFingering
//    - place numbers above a note execpt for the last
//      staff in a multi stave part (piano)
//    - does not handle chords
//---------------------------------------------------------

void Score::layoutFingering(Fingering* f)
      {
      if (f == 0)
            return;
      Note* note   = f->note();
      Chord* chord = note->chord();
      Staff* staff = chord->staff();
      Part* part   = staff->part();
      int n        = part->nstaves();
      bool below   = (n > 1) && (staff->rstaff() == n-1);

      f->layout();
      qreal x = 0.0;
      qreal y = 0.0;
      qreal headWidth = note->headWidth();
      qreal headHeight = note->headHeight();
      qreal fh = headHeight;        // TODO: fingering number height

      if (chord->notes().size() == 1) {
            x = headWidth * .5;
            if (below) {
                  // place fingering below note
                  y = fh + spatium() * .4;
                  if (chord->stem() && !chord->up()) {
                        // on stem side
                        y += chord->stem()->height();
                        x -= spatium() * .4;
                        }
                  }
            else {
                  // place fingering above note
                  y = -headHeight - spatium() * .4;
                  if (chord->stem() && chord->up()) {
                        // on stem side
                        y -= chord->stem()->height();
                        x += spatium() * .4;
                        }
                  }
            }
      f->setUserOff(QPointF(x, y));
      }

//---------------------------------------------------------
//   layoutSystemRow
//    return height in h
//---------------------------------------------------------

QList<System*> Score::layoutSystemRow(qreal rowWidth, bool isFirstSystem, bool useLongName)
      {
      bool raggedRight = MScore::layoutDebug;

      QList<System*> sl;

      qreal ww = rowWidth;
      qreal minWidth;
      for (bool a = true; a;) {
            a = layoutSystem(minWidth, ww, isFirstSystem, useLongName);
            sl.append(_systems[curSystem]);
            ++curSystem;
            ww -= minWidth;
            }
      //
      // dont stretch last system row, if minWidth is <= lastSystemFillLimit
      //
      if (curMeasure == 0 && ((minWidth / rowWidth) <= styleD(ST_lastSystemFillLimit)))
            raggedRight = true;

      //-------------------------------------------------------
      //    Round II
      //    stretch measures
      //    "nm" measures fit on this line of score
      //-------------------------------------------------------

      bool needRelayout = false;

      foreach (System* system, sl) {
            //
            //    add cautionary time/key signatures if needed
            //

            if (system->measures().isEmpty()) {
                  qDebug("system %p is empty\n", system);
                  abort();
                  }
            Measure* m = system->lastMeasure();
            bool hasCourtesyKeysig = false;
            Measure* nm = m ? m->nextMeasure() : 0;
            Segment* s;

            if (m && nm && !(m->sectionBreak() && _layoutMode != LayoutFloat)) {
                  int tick = m->tick() + m->ticks();

                  // locate a time sig. in the next measure and, if found,
                  // check if it has cout. sig. turned off
                  TimeSig* ts;
                  Segment* tss         = nm->findSegment(Segment::SegTimeSig, tick);
                  bool showCourtesySig = tss && styleB(ST_genCourtesyTimesig);
                  if (showCourtesySig) {
                        ts = static_cast<TimeSig*>(tss->element(0));
                        if (ts && !ts->showCourtesySig())
                              showCourtesySig = false;     // this key change has court. sig turned off
                        }
                  if (showCourtesySig) {
                        // if due, create a new courtesy time signature for each staff
                        s  = m->undoGetSegment(Segment::SegTimeSigAnnounce, tick);
                        int nstaves = Score::nstaves();
                        for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                              TimeSig* nts = static_cast<TimeSig*>(tss->element(track));
                              if (!nts)
                                    continue;
                              ts = static_cast<TimeSig*>(s->element(track));
                              if (ts == 0) {
                                    ts = new TimeSig(this);
                                    ts->setTrack(track);
                                    ts->setGenerated(true);
                                    ts->setParent(s);
                                    undoAddElement(ts);
                                    }
                              ts->setFrom(nts);
                              m->setDirty();
                              }
                        }
                  else {
                        // remove any existing time signatures
                        Segment* tss = m->findSegment(Segment::SegTimeSigAnnounce, tick);
                        if (tss) {
                              undoRemoveElement(tss);
                              }
                        }

                  // courtesy key signatures
                  int n = _staves.size();
                  for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                        int track = staffIdx * VOICES;
                        Staff* staff = _staves[staffIdx];
                        showCourtesySig = false;

                        KeySigEvent key1 = staff->key(tick - 1);
                        KeySigEvent key2 = staff->key(tick);
                        if (styleB(ST_genCourtesyKeysig) && (key1 != key2)) {
                              // locate a key sig. in next measure and, if found,
                              // check if it has court. sig turned off
                              s = nm->findSegment(Segment::SegKeySig, tick);
                              showCourtesySig = true;	// assume this key change has court. sig turned on
                              if (s) {
                                    KeySig* ks = static_cast<KeySig*>(s->element(track));
                                    if (ks && !ks->showCourtesy())
                                          showCourtesySig = false;     // this key change has court. sig turned off
                                    }

                              if (showCourtesySig) {
                                    hasCourtesyKeysig = true;
                                    s  = m->undoGetSegment(Segment::SegKeySigAnnounce, tick);
                                    KeySig* ks = static_cast<KeySig*>(s->element(track));
                                    KeySigEvent ksv(key2);
                                    ksv.setNaturalType(key1.accidentalType());

                                    if (!ks) {
                                          ks = new KeySig(this);
                                          ks->setKeySigEvent(ksv);
                                          ks->setTrack(track);
                                          ks->setGenerated(true);
                                          ks->setParent(s);
                                          undoAddElement(ks);
                                          }
                                    else if (ks->keySigEvent() != ksv) {
                                          undo(new ChangeKeySig(ks, ksv,
                                             ks->showCourtesy(), ks->showNaturals()));
                                          }
                                    // change bar line to qreal bar line
                                    // m->setEndBarLineType(DOUBLE_BAR, true); // this caused issue #12918
                                    }
                              }
                        if (!showCourtesySig) {
                              // remove any existent courtesy key signature
                              Segment* s = m->findSegment(Segment::SegKeySigAnnounce, tick);
                              if (s && s->element(track))
                                    undoRemoveElement(s->element(track));
                              }
                        }

                  // courtesy clefs
                  // no courtesy clef if this measure is the end of a repeat

                  bool showCourtesyClef = styleB(ST_genCourtesyClef) && nm && !(m->repeatFlags() & RepeatEnd);
                  for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                        Staff* staff = _staves[staffIdx];
                        ClefType clefType = staff->clef(tick);
                        if (clefType == staff->clef(tick-1))
                              continue;

                        bool show = showCourtesyClef;
                        Clef* clef;
                        if (showCourtesyClef) {
                              // locate a clef in next measure and, if found,
                              // check if it has court. sig turned off
                              s = nm->findSegment(Segment::SegClef, tick);
                              if (s) {
                                    clef = static_cast<Clef*>(s->element(staffIdx*VOICES));
                                    if (clef && !clef->showCourtesy()) {
                                          show = false;
                                          continue;   // this key change has court. sig turned off
                                          }
                                    }
                              }
                        s = m->findSegment(Segment::SegClef, tick);
                        if (s)
                              clef = static_cast<Clef*>(s->element(staffIdx * VOICES));
                        else
                              clef = 0;
                        Clef* nextClef = 0;
                        Segment* ns = nm->findSegment(Segment::SegClef, tick);
                        if (ns)
                              nextClef = static_cast<Clef*>(ns->element(staffIdx * VOICES));
                        if (clef && !nextClef) {
                              //
                              // move original clef to next measure
                              //
                              nextClef = clef->clone();
                              ns = nm->undoGetSegment(Segment::SegClef, tick);
                              nextClef->setParent(ns);
                              undoAddElement(nextClef);
                              }
                        if (clef && !show) {
                              undoRemoveElement(clef);
                              clef = 0;
                              }
                        else if (!clef && show) {
                              s = m->undoGetSegment(Segment::SegClef, tick);
                              int track = staffIdx * VOICES;
                              clef = new Clef(this);
                              clef->setClefType(clefType);
                              clef->setTrack(track);
                              clef->setSmall(true);
                              clef->setParent(s);
                              undoAddElement(clef);
                              }
                        if (clef)
                              clef->setGenerated(true);
                        if (nextClef)
                              nextClef->setGenerated(false);
                        }
                  }

            //
            //    compute repeat bar lines
            //
//            if (!undoRedo()) {
                  bool firstMeasure = true;
                  const QList<MeasureBase*>& ml = system->measures();
                  MeasureBase* lmb = ml.back();
#if 0 // MM
                  if (lmb->type() == Element::MEASURE) {
                        if (static_cast<Measure*>(lmb)->multiMeasure() > 0) {
                              for (;;lmb = lmb->next()) {
                                    if (lmb->next() == 0)
                                          break;
                                    if ((lmb->next()->type() == Element::MEASURE) && ((Measure*)(lmb->next()))->multiMeasure() >= 0)
                                          break;
                                    }
                              }
                        }
#endif
                  for (MeasureBase* mb = ml.front(); mb; mb = mb->next()) {
                        if (mb->type() != Element::MEASURE) {
                              if (mb == lmb)
                                    break;
                              continue;
                              }
                        Measure* m = static_cast<Measure*>(mb);
                        // first measure repeat?
                        bool fmr = firstMeasure && (m->repeatFlags() & RepeatStart);

                        if (mb == ml.back()) {       // last measure in system?
                              //
                              // if last bar has a courtesy key signature,
                              // create a double bar line as end bar line
                              //
                              BarLineType bl = hasCourtesyKeysig ? DOUBLE_BAR : NORMAL_BAR;
                              if (m->repeatFlags() & RepeatEnd)
                                    m->setEndBarLineType(END_REPEAT, true);
                              else if (m->endBarLineGenerated())
                                    m->setEndBarLineType(bl, true);
                              if (m->setStartRepeatBarLine(fmr))
                                    m->setDirty();
                              }
                        else {
                              MeasureBase* mb = m->next();
                              while (mb && mb->type() != Element::MEASURE && (mb != ml.back()))
                                    mb = mb->next();

                              Measure* nm = 0;
                              if (mb && mb->type() == Element::MEASURE)
                                    nm = static_cast<Measure*>(mb);

                              needRelayout |= m->setStartRepeatBarLine(fmr);
                              if (m->repeatFlags() & RepeatEnd) {
                                    if (nm && (nm->repeatFlags() & RepeatStart))
                                          m->setEndBarLineType(END_START_REPEAT, true);
                                    else
                                          m->setEndBarLineType(END_REPEAT, true);
                                    }
                              else if (nm && (nm->repeatFlags() & RepeatStart))
                                    m->setEndBarLineType(START_REPEAT, true);
                              else if (m->endBarLineGenerated())
                                    m->setEndBarLineType(NORMAL_BAR, true);
                              }
                        if (m->createEndBarLines())
                              m->setDirty();
                        firstMeasure = false;
                        if (mb == lmb)
                              break;
                        }
#if 0 // MM
                  foreach (MeasureBase* mb, ml) {
                        if (mb->type() != Element::MEASURE)
                              continue;
                        Measure* m = static_cast<Measure*>(mb);
                        int nn = m->multiMeasure() - 1;
                        if (nn > 0) {
                              // skip to last rest measure of multi measure rest
                              Measure* mm = m;
                              for (int k = 0; k < nn; ++k)
                                    mm = mm->nextMeasure();
                              if (mm) {
                                    m->setMmEndBarLineType(mm->endBarLineType());
                                    if (m->createEndBarLines())
                                          m->setDirty();
                                    }
                              }
                        }
#endif

                  }
//            }

      minWidth          = 0.0;
      qreal totalWeight = 0.0;

      foreach(System* system, sl) {
            foreach (MeasureBase* mb, system->measures()) {
                  if (mb->type() == Element::HBOX)
                        minWidth += point(((Box*)mb)->boxWidth());
                  else if (mb->type() == Element::MEASURE) {
                        Measure* m = (Measure*)mb;
                        if (needRelayout)
                              m->setDirty();
                        minWidth    += m->minWidth2();
                        totalWeight += m->ticks() * m->userStretch();
                        }
                  }
            minWidth += system->leftMargin();
            }

      // stretch incomplete row
      qreal rest;
      if (MScore::layoutDebug)
            rest = 0;
      else {
            rest = rowWidth - minWidth;
            if (raggedRight) {
                  if (minWidth > rest)
                        rest = rest * .5;
                  else
                        rest = minWidth;
                  }
            rest /= totalWeight;
            }
      qreal xx   = 0.0;
      qreal y    = 0.0;

      foreach(System* system, sl) {
            QPointF pos;

            bool firstMeasure = true;
            foreach(MeasureBase* mb, system->measures()) {
                  qreal ww = 0.0;
                  if (mb->type() == Element::MEASURE) {
                        if (firstMeasure) {
                              pos.rx() += system->leftMargin();
                              firstMeasure = false;
                              }
                        mb->setPos(pos);
                        Measure* m    = static_cast<Measure*>(mb);
                        if (styleB(ST_FixMeasureWidth)) {
                              ww = rowWidth / system->measures().size();
                              }
                        else {
                              qreal weight = m->ticks() * m->userStretch();
                              ww           = m->minWidth2() + rest * weight;
                              }
                        m->layout(ww);
                        }
                  else if (mb->type() == Element::HBOX) {
                        mb->setPos(pos);
                        ww = point(static_cast<Box*>(mb)->boxWidth());
                        mb->layout();
                        }
                  pos.rx() += ww;
                  }
            system->setPos(xx, y);
            qreal w = pos.x();
            system->setWidth(w);
            system->layout2();
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() == Element::HBOX) {
                        mb->setHeight(system->height());
                        }
                  }
            xx += w;
            }
      return sl;
      }

//---------------------------------------------------------
//   layoutSystems
//   create list of systems
//---------------------------------------------------------

void Score::layoutSystems()
      {
      curMeasure              = _showVBox ? firstMM() : firstMeasureMM();
      curSystem               = 0;
      bool firstSystem        = true;
      bool startWithLongNames = true;

      qreal w  = pageFormat()->printableWidth() * MScore::DPI;

      while (curMeasure) {
            Element::ElementType t = curMeasure->type();
            if (t == Element::VBOX || t == Element::TBOX || t == Element::FBOX) {
                  System* system = getNextSystem(false, true);
                  foreach(SysStaff* ss, *system->staves())
                        delete ss;
                  system->staves()->clear();

                  system->setWidth(w);
                  VBox* vbox = static_cast<VBox*>(curMeasure);
                  vbox->setParent(system);
                  vbox->layout();
                  system->setHeight(vbox->height());
                  system->rxpos() = 0.0;
                  system->setPageBreak(vbox->pageBreak());
                  system->measures().push_back(vbox);
                  curMeasure = curMeasure->nextMM();
                  ++curSystem;
                  }
            else {
                  QList<System*> sl  = layoutSystemRow(w, firstSystem, startWithLongNames);
                  for (int i = 0; i < sl.size(); ++i)
                        sl[i]->setSameLine(i != 0);
                  firstSystem = false;
                  startWithLongNames = false;
                  if (!sl.isEmpty()) {
                        Measure* lm = sl.back()->lastMeasure();
                        firstSystem = lm && lm->sectionBreak() && _layoutMode != LayoutFloat;
                        startWithLongNames = firstSystem && lm->sectionBreak()->startWithLongNames();
                        }
                  else
                        qDebug("empty system!\n");
                  }
            }
      // TODO: make undoable:
      while (_systems.size() > curSystem)
            _systems.takeLast();
      }

//---------------------------------------------------------
//   layoutSystems2
//    update distanceUp, distanceDown
//---------------------------------------------------------

void Score::layoutSystems2()
      {
      int n = _systems.size();
      for (int i = 0; i < n; ++i) {
            System* system = _systems.at(i);
            if (!system->isVbox()) {
                  system->layout2();
                  }
            }
      }

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Score::layoutLinear()
      {
      curMeasure     = first();
      curSystem      = 0;
      System* system = getNextSystem(true, false);
      system->setInstrumentNames(true);
      qreal xo = 0;

      Measure* fm = firstMeasure();
      for (MeasureBase* m = first(); m != fm ; m = m->next()) {
            if (m->type() == Element::HBOX)
                  xo += point(static_cast<Box*>(m)->boxWidth());
            }

      system->layout(xo);
      system->setPos(0.0, spatium() * 10.0);
      curPage = 0;
      Page* page = getEmptyPage();
      page->appendSystem(system);

      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            Element::ElementType t = curMeasure->type();
            if (t == Element::VBOX || t == Element::TBOX || t == Element::FBOX) {
                  curMeasure = curMeasure->next();
                  continue;
                  }
            if (styleB(ST_createMultiMeasureRests) && t == Element::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  if (m->hasMMRest())
                        mb = m->mmRest();
                  }
            mb->setSystem(system);
            system->measures().append(mb);
            }
      if (system->measures().isEmpty())
            return;
      addSystemHeader(firstMeasureMM(), true);
      removeGeneratedElements(firstMeasureMM(), lastMeasureMM());

      QPointF pos(0.0, 0.0);
      bool isFirstMeasure = true;
      foreach (MeasureBase* mb, system->measures()) {
            qreal w = 0.0;
            if (mb->type() == Element::MEASURE) {
                  if(isFirstMeasure) {
                        pos.rx() += system->leftMargin();
                        isFirstMeasure = false;
                        }
                  Measure* m = static_cast<Measure*>(mb);
                  Measure* nm = m->nextMeasure();
                 if (m->repeatFlags() & RepeatEnd) {
                        if (nm && (nm->repeatFlags() & RepeatStart))
                              m->setEndBarLineType(END_START_REPEAT, true);
                        else
                              m->setEndBarLineType(END_REPEAT, true);
                        }
                  else if (nm && (nm->repeatFlags() & RepeatStart))
                        m->setEndBarLineType(START_REPEAT, true);
                  m->createEndBarLines();
                  w = m->minWidth1() * styleD(ST_linearStretch);
                  m->layout(w);
                  }
            else {
                  mb->layout();
                  w = mb->width();
                 }

            mb->setPos(pos);
            pos.rx() += w;
            }
      system->setWidth(pos.x());
      page->setWidth(pos.x());
      system->layout2();

      while (_pages.size() > 1)
            _pages.takeLast();
      }

//---------------------------------------------------------
//   getEmptyPage
//---------------------------------------------------------

Page* Score::getEmptyPage()
      {
      Page* page = curPage >= _pages.size() ? addPage() : _pages[curPage];
      page->setNo(curPage);
      page->layout();
      qreal x = (curPage == 0) ? 0.0 : _pages[curPage - 1]->pos().x()
         + page->width() + ((curPage & 1) ? 50.0 : 1.0);
      ++curPage;
      page->setPos(x, 0.0);
      page->systems()->clear();
      return page;
      }

//---------------------------------------------------------
//   SystemRow
//---------------------------------------------------------

struct SystemRow {
      QList<System*> systems;

      qreal height() const {
            qreal h = 0.0;
            foreach(System* s, systems) {
                  if (s->height() > h)
                        h = s->height();
                  }
            return h;
            }
      bool isVbox() const {
            return (systems.size() > 0) ? systems.back()->isVbox() : false;
            }
      VBox* vbox() const {
            return (systems.size() > 0) ? systems.back()->vbox() : 0;
            }
      bool pageBreak() const {
            return (systems.size() > 0) ? systems.back()->pageBreak() : false;
            }
      qreal tm() const {
            qreal v = 0.0;
            foreach(System* s, systems) {
                  if (!s->staves()->isEmpty())
                        v = qMax(s->distanceUp(0), v);
                  }
            return v;
            }
      qreal bm() const {
            qreal v = 0.0;
            foreach(System* s, systems) {
                  int staffIdx = s->staves()->size() - 1;
                  if (staffIdx >= 0)
                        v = qMax(s->distanceDown(staffIdx), v);
                  }
            return v;
            }
      void clear() {
            systems.clear();
            }
      };

//---------------------------------------------------------
//   PageContext
//---------------------------------------------------------

struct PageContext {
      qreal _spatium;
      qreal slb;
      Score* score;
      Page* page;
      qreal ey;
      qreal y;
      int gaps;
      SystemRow sr;

      System* lastSystem;
      qreal prevDist;

      PageContext(Score* s) : score(s) {
            _spatium = score->spatium();
            slb      = score->styleS(ST_staffLowerBorder).val() * _spatium;
            }
      void newPage() {
            page       = score->getEmptyPage();
            ey         = page->height() - page->bm();
            y          = page->tm();
            gaps       = 0;
            lastSystem = 0;
            prevDist   = 0.0;
            }
      void layoutPage() {
            qreal d = sr.isVbox() ? sr.vbox()->bottomGap() : slb;
            score->layoutPage(*this, d);
            }
      qreal bm() const { return sr.bm(); }
      qreal tm() const { return sr.tm(); }
      };

//---------------------------------------------------------
//   layoutPages
//    create list of pages
//---------------------------------------------------------

void Score::layoutPages()
      {
      const qreal _spatium            = spatium();
      const qreal slb                 = styleS(ST_staffLowerBorder).val()    * _spatium;
      const qreal sub                 = styleS(ST_staffUpperBorder).val()    * _spatium;
      const qreal systemDist          = styleS(ST_minSystemDistance).val()   * _spatium;
      const qreal systemFrameDistance = styleS(ST_systemFrameDistance).val() * _spatium;
      const qreal frameSystemDistance = styleS(ST_frameSystemDistance).val() * _spatium;

      curPage = 0;

      PageContext pC(this);
      pC.newPage();

      int nSystems = _systems.size();

      for (int i = 0; i < nSystems; ++i) {
            //
            // collect system row
            //
            pC.sr.clear();
            for (;;) {
                  System* system = _systems[i];
                  pC.sr.systems.append(system);
                  if (i+1 == nSystems)
                        break;
                  if (!_systems[i+1]->sameLine())
                        break;
                  ++i;
                  }
            qreal tmargin = 0.0;    // top system margin
            qreal bmargin;          // bottom system margin

            if (pC.sr.isVbox()) {
                  VBox* vbox = pC.sr.vbox();
                  bmargin  = vbox->bottomGap();
                  tmargin += vbox->topGap();
                  if (pC.lastSystem) {
                       if (pC.lastSystem->isVbox())
                              tmargin += pC.lastSystem->vbox()->bottomGap();
                        else
                              tmargin += systemFrameDistance;
                        }
                  }
            else {
                  if (pC.lastSystem) {
                        if (pC.lastSystem->isVbox())
                              tmargin = pC.lastSystem->vbox()->bottomGap() + frameSystemDistance;
                        else
                              tmargin = qMax(pC.tm(), systemDist);
                        }
                  else
                        tmargin = qMax(pC.tm(), sub);
                  bmargin = pC.bm();
                  }

            tmargin     = qMax(tmargin, pC.prevDist);
            pC.prevDist = bmargin;

            qreal h = pC.sr.height();
            if (pC.lastSystem && (pC.y + h + tmargin + qMax(bmargin, slb) > pC.ey)) {
                  //
                  // prepare next page
                  //
                  qreal d;
                  if (pC.lastSystem->isVbox())
                        d = pC.lastSystem->vbox()->bottomGap();
                  else
                        d = slb;
                  layoutPage(pC, d);
                  pC.newPage();
                  if (pC.sr.isVbox())
                        tmargin = pC.sr.vbox()->topGap();
                  else
                        tmargin = qMax(pC.sr.tm(), sub);
                  }

            qreal x = pC.page->lm();
            pC.y   += tmargin;

            foreach(System* system, pC.sr.systems) {
                  system->setPos(x, pC.y);
                  x += system->width();
                  pC.page->appendSystem(system);
                  system->setAddStretch(false);
                  }

            if (pC.lastSystem) {
                  bool addStretch = !pC.lastSystem->isVbox() && !pC.sr.isVbox();
                  pC.lastSystem->setAddStretch(addStretch);
                  if (addStretch)
                        ++pC.gaps;
                  }

            pC.y += h;
            if (pC.sr.pageBreak() && (_layoutMode == LayoutPage)) {
                  if ((i + 1) == nSystems)
                        break;
                  pC.layoutPage();
                  pC.newPage();
                  }
            else
                  pC.lastSystem = pC.sr.systems.back();
            }
      if (pC.page)
            pC.layoutPage();

      // Remove not needed pages. TODO: make undoable:
      while (_pages.size() > curPage)
            _pages.takeLast();
      }

//---------------------------------------------------------
//   systemDistCompare
//---------------------------------------------------------

static bool systemDistCompare(System* s1, System* s2)
      {
      return s1->distance() < s2->distance();
      }

//---------------------------------------------------------
//   layoutPage
//    increase system distance upto maxSystemDistance
//    to fill page
//---------------------------------------------------------

void Score::layoutPage(const PageContext& pC, qreal d)
      {
      Page* page       = pC.page;
      int gaps         = pC.gaps;
      qreal restHeight = pC.ey - pC.y - d;

      if (!gaps || MScore::layoutDebug) {
            if (_layoutMode == LayoutFloat) {
                  qreal y = restHeight * .5;
                  int n = page->systems()->size();
                  for (int i = 0; i < n; ++i) {
                        System* system = page->systems()->at(i);
                        system->move(0, y);
                        }
                  }
            return;
            }

      const qreal maxStretch = styleP(ST_maxSystemDistance) - styleP(ST_minSystemDistance);

      QList<System*> slist;
      int n = page->systems()->size();
      for (int i = 0; i < n; ++i) {
            System* system = page->systems()->at(i);
            qreal lastY1   = system->pos().y() + system->height();

            if (system->addStretch()) {
                  System* ns = page->systems()->at(i + 1);
                  qreal dist = ns->pos().y() - lastY1;
                  system->setDistance(dist);
                  slist.append(system);
                  system->setStretchDistance(0.0);
                  }
            }
      qSort(slist.begin(), slist.end(), systemDistCompare);

      n = slist.size();
      for (int i = 0; i < (n-1); ++i) {
            System* s1 = slist.at(i);
            System* s2 = slist.at(i + 1);
            qreal td = s2->distance() - s1->distance();
            if (td > 0.001) {
                  int nn = i + 1;
                  qreal tdd = td * nn;
                  if (tdd > restHeight) {
                        tdd = restHeight;
                        td  = tdd / nn;
                        }
                  if (s1->stretchDistance() + td > maxStretch) {
                        td = maxStretch - s1->stretchDistance();
                        tdd = td * nn;
                        }
                  for (int k = 0; k <= i; ++k)
                        slist.at(k)->addStretchDistance(td);
                  restHeight -= tdd;
                  }
            }
      qreal td = restHeight / n;
      if (td > 0.001) {
            qreal sd = slist.at(0)->stretchDistance();
            if (sd + td > maxStretch)
                  td = maxStretch - sd;
            for (int k = 0; k < n; ++k)
                  slist.at(k)->addStretchDistance(td);
            }

      qreal y = 0.0;
      n = page->systems()->size();
      for (int i = 0; i < n; ++i) {
            System* system = page->systems()->at(i);
            system->move(0, y);
            if (system->addStretch())
                  y += system->stretchDistance();
            }
      }

//---------------------------------------------------------
//   doLayoutSystems
//    layout staves in a system
//    layout pages
//---------------------------------------------------------

void Score::doLayoutSystems()
      {
      foreach(System* system, _systems)
            system->layout2();
      layoutPages();
      rebuildBspTree();
      _updateAll = true;

      foreach(MuseScoreView* v, viewer)
            v->layoutChanged();
      }

//---------------------------------------------------------
//   doLayoutPages
//    small wrapper for layoutPages()
//---------------------------------------------------------

void Score::doLayoutPages()
      {
      layoutPages();
      rebuildBspTree();
      _updateAll = true;
      foreach(MuseScoreView* v, viewer)
            v->layoutChanged();
      }

//---------------------------------------------------------
//   sff
//    compute 1/Force for a given Extend
//---------------------------------------------------------

qreal sff(qreal x, qreal xMin, const SpringMap& springs)
      {
      if (x <= xMin)
            return 0.0;
      iSpring i = springs.begin();
      qreal c  = i->second.stretch;
      if (c == 0.0)           //DEBUG
            c = 1.1;
      qreal f = 0.0;
      for (; i != springs.end();) {
            xMin -= i->second.fix;
            f = (x - xMin) / c;
            ++i;
            if (i == springs.end() || f <= i->first)
                  break;
            c += i->second.stretch;
            }
      return f;
      }

//---------------------------------------------------------
//   respace
//---------------------------------------------------------

void Score::respace(QList<ChordRest*>* elements)
      {
      ChordRest* cr1 = elements->front();
      ChordRest* cr2 = elements->back();
      int n          = elements->size();
      qreal x1       = cr1->segment()->pos().x();
      qreal x2       = cr2->segment()->pos().x();

      qreal width[n-1];
      int ticksList[n-1];
      int minTick = 100000;

      for (int i = 0; i < n-1; ++i) {
            ChordRest* cr  = (*elements)[i];
            ChordRest* ncr = (*elements)[i+1];
            Space space(cr->space());
            Space nspace(ncr->space());
            width[i] = space.rw() + nspace.lw();
            ticksList[i] = ncr->segment()->tick() - cr->segment()->tick();
            minTick = qMin(ticksList[i], minTick);
            }

      //---------------------------------------------------
      // compute stretches
      //---------------------------------------------------

      SpringMap springs;
      qreal minimum = 0.0;
      for (int i = 0; i < n-1; ++i) {
            qreal w   = width[i];
            int t     = ticksList[i];
            // qreal str = 1.0 + .6 * log(qreal(t) / qreal(minTick)) / log(2.0);
            qreal str = 1.0 + 0.865617 * log(qreal(t) / qreal(minTick));
            qreal d   = w / str;

            springs.insert(std::pair<qreal, Spring>(d, Spring(i, str, w)));
            minimum += w;
            }

      //---------------------------------------------------
      //    distribute stretch to elements
      //---------------------------------------------------

      qreal force = sff(x2 - x1, minimum, springs);
      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            qreal stretch = force * i->second.stretch;
            if (stretch < i->second.fix)
                  stretch = i->second.fix;
            width[i->second.seg] = stretch;
            }
      qreal x = x1;
      for (int i = 1; i < n-1; ++i) {
            x += width[i-1];
            ChordRest* cr = (*elements)[i];
            qreal dx = x - cr->segment()->pos().x();
            cr->rxpos() += dx;
            }
      }

//---------------------------------------------------------
//   computeMinWidth
///    compute the minimum width of a measure with
///    segment list fs
//---------------------------------------------------------

qreal Score::computeMinWidth(Segment* fs)
      {
      int _nstaves = nstaves();
      if (_nstaves == 0)
            return 1.0;

      qreal _spatium           = spatium();
      qreal clefKeyRightMargin = styleS(ST_clefKeyRightMargin).val() * _spatium;
      qreal minNoteDistance    = styleS(ST_minNoteDistance).val()    * _spatium;
      qreal minHarmonyDistance = styleS(ST_minHarmonyDistance).val() * _spatium;

      qreal rest[_nstaves];    // fixed space needed from previous segment
      memset(rest, 0, _nstaves * sizeof(qreal));

      qreal hRest[_nstaves];    // fixed space needed from previous harmony
      memset(hRest, 0, _nstaves * sizeof(qreal));

      qreal clefWidth[_nstaves];
      memset(clefWidth, 0, _nstaves * sizeof(qreal));

      std::vector<QRectF> hLastBbox(_nstaves);    // bbox of previous harmony to test vertical separation

      int segmentIdx = 0;
      qreal x        = 0.0;

      const Segment* pSeg = 0;
      for (Segment* s = fs; s; s = s->next(), ++segmentIdx) {
            qreal elsp = s->extraLeadingSpace().val()  * _spatium;
            qreal etsp = s->extraTrailingSpace().val() * _spatium;

            if ((s->segmentType() == Segment::SegClef) && (s != fs)) {
                  --segmentIdx;
                  for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
                        if (!staff(staffIdx)->show())
                              continue;
                        int track  = staffIdx * VOICES;
                        Element* e = s->element(track);
                        if (e) {
                              e->layout();
                              clefWidth[staffIdx] = e->width() + _spatium + elsp;
                              }
                        }
                  continue;
                  }
            bool rest2[_nstaves];
            bool hRest2[_nstaves];
            Segment::SegmentType segType = s->segmentType();
            qreal segmentWidth     = 0.0;
            qreal stretchDistance  = 0.0;
            int pt                 = pSeg ? pSeg->segmentType() : Segment::SegBarLine;

            for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
                  if (!staff(staffIdx)->show())
                        continue;
                  qreal minDistance = 0.0;
                  Space space;
                  Space hSpace;
                  QRectF hBbox;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  bool hFound = false;
                  bool eFound = false;
                  if (segType & (Segment::SegChordRest)) {
                        qreal llw = 0.0;
                        qreal rrw = 0.0;
                        Lyrics* lyrics = 0;
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (pt & (Segment::SegStartRepeatBarLine | Segment::SegBarLine)) {
                                    // check for accidentals in chord
                                    bool accidental = false;
                                    if (cr->type() == Element::CHORD) {
                                          Chord* c = static_cast<Chord*>(cr);
                                          if (!c->graceNotes().empty())
                                                accidental = true;
                                          else {
                                                for (Note* note : c->notes()) {
                                                      if (note->accidental()) {
                                                            accidental = true;
                                                            break;
                                                            }
                                                      }
                                                }
                                          }
                                    StyleIdx si = accidental ? ST_barAccidentalDistance : ST_barNoteDistance;
                                    qreal sp    = styleS(si).val() * _spatium;
                                    sp         += elsp;
                                    minDistance = qMax(minDistance, sp);
                                    stretchDistance = sp * .7;
                                    }
                              else if (pt & (Segment::SegChordRest)) {
                                    minDistance = qMax(minDistance, minNoteDistance);
                                    }
                              else {
                                    // if (pt & (Segment::SegKeySig | Segment::SegClef))
                                    bool firstClef = (segmentIdx == 1) && (pt == Segment::SegClef);
                                    if ((pt & (Segment::SegKeySig | Segment::SegTimeSig)) || firstClef)
                                          minDistance = qMax(minDistance, clefKeyRightMargin);
                                    }
                              cr->layout();
                              space.max(cr->space());
                              foreach(Lyrics* l, cr->lyricsList()) {
                                    if (!l)
                                          continue;
                                    if (!l->isEmpty()) {
                                          l->layout();
                                          lyrics = l;
                                          if (!lyrics->isMelisma()) {
                                                QRectF b(l->bbox().translated(l->pos()));
                                                llw = qMax(llw, -b.left());
                                                rrw = qMax(rrw, b.right());
                                                }
                                          }
                                    }
                              }
                        if (lyrics)
                              space.max(Space(llw, rrw));

                        // add spacing for chord symbols
                        foreach (Element* e, s->annotations()) {
                              if (e->type() != Element::HARMONY || e->track() < track || e->track() >= track+VOICES)
                                    continue;
                              Harmony* h = static_cast<Harmony*>(e);
                              //h->layout();
                              h->calculateBoundingRect();
                              QRectF b(h->bboxtight().translated(h->pos()));
                              if (hFound)
                                    hBbox |= b;
                              else
                                    hBbox = b;
                              hFound = true;
                              // allow chord at the beginning of a measure to be dragged left
                              hSpace.max(Space(s->rtick()?-b.left():0.0, b.right()));
                              }
                        }
                  else {
                        Element* e = s->element(track);
                        if ((segType == Segment::SegClef) && (pt != Segment::SegChordRest))
                              minDistance = styleP(ST_clefLeftMargin);
                        else if (segType == Segment::SegStartRepeatBarLine)
                              minDistance = .5 * _spatium;
                        else if ((segType == Segment::SegEndBarLine) && segmentIdx) {
                              if (pt == Segment::SegClef)
                                    minDistance = styleP(ST_clefBarlineDistance);
                              else
                                    stretchDistance = styleP(ST_noteBarDistance);
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
                              e->layout();
                              space.max(e->space());
                              }
                        }
                  space += Space(elsp, etsp);

                  if (found || eFound) {
                        space.rLw() += clefWidth[staffIdx];
                        qreal sp     = minDistance + rest[staffIdx] + qMax(space.lw(), stretchDistance);
                        rest[staffIdx]  = space.rw();
                        rest2[staffIdx] = false;
                        segmentWidth    = qMax(segmentWidth, sp);
                        }
                  else
                        rest2[staffIdx] = true;

                  // space chord symbols separately from segments
                  if (hFound || eFound) {
                        qreal sp = 0.0;

                        // space chord symbols unless they miss each other vertically
                        if (eFound || (hBbox.top() < hLastBbox[staffIdx].bottom() && hBbox.bottom() > hLastBbox[staffIdx].top()))
                              sp = hRest[staffIdx] + minHarmonyDistance + hSpace.lw();
                        hLastBbox[staffIdx] = hBbox;

                        hRest[staffIdx] = hSpace.rw();
                        hRest2[staffIdx] = false;
                        segmentWidth = qMax(segmentWidth, sp);
                        }
                  else
                        hRest2[staffIdx] = true;

                  clefWidth[staffIdx] = 0.0;
                  }

            x += segmentWidth;

//            if (segmentIdx && pSeg)
//                  pSeg->setbbox(QRectF(0.0, 0.0, segmentWidth, _spatium * 5));  //??

            for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
                  if (!staff(staffIdx)->show())
                        continue;
                  if (rest2[staffIdx])
                        rest[staffIdx] -= qMin(rest[staffIdx],segmentWidth);
                  if (hRest2[staffIdx])
                        hRest[staffIdx] -= qMin(hRest[staffIdx],segmentWidth);
                  }
            //
            // set pSeg only to used segments
            //
            for (int voice = 0; voice < _nstaves * VOICES; ++voice) {
                  if (!staff(voice/VOICES)->show()) {
                        voice += VOICES-1;
                        continue;
                        }
                  if (s->element(voice)) {
                        pSeg = s;
                        break;
                        }
                  }
            }

      qreal segmentWidth = 0.0;
      for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
            if (!staff(staffIdx)->show())
                  continue;
            segmentWidth = qMax(segmentWidth, rest[staffIdx]);
            segmentWidth = qMax(segmentWidth, hRest[staffIdx]);
            }
      x += segmentWidth;
      return x;
      }

//---------------------------------------------------------
//   updateBarLineSpans
///   updates bar line span(s) when the number of lines of a staff changes
//---------------------------------------------------------

void Score::updateBarLineSpans(int idx, int linesOld, int linesNew)
      {
      int         nStaves = nstaves();
      Staff*      _staff;

      // scan staves and check the destination staff of each bar line span
      // barLineSpan is not changed; barLineFrom and barLineTo are changed if they occur in the bottom half of a staff
      // in practice, a barLineFrom/To from/to the top half of the staff is linked to the staff top line,
      // a barLineFrom/To from/to the bottom half of the staff is linked to staff bottom line;
      // this ensures plainchant and mensurstrich special bar lines keep their relationships to the staff lines.
      // 1-line staves are traited as a special case.
      for(int sIdx = 0; sIdx < nStaves; sIdx++) {
            _staff = staff(sIdx);
            // if this is the modified staff
            if(sIdx == idx) {
                  // if it has no bar line, set barLineTo to a default value
                  if(_staff->barLineSpan() == 0)
                        _staff->setBarLineTo( (linesNew-1) * 2);
                  // if new line number is 1, set default From for 1-line staves
                  else if(linesNew == 1)
                        _staff->setBarLineFrom(BARLINE_SPAN_1LINESTAFF_FROM);
                  // if barLineFrom was below the staff middle position
                  // raise or lower it to account for new number of lines
                  else if(_staff->barLineFrom() > linesOld - 1)
                        _staff->setBarLineFrom(_staff->barLineFrom() + (linesNew - linesOld)*2);
            }

            // if the modified staff is the destination of the current staff bar span:
            if(sIdx + _staff->barLineSpan() - 1 == idx) {
                  // if new line number is 1, set default To for 1-line staves
                  if(linesNew == 1)
                        _staff->setBarLineTo(BARLINE_SPAN_1LINESTAFF_TO);
                  // if barLineTo was below its middle position, raise or lower it
                  else if(_staff->barLineTo() > linesOld - 1)
                        _staff->setBarLineTo(_staff->barLineTo() + (linesNew - linesOld)*2);
                  }
            }
      }

}

