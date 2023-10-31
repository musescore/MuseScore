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

#include "utils.h"
#include "score.h"
#include "pitchspelling.h"
#include "key.h"
#include "staff.h"
#include "note.h"
#include "harmony.h"
#include "segment.h"
#include "undo.h"
#include "keysig.h"
#include "stafftype.h"
#include "chord.h"
#include "measure.h"
#include "fret.h"
#include "part.h"

namespace Ms {

//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

static Interval keydiff2Interval(Key oKey, Key nKey, TransposeDirection dir)
      {
      static int stepTable[15] = {
            // C  G  D  A  E  B Fis
               0, 4, 1, 5, 2, 6, 3,
            };

      int cofSteps;     // circle of fifth steps
      int diatonic;
      if (nKey > oKey)
            cofSteps = int(nKey) - int(oKey);
      else
            cofSteps = 12 - (int(oKey) - int(nKey));
      diatonic = stepTable[(int(nKey) + 7) % 7] - stepTable[(int(oKey) + 7) % 7];
      if (diatonic < 0)
            diatonic += 7;
      diatonic %= 7;
      int chromatic = (cofSteps * 7) % 12;


      if ((dir == TransposeDirection::CLOSEST) && (chromatic > 6))
            dir = TransposeDirection::DOWN;

      if (dir == TransposeDirection::DOWN) {
            chromatic = chromatic - 12;
            diatonic  = diatonic - 7;
            if (diatonic == -7)
                  diatonic = 0;
            if (chromatic == -12)
                  chromatic = 0;
            }
      return Interval(diatonic, chromatic);
      }

/*!
 * Transposes both pitch and spelling for a note given an interval.
 *
 * Uses addition for pitch and transposeTpc() for spelling.
 *
 * @param pitch
 *  The initial (current) pitch. (pitch)
 * @param tpc
 *  The initial spelling. (tpc)
 * @param rpitch
 *  A pointer to the transposed pitch, calculated by this function. (pitch)
 * @param rtpc
 *  A pointer to the transposed spelling. (tcp)
 * @param interval
 *  The interval to transpose by.
 * @param useDoubleSharpsFlats
 *  Determines whether the output may include double sharps or flats (Abb)
 *  or should use an enharmonic pitch (Abb = G).
 */

void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, Interval interval,
   bool useDoubleSharpsFlats)
      {
      *rpitch = pitch + interval.chromatic;
      *rtpc   = transposeTpc(tpc, interval, useDoubleSharpsFlats);
      }

/*!
 * Transposes a pitch spelling given an interval.
 *
 * This function transposes a pitch spelling using first
 * a diatonic transposition and then calculating any accidentals.
 * This insures that the note is changed by the correct number of
 * scale degrees unless it would require too many accidentals.
 *
 * @param tpc
 *  The initial pitch spelling.
 * @param interval
 *  The interval to be transposed by.
 * @param useDoubleSharpsFlats
 *  Determines whether the output may include double sharps or flats (Abb)
 *  or should use an enharmonic pitch (Abb = G).
 *
 * @return
 *  The transposed pitch spelling (tpc).
 */

int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats)
      {
      if (tpc == Tpc::TPC_INVALID) // perfect unison & perfect octave
            return tpc;

      int minAlter;
      int maxAlter;
      if (useDoubleSharpsFlats) {
            minAlter = -2;
            maxAlter = 2;
            }
      else {
            minAlter = -1;
            maxAlter = 1;
            }
      int steps     = interval.diatonic;
      int semitones = interval.chromatic;

// qDebug("transposeTpc tpc %d steps %d semitones %d", tpc, steps, semitones);
      if (semitones == 0 && steps == 0)
            return tpc;

      int step;
      int alter;
      int pitch = tpc2pitch(tpc);

      for (int k = 0; k < 10; ++k) {
            step = tpc2step(tpc) + steps;
            while (step < 0)
                  step += 7;
            step   %= 7;
            int p1 = tpc2pitch(step2tpc(step, AccidentalVal::NATURAL));
            alter  = semitones - (p1 - pitch);
            // alter  = p1 + semitones - pitch;

//            if (alter < 0) {
//                  alter *= -1;
//                  alter = 12 - alter;
//                  }
            while (alter < 0)
                  alter += 12;

            alter %= 12;
            if (alter > 6)
                  alter -= 12;
            if (alter > maxAlter)
                  ++steps;
            else if (alter < minAlter)
                  --steps;
            else
                  break;
//            qDebug("  again alter %d steps %d, step %d", alter, steps, step);
            }
//      qDebug("  = step %d alter %d  tpc %d", step, alter, step2tpc(step, alter));
      return step2tpc(step, AccidentalVal(alter));
      }

//---------------------------------------------------------
//   transposeTpcDiatonicByKey
//
// returns the tpc diatonically transposed by steps, using degrees of given key
// option to keep any alteration tpc had with respect to unaltered corresponding degree of key
// option to enharmonically reduce tpc using double alterations
//---------------------------------------------------------

int transposeTpcDiatonicByKey(int tpc, int steps, Key key, bool keepAlteredDegrees, bool useDoubleSharpsFlats)
      {
      if (tpc == Tpc::TPC_INVALID)
            return tpc;

      // get step for tpc with alteration for key
      int alter;
      int step = tpc2stepByKey(tpc, key, alter);

      // transpose step and get tpc for step/key
      step += steps;
      int newTpc = step2tpcByKey(step, key);

      // if required, apply alteration to new tpc
      if(keepAlteredDegrees)
            newTpc += alter * TPC_DELTA_SEMITONE;

      // check results are in ranges
      while (newTpc > Tpc::TPC_MAX)      newTpc   -= TPC_DELTA_ENHARMONIC;
      while (newTpc < Tpc::TPC_MIN)      newTpc   += TPC_DELTA_ENHARMONIC;

      // if required, reduce double alterations
      if(!useDoubleSharpsFlats) {
            if(newTpc >= Tpc::TPC_F_SS)  newTpc   -= TPC_DELTA_ENHARMONIC;
            if(newTpc <= Tpc::TPC_B_BB)  newTpc   += TPC_DELTA_ENHARMONIC;
            }

      return newTpc;
      }

//---------------------------------------------------------
//   transpose
//    return false on failure
//---------------------------------------------------------

bool Score::transpose(Note* n, Interval interval, bool useDoubleSharpsFlats)
      {
      int npitch;
      int ntpc1, ntpc2;
      transposeInterval(n->pitch(), n->tpc1(), &npitch, &ntpc1, interval, useDoubleSharpsFlats);
      if (n->transposition()) {
            int p;
            transposeInterval(n->pitch() - n->transposition(), n->tpc2(), &p, &ntpc2, interval, useDoubleSharpsFlats);
            }
      else
            ntpc2 = ntpc1;
      if (npitch > 127)
            return false;
      undoChangePitch(n, npitch, ntpc1, ntpc2);
      return true;
      }

//---------------------------------------------------------
//   transpose
//    return false on failure
//---------------------------------------------------------

bool Score::transpose(TransposeMode mode, TransposeDirection direction, Key trKey,
  int transposeInterval, bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats)
      {
      bool result = true;
      bool rangeSelection = selection().isRange();
      int startStaffIdx   = 0;
      int endStaffIdx     = 0;
      Fraction startTick  = Fraction(0,1);
      if (rangeSelection) {
            startStaffIdx = selection().staffStart();
            endStaffIdx   = selection().staffEnd();
            startTick     = selection().tickStart();
            }

      Staff* st = staff(startStaffIdx);

      Interval interval;
      if (mode != TransposeMode::DIATONICALLY) {
            if (mode == TransposeMode::TO_KEY) {
                  // calculate interval from "transpose to key"
                  // find the key of the first pitched staff
                  Key key = Key::C;
                  for (int i = startStaffIdx; i < endStaffIdx; ++i) {
                        Staff* s = staff(i);
                        if (s->isPitchedStaff(startTick)) {
                              key = s->key(startTick);
                              if (!styleB(Sid::concertPitch)) {
                                    int diff = s->part()->instrument(startTick)->transpose().chromatic;
                                    if (diff)
                                          key = transposeKey(key, diff, s->part()->preferSharpFlat());
                                    }
                              // remember this staff to use as basis in transposing key signatures
                              st = s;
                              break;
                              }
                        }
                  if (key != trKey) {
                        interval = keydiff2Interval(key, trKey, direction);
                        }
                  else {      //same key, which direction?
                        if (direction == TransposeDirection::UP)
                              interval = Interval(12);
                        else if (direction == TransposeDirection::DOWN)
                              interval = Interval(-12);
                        else  //don't do anything for same key and closest direction
                              return true;
                        }
                  }
            else {
                  interval = intervalList[transposeInterval];
                  if (direction == TransposeDirection::DOWN)
                        interval.flip();
                  }

            if (!rangeSelection) {
                  trKeys = false;
                  }
            bool fullOctave = (interval.chromatic % 12) == 0;
            if (fullOctave && (mode != TransposeMode::TO_KEY)) {
                  trKeys = false;
                  transposeChordNames = false;
                  }
            }
      else  { // diatonic transposition
            if (direction == TransposeDirection::DOWN)
                  transposeInterval *= -1;
            }

      if (_selection.isList()) {
            foreach (Element* e, _selection.uniqueElements()) {
                  if (!e->staff() || e->staff()->staffType(e->tick())->group() == StaffGroup::PERCUSSION)
                        continue;
                  if (e->isNote()) {
                        Note* note = toNote(e);
                        if (mode == TransposeMode::DIATONICALLY)
                              note->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                        else {
                              if (!transpose(note, interval, useDoubleSharpsFlats)) {
                                    result = false;
                                    continue;
                                    }
                              }
                        }
                  else if (e->isHarmony() && transposeChordNames) {
                        Harmony* h  = toHarmony(e);
                        int rootTpc, baseTpc;
                        if (mode == TransposeMode::DIATONICALLY) {
                              Fraction tick = Fraction(0,1);
                              if (h->parent()->isSegment())
                                    tick = toSegment(h->parent())->tick();
                              else if (h->parent()->isFretDiagram() && h->parent()->parent()->isSegment())
                                    tick = toSegment(h->parent()->parent())->tick();
                              Key key = !h->staff() ? Key::C : h->staff()->key(tick);
                              rootTpc = transposeTpcDiatonicByKey(h->rootTpc(),
                                          transposeInterval, key, trKeys, useDoubleSharpsFlats);
                              baseTpc = transposeTpcDiatonicByKey(h->baseTpc(),
                                          transposeInterval, key, trKeys, useDoubleSharpsFlats);
                        }
                        else {
                              rootTpc = transposeTpc(h->rootTpc(), interval, useDoubleSharpsFlats);
                              baseTpc = transposeTpc(h->baseTpc(), interval, useDoubleSharpsFlats);
                              }
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  else if (e->isKeySig() && mode != TransposeMode::DIATONICALLY && trKeys) {
                        // TODO: this currently is disabled in dialog
                        // if we enabled it, then it will need work
                        // probably the code should look more like the range selection code
                        KeySig* ks     = toKeySig(e);
                        if (!ks->isCustom() && !ks->isAtonal()) {
                              Key key        = st->key(ks->tick());
                              KeySigEvent ke = ks->keySigEvent();
                              ke.setKey(key);
                              undo(new ChangeKeySig(ks, ke, ks->showCourtesy()));
                              }
                        }
                  }
            }

      //--------------------------
      // process range selection
      //--------------------------

      QList<Staff*> sl;
      for (int staffIdx = _selection.staffStart(); staffIdx < _selection.staffEnd(); ++staffIdx) {
            Staff* s = staff(staffIdx);
            if (s->staffType(Fraction(0,1))->group() == StaffGroup::PERCUSSION)      // ignore percussion staff
                  continue;
            if (sl.contains(s))
                  continue;
            bool alreadyThere = false;
            for (Staff* s2 : sl) {
                  if (s2 == s || (s2->links() && s2->links()->contains(s))) {
                        alreadyThere = true;
                        break;
                        }
                  }
            if (!alreadyThere)
                  sl.append(s);
            }
      QList<int> tracks;
      for (Staff* s : sl) {
            int idx = s->idx() * VOICES;
            for (int i = 0; i < VOICES; ++i)
                  tracks.append(idx + i);
            }

      Segment* s1 = _selection.startSegment();
      if (!s1)
            return result;
      // if range start on mmRest, get the actual segment instead
      if (s1->measure()->isMMRest())
            s1 = tick2segment(s1->tick(), true, s1->segmentType(), false);
      // if range starts with first CR of measure
      // then start looping from very beginning of measure
      // so we include key signature and can transpose that if requested
      if (s1->rtick().isZero())
            s1 = s1->measure()->first();
      Segment* s2 = _selection.endSegment();
      for (Segment* segment = s1; segment && segment != s2; segment = segment->next1()) {
            if (!segment->enabled())
                  continue;
            for (int track : tracks) {
                  if (staff(track/VOICES)->staffType(s1->tick())->group() == StaffGroup::PERCUSSION)
                        continue;
                  Element* e = segment->element(track);
                  if (!e)
                        continue;

                  if (e->isChord()) {
                        Chord* chord = toChord(e);
                        std::vector<Note*> nl = chord->notes();
                        for (Note* n : nl) {
                              if (mode == TransposeMode::DIATONICALLY)
                                    n->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                              else {
                                    if (!transpose(n, interval, useDoubleSharpsFlats)) {
                                          result = false;
                                          continue;
                                          }
                                    }
                              }
                        for (Chord* g : chord->graceNotes()) {
                              for (Note* n : g->notes()) {
                                    if (mode == TransposeMode::DIATONICALLY)
                                          n->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                                    else {
                                          if (!transpose(n, interval, useDoubleSharpsFlats)) {
                                                result = false;
                                                continue;
                                                }
                                          }
                                    }
                              }
                        }
                  else if (e->isKeySig() && trKeys && mode != TransposeMode::DIATONICALLY) {
                        KeySig* ks = toKeySig(e);
                        Fraction tick = segment->tick();
                        bool startKey = tick == s1->tick() && !isFirstSystemKeySig(ks);
                        bool addKey = ks->isChange();
                        if ((startKey || addKey) && !ks->isCustom() && !ks->isAtonal()) {
                              Staff* staff = ks->staff();
                              Key oKey = ks->key();
                              if (!styleB(Sid::concertPitch)) {
                                    Interval i = staff->part()->instrument(tick)->transpose();
                                    // TODO: here we are converting the instrument-transposed key to concert pitch
                                    // ideally, we would figure out how to get the original concert pitch key,
                                    // since the current key may be affected by preferSharpsFlats() setting
                                    // but ultimately it should not matter,
                                    // because undoChangeKeySig() is going to change this again
                                    oKey = transposeKey(oKey, i);
                                    }
                              Key nKey = transposeKey(oKey, interval);
                              KeySigEvent ke = ks->keySigEvent();
                              ke.setKey(nKey);
                              // undoChangeKey handles linked staves/parts and generating new keysigs as needed
                              // it always sets the keysig non-generated
                              // so only call it when needed
                              undoChangeKeySig(staff, tick, ke);
                              }
                        }
                  }
            if (transposeChordNames) {
                  foreach (Element* e, segment->annotations()) {
                        if ((e->type() != ElementType::HARMONY) || (!tracks.contains(e->track())))
                              continue;
                        Harmony* hh  = toHarmony(e);
                        int rootTpc, baseTpc;
                        // undoTransposeHarmony does not do links
                        // because it is also used to handle transposing instruments
                        // and score / parts could be in different concert pitch states
                        for (ScoreElement* se : hh->linkList()) {
                              Harmony* h = toHarmony(se);
                              if (mode == TransposeMode::DIATONICALLY) {
                                    Fraction tick = segment->tick();
                                    Key key = !h->staff() ? Key::C : h->staff()->key(tick);
                                    rootTpc = transposeTpcDiatonicByKey(h->rootTpc(),
                                                transposeInterval, key, trKeys, useDoubleSharpsFlats);
                                    baseTpc = transposeTpcDiatonicByKey(h->baseTpc(),
                                                transposeInterval, key, trKeys, useDoubleSharpsFlats);
                                    }
                              else {
                                    rootTpc = transposeTpc(h->rootTpc(), interval, useDoubleSharpsFlats);
                                    baseTpc = transposeTpc(h->baseTpc(), interval, useDoubleSharpsFlats);
                                    }
                              undoTransposeHarmony(h, rootTpc, baseTpc);
                              }
                        }
                  }
            }
      //
      // create missing key signatures
      //
      if (trKeys && (mode != TransposeMode::DIATONICALLY) && (s1->tick() == Fraction(0,1))) {
            for (int track : tracks) {
                  if (track % VOICES)
                        continue;
                  Segment* seg = firstMeasure()->undoGetSegmentR(SegmentType::KeySig, Fraction(0,1));
                  KeySig* ks = toKeySig(seg->element(track));
                  if (!ks) {
                        ks = new KeySig(this);
                        ks->setTrack(track);
                        Key nKey = transposeKey(Key::C, interval, ks->part()->preferSharpFlat());
                        ks->setKey(nKey);
                        ks->setParent(seg);
                        undoAddElement(ks);
                        }
                  }
            }
      return result;
      }

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeKeys(int staffStart, int staffEnd, const Fraction& ts, const Fraction& tickEnd, const Interval& interval, bool useInstrument, bool flip)
      {
      Fraction tickStart(ts);
      Interval firstInterval = interval;
      Interval segmentInterval = interval;
      if (tickStart < Fraction(0,1))            // -1 and 0 are valid values to indicate start of score
            tickStart = Fraction(0,1);
      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            Staff* st = staff(staffIdx);
            if (st->staffType(tickStart)->group() == StaffGroup::PERCUSSION)
                  continue;

            bool createKey = tickStart.isZero();
            for (Segment* s = firstSegment(SegmentType::KeySig); s; s = s->next1(SegmentType::KeySig)) {
                  if (!s->enabled() || s->tick() < tickStart)
                        continue;
                  if (tickEnd != Fraction(-1,1) && s->tick() >= tickEnd)
                        break;
                  if (useInstrument) {
                        segmentInterval = st->part()->instrument(s->tick())->transpose();
                        if (flip)
                              segmentInterval.flip();
                        }
                  KeySig* ks = toKeySig(s->element(staffIdx * VOICES));
                  if (!ks || ks->generated())
                        continue;
                  if (s->tick().isZero())
                        createKey = false;
                  if (!ks->isCustom() && !ks->isAtonal()) {
                        KeySigEvent ke = st->keySigEvent(s->tick());
                        PreferSharpFlat pref = ks->part()->preferSharpFlat();
                        // TODO: if we are transposing to concert pitch,
                        // then instead of using the part preferSharpFlat() setting,
                        // we should somehow determine the actual concert pitch key
                        // eg, if concert pitch for the score as a whole is B, Eb instruments transpose to Ab
                        // it would be nice if when toggling concert pitch, those instruments were B, not Cb
                        // the code below makes a bold but unwarranted assumption:
                        // if you are using the prefer flats option,
                        // it's because you have sharps in your concert key signatures
                        // (and vice versa)
                        // better would be to check other non-transposing staves,
                        // but ideally we would record and save transposed and concert keys separately
                        // (like we do tpc1 & tpc2 for notes)
                        //if (useInstrument && !flip) {
                        //      if (pref == PreferSharpFlat::FLATS)
                        //            pref = PreferSharpFlat::SHARPS;
                        //      else if (pref == PreferSharpFlat::SHARPS)
                        //            pref = PreferSharpFlat::FLATS;
                        //      }
                        Key nKey = transposeKey(ke.key(), segmentInterval, pref);
                        // remove initial C major key signatures
                        if (nKey == Key::C && s->tick().isZero()) {
                              undo(new RemoveElement(ks));
                              if (s->empty())
                                    undo(new RemoveElement(s));
                              }
                        else {
                              ke.setKey(nKey);
                              undo(new ChangeKeySig(ks, ke, ks->showCourtesy()));
                              }
                        }
                  }
            if (createKey && firstMeasure()) {
                  KeySig* ks = new KeySig(this);
                  ks->setTrack(staffIdx * VOICES);
                  Key nKey = transposeKey(Key::C, firstInterval, ks->part()->preferSharpFlat());
                  KeySigEvent ke;
                  ke.setKey(nKey);
                  ks->setKeySigEvent(ke);
                  Segment* seg = firstMeasure()->undoGetSegmentR(SegmentType::KeySig, Fraction(0,1));
                  seg->setHeader(true);
                  ks->setParent(seg);
                  undoAddElement(ks);
                  }
            }
      }

//---------------------------------------------------------
//   transposeSemitone
//---------------------------------------------------------

void Score::transposeSemitone(int step)
      {
      if (step == 0)
            return;
      if (step > 1)
            step = 1;
      if (step < -1)
            step = -1;

      TransposeDirection dir = step > 0 ? TransposeDirection::UP : TransposeDirection::DOWN;

      int keyType = int(staff(0)->key(Fraction(0,1))) + 7;   // ??

      int intervalListArray[15][2] = {
            // up - down
            { 1, 1 },  // Cb
            { 1, 1 },  // Gb
            { 1, 1 },  // Db
            { 1, 1 },  // Ab
            { 1, 1 },  // Eb
            { 1, 1 },  // Bb
            { 1, 1 },  // F
            { 1, 1 },  // C
            { 1, 1 },  // G
            { 1, 1 },  // D
            { 1, 1 },  // A
            { 1, 1 },  // E
            { 1, 1 },  // B
            { 1, 1 },  // F#
            { 1, 1 }   // C#
            };

      const int interval = intervalListArray[keyType][step > 0 ? 0 : 1];

      if (!transpose(TransposeMode::BY_INTERVAL, dir, Key::C, interval, true, true, false)) {
            qDebug("Score::transposeSemitone: failed");
            // TODO: set error message
            }
      else setSelectionChanged(true);
      }

//---------------------------------------------------------
//   Note::transposeDiatonic
//---------------------------------------------------------

void Note::transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals)
      {
      // compute note current absolute step
      int alter;
      Fraction tick = chord()->segment()->tick();
      Key key       = staff() ? staff()->key(tick) : Key::C;
      int absStep   = pitch2absStepByKey(epitch(), tpc(), key, alter);

      // get pitch and tcp corresponding to unaltered degree for this key
      int newPitch = absStep2pitchByKey(absStep + interval, key);
      int newTpc   = step2tpcByKey((absStep + interval) % STEP_DELTA_OCTAVE, key);

      // if required, transfer original degree alteration to new pitch and tpc
      if (keepAlterations) {
            newPitch += alter;
            newTpc  += alter * TPC_DELTA_SEMITONE;
            }

      // transpose appropriately
      int newTpc1 = TPC_INVALID;
      int newTpc2 = TPC_INVALID;
      Interval v   = staff() ? staff()->part()->instrument(tick)->transpose() : Interval(0);
      if (concertPitch()) {
            v.flip();
            newTpc1 = newTpc;
            newTpc2 = Ms::transposeTpc(newTpc, v, true);
            }
      else {
            newPitch += v.chromatic;
            newTpc1 = Ms::transposeTpc(newTpc, v, true);
            newTpc2 = newTpc;
            }

      // check results are in ranges
      while (newPitch > 127)
            newPitch -= PITCH_DELTA_OCTAVE;
      while (newPitch < 0)
            newPitch += PITCH_DELTA_OCTAVE;
      while (newTpc1 > Tpc::TPC_MAX)
            newTpc1 -= TPC_DELTA_ENHARMONIC;
      while (newTpc1 < Tpc::TPC_MIN)
            newTpc1 += TPC_DELTA_ENHARMONIC;
      while (newTpc2 > Tpc::TPC_MAX)
            newTpc2 -= TPC_DELTA_ENHARMONIC;
      while (newTpc2 < Tpc::TPC_MIN)
            newTpc2 += TPC_DELTA_ENHARMONIC;

      // if required, reduce double alterations
      if (!useDoubleAccidentals) {
            if (newTpc1 >= Tpc::TPC_F_SS)
                  newTpc1 -= TPC_DELTA_ENHARMONIC;
            if (newTpc1 <= Tpc::TPC_B_BB)
                  newTpc1 += TPC_DELTA_ENHARMONIC;
            if (newTpc2 >= Tpc::TPC_F_SS)
                  newTpc2 -= TPC_DELTA_ENHARMONIC;
            if (newTpc2 <= Tpc::TPC_B_BB)
                  newTpc2 += TPC_DELTA_ENHARMONIC;
            }

      // store new data
      score()->undoChangePitch(this, newPitch, newTpc1, newTpc2);
      }

//---------------------------------------------------------
//   transposeDiatonicAlterations
//---------------------------------------------------------

void Score::transposeDiatonicAlterations(TransposeDirection direction)
      {
      // Transpose current selection diatonically (up/down) while keeping degree alterations
      // Note: Score::transpose() absolutely requires valid selection before invocation.
      if (!selection().isNone()) {
            transpose(TransposeMode::DIATONICALLY, direction, Key::C, 1, true, true, true);
            setPlayNote(true); // For when selection is a single note, also playback that note
            setSelectionChanged(true); // This will update the on-screen keyboard
            }
      }

//---------------------------------------------------------
//   transpositionChanged
//---------------------------------------------------------

void Score::transpositionChanged(Part* part, Interval oldV, Fraction tickStart, Fraction tickEnd)
      {
      if (tickStart == Fraction(-1,1))
            tickStart = Fraction(0,1);
      Interval v = part->instrument(tickStart)->transpose();
      v.flip();
      Interval diffV(oldV.chromatic + v.chromatic);

      // transpose keys first
      QList<Score*> scores;
      for (Staff* ls : part->staff(0)->staffList()) {
            // TODO: special handling for linked staves within a score
            // could be useful for capo
            Score* score = ls->score();
            if (scores.contains(score))
                  continue;
            scores.append(score);
            Part* lp = ls->part();
            if (!score->styleB(Sid::concertPitch))
                  score->transposeKeys(lp->startTrack() / VOICES, lp->endTrack() / VOICES, tickStart, tickEnd, diffV);
            }

      // now transpose notes and chord symbols
      for (Segment* s = firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            if (s->tick() < tickStart)
                  continue;
            if (tickEnd != Fraction(-1,1) && s->tick() >= tickEnd)
                  break;
            for (Staff* st : *part->staves()) {
                  if (st->staffType(tickStart)->group() == StaffGroup::PERCUSSION)
                        continue;
                  int t1 = st->idx() * VOICES;
                  int t2 = t1 + VOICES;
                  for (int track = t1; track < t2; ++track) {
                        Element* e = s->element(track);
                        if (e && e->isChord()) {
                              Chord* c = toChord(e);
                              for (Chord* gc : c->graceNotes()) {
                                    for (Note* n : gc->notes()) {
                                          int tpc = transposeTpc(n->tpc1(), v, true);
                                          n->undoChangeProperty(Pid::TPC2, tpc);
                                          }
                                    }
                              for (Note* n : c->notes()) {
                                    int tpc = transposeTpc(n->tpc1(), v, true);
                                    n->undoChangeProperty(Pid::TPC2, tpc);
                                    }
                              }
                        // find chord symbols
                        for (Element* element : s->annotations()) {
                              if (element->track() != track || element->type() != ElementType::HARMONY)
                                    continue;
                              Harmony* h  = toHarmony(element);
                              int rootTpc = transposeTpc(h->rootTpc(), diffV, false);
                              int baseTpc = transposeTpc(h->baseTpc(), diffV, false);
                              for (ScoreElement* scoreElement : h->linkList()) {
                                    if (!scoreElement->score()->styleB(Sid::concertPitch))
                                          undoTransposeHarmony(toHarmony(scoreElement), rootTpc, baseTpc);
                                    }
                              }
                        }
                  }
            }
      }
}

