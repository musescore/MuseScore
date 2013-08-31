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

namespace Ms {

//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

static Interval keydiff2Interval(int oKey, int nKey, TransposeDirection dir)
      {
      static int stepTable[15] = {
            // C  G  D  A  E  B Fis
               0, 4, 1, 5, 2, 6, 3,
            };

      int cofSteps;     // circle of fifth steps
      int diatonic;
      if (nKey > oKey)
            cofSteps = nKey - oKey;
      else
            cofSteps = 12 - (oKey - nKey);
      diatonic = stepTable[(nKey + 7) % 7] - stepTable[(oKey + 7) % 7];
      if (diatonic < 0)
            diatonic += 7;
      diatonic %= 7;
      int chromatic = (cofSteps * 7) % 12;


      if ((dir == TRANSPOSE_CLOSEST) && (chromatic > 6))
            dir = TRANSPOSE_DOWN;

      if (dir == TRANSPOSE_DOWN) {
            chromatic = chromatic - 12;
            diatonic  = diatonic - 7;
            if (diatonic == -7)
                  diatonic = 0;
            if (chromatic == -12)
                  chromatic = 0;
            }
qDebug("TransposeByKey %d -> %d   chromatic %d diatonic %d\n", oKey, nKey, chromatic, diatonic);
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
      if (tpc == INVALID_TPC) // perfect unison & perfect octave
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

// qDebug("transposeTpc tpc %d steps %d semitones %d\n", tpc, steps, semitones);
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
            int p1 = tpc2pitch(step2tpc(step, NATURAL));
            alter  = AccidentalVal(semitones - (p1 - pitch));
            // alter  = p1 + semitones - pitch;
            if (alter < 0) {
                  alter *= -1;
                  alter = 12 - alter;
                  }
            alter %= 12;
	          if (alter > 6)
	               alter -= 12;
	          if (alter > maxAlter)
                  ++steps;
            else if (alter < minAlter)
                  --steps;
            else
                  break;
//            qDebug("  again alter %d steps %d, step %d\n", alter, steps, step);
            }
//      qDebug("  = step %d alter %d  tpc %d\n", step, alter, step2tpc(step, alter));
      return step2tpc(step, AccidentalVal(alter));
      }

//---------------------------------------------------------
//   transposeTpcDiatonicByKey
//
// returns the tpc diatonically transposed by steps, using degrees of given key
// option to keep any alteration tpc had with respect to unaltered corresponding degree of key
// option to enharmonically reduce tpc using double alterations
//---------------------------------------------------------

int transposeTpcDiatonicByKey(int tpc, int steps, int key, bool keepAlteredDegrees, bool useDoubleSharpsFlats)
      {
      if (tpc == INVALID_TPC)
            return tpc;

      // get step for tpc with alteration for key
      int alter;
      int step = tpc2stepByKey(tpc, key, &alter);

      // transpose step and get tpc for step/key
      step += steps;
      int newTpc = step2tpcByKey(step, key);

      // if required, apply alteration to new tpc
      if(keepAlteredDegrees)
            newTpc += alter * TPC_DELTA_SEMITONE;

      // check results are in ranges
      while (newTpc > TPC_MAX)      newTpc   -= TPC_DELTA_ENHARMONIC;
      while (newTpc < TPC_MIN)      newTpc   += TPC_DELTA_ENHARMONIC;

      // if required, reduce double alterations
      if(!useDoubleSharpsFlats) {
            if(newTpc >= TPC_F_SS)  newTpc   -= TPC_DELTA_ENHARMONIC;
            if(newTpc <= TPC_B_BB)  newTpc   += TPC_DELTA_ENHARMONIC;
            }

      return newTpc;
      }

//---------------------------------------------------------
//   transposeStaff
//---------------------------------------------------------

void Score::cmdTransposeStaff(int staffIdx, Interval interval, bool useDoubleSharpsFlats)
      {
      if (staff(staffIdx)->staffType()->group() == PERCUSSION_STAFF_GROUP)
            return;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      transposeKeys(staffIdx, staffIdx+1, 0, lastSegment()->tick(), interval);

      for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
           for (int st = startTrack; st < endTrack; ++st) {
                  Element* e = segment->element(st);
                  if (!e || e->type() != Element::CHORD)
                      continue;

                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  foreach(Note* n, nl)
                      transpose(n, interval, useDoubleSharpsFlats);
                  }
            foreach (Element* e, segment->annotations()) {
                  if ((e->type() != Element::HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                        continue;
                  Harmony* h  = static_cast<Harmony*>(e);
                  int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                  int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                  undoTransposeHarmony(h, rootTpc, baseTpc);
                  }
            }
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose(Note* n, Interval interval, bool useDoubleSharpsFlats)
      {
      int npitch;
      int ntpc;
      transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc, interval,
        useDoubleSharpsFlats);
      undoChangePitch(n, npitch, ntpc, n->line()/*, n->fret(), n->string()*/);
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose(int mode, TransposeDirection direction, int transposeKey,
  int transposeInterval,
   bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats)
      {
      bool rangeSelection = selection().state() == SEL_RANGE;
      int startStaffIdx = 0;
      int startTick     = 0;
      if (rangeSelection) {
            startStaffIdx = selection().staffStart();
            startTick     = selection().tickStart();
            }
      KeyList* km = staff(startStaffIdx)->keymap();

      Interval interval;
      if (mode != TRANSPOSE_DIATONICALLY) {
            if (mode == TRANSPOSE_BY_KEY) {
                  // calculate interval from "transpose by key"
//                  km       = staff(startStaffIdx)->keymap();
                  int oKey = km->key(startTick).accidentalType();
                  interval = keydiff2Interval(oKey, transposeKey, direction);
                  }
            else {
                  interval = intervalList[transposeInterval];
                  if (direction == TRANSPOSE_DOWN)
                        interval.flip();
                  }

            if (!rangeSelection) {
                  trKeys = false;
                  }
            bool fullOctave = (interval.chromatic % 12) == 0;
            if (fullOctave && (mode != TRANSPOSE_BY_KEY)) {
                  trKeys = false;
                  transposeChordNames = false;
                  }
            }
      else                          // diatonic transposition
            if (direction == TRANSPOSE_DOWN)
                  transposeInterval *= -1;

      if (_selection.state() == SEL_LIST) {
            foreach(Element* e, _selection.elements()) {
                  if (e->staff()->staffType()->group() == PERCUSSION_STAFF_GROUP)
                        continue;
                  if (e->type() == Element::NOTE) {
                        Note* note = static_cast<Note*>(e);
                        if (mode == TRANSPOSE_DIATONICALLY)
                              note->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                        else
                              transpose(note, interval, useDoubleSharpsFlats);
                        }
                  else if ((e->type() == Element::HARMONY) && transposeChordNames) {
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc, baseTpc;
                        if (mode == TRANSPOSE_DIATONICALLY) {
                              int tick = 0;
                              if (h->parent()->type() == Element::SEGMENT)
                                    tick = static_cast<Segment*>(h->parent())->tick();
                              else if (h->parent()->type() == Element::FRET_DIAGRAM
                                 && h->parent()->parent()->type() == Element::SEGMENT) {
                                    tick = static_cast<Segment*>(h->parent()->parent())->tick();
                                    }
                              int key = !h->staff() ? KEY_C : h->staff()->keymap()->key(tick).accidentalType();
                              rootTpc = transposeTpcDiatonicByKey(h->rootTpc(),
                                          transposeInterval, key, trKeys, useDoubleSharpsFlats);
                              baseTpc = transposeTpcDiatonicByKey(h->baseTpc(),
                                          transposeInterval, key, trKeys, useDoubleSharpsFlats);
                        }
                        else {
                              rootTpc = transposeTpc(h->rootTpc(), interval, false);
                              baseTpc = transposeTpc(h->baseTpc(), interval, false);
                              }
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  else if ((e->type() == Element::KEYSIG) && mode != TRANSPOSE_DIATONICALLY && trKeys) {
                        KeySig* ks = static_cast<KeySig*>(e);
                        KeySigEvent key  = km->key(ks->tick());
                        KeySigEvent okey = km->key(ks->tick() - 1);
                        key.setNaturalType(okey.accidentalType());
                        undo(new ChangeKeySig(ks, key, ks->showCourtesy(),
                           ks->showNaturals()));
                        }
                  }
            return;
            }

      int startTrack = _selection.staffStart() * VOICES;
      int endTrack   = _selection.staffEnd() * VOICES;

      for (Segment* segment = _selection.startSegment(); segment && segment != _selection.endSegment(); segment = segment->next1()) {
            for (int st = startTrack; st < endTrack; ++st) {
                  if (staff(st/VOICES)->staffType()->group() == PERCUSSION_STAFF_GROUP)
                        continue;
                  Element* e = segment->element(st);
                  if (!e || e->type() != Element::CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  for (Note* n : nl) {
                        if (mode == TRANSPOSE_DIATONICALLY)
                              n->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                        else
                              transpose(n, interval, useDoubleSharpsFlats);
                        }
                  for (Chord* g : chord->graceNotes()) {
                        for (Note* n : g->notes()) {
                              if (mode == TRANSPOSE_DIATONICALLY)
                                    n->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                              else
                                    transpose(n, interval, useDoubleSharpsFlats);
                              }
                        }
                  }
            if (transposeChordNames) {
                  foreach (Element* e, segment->annotations()) {
                        if ((e->type() != Element::HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                              continue;
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc, baseTpc;
                        if (mode == TRANSPOSE_DIATONICALLY) {
                              int tick = segment->tick();
                              int key = !h->staff() ? KEY_C : h->staff()->keymap()->key(tick).accidentalType();
                              rootTpc = transposeTpcDiatonicByKey(h->rootTpc(),
                                          transposeInterval, key, trKeys, useDoubleSharpsFlats);
                              baseTpc = transposeTpcDiatonicByKey(h->baseTpc(),
                                          transposeInterval, key, trKeys, useDoubleSharpsFlats);
                              }
                        else {
                              rootTpc = transposeTpc(h->rootTpc(), interval, false);
                              baseTpc = transposeTpc(h->baseTpc(), interval, false);
                              }
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  }
            }
      if (trKeys && mode != TRANSPOSE_DIATONICALLY) {
            transposeKeys(_selection.staffStart(), _selection.staffEnd(),
               _selection.tickStart(), _selection.tickEnd(), interval);
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, const Interval& interval)
      {
      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            if (staff(staffIdx)->staffType()->group() == PERCUSSION_STAFF_GROUP)
                  continue;

            KeyList* km = staff(staffIdx)->keymap();
            for (auto ke = km->lower_bound(tickStart);
                  ke != km->lower_bound(tickEnd); ++ke) {
                  KeySigEvent oKey  = ke->second;
                  int tick  = ke->first;
                  int nKeyType = transposeKey(oKey.accidentalType(), interval);
                  KeySigEvent nKey;
                  nKey.setAccidentalType(nKeyType);
                  (*km)[tick] = nKey;
                  // undoChangeKey(staff(staffIdx), tick, oKey, nKey);
                  }
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  if (s->segmentType() != Segment::SegKeySig)
                        continue;
                  if (s->tick() < tickStart)
                        continue;
                  if (s->tick() >= tickEnd)
                        break;
                  KeySig* ks = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                  if (ks) {
                        KeySigEvent key  = km->key(s->tick());
                        KeySigEvent okey = km->key(s->tick() - 1);
                        key.setNaturalType(okey.accidentalType());
                        undo(new ChangeKeySig(ks, key, ks->showCourtesy(),
                           ks->showNaturals()));
                        }
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

      TransposeDirection dir = step > 0 ? TRANSPOSE_UP : TRANSPOSE_DOWN;


      KeyList* km = staff(0)->keymap();
      KeySigEvent key = km->lower_bound(0)->second;
      int keyType = key.accidentalType() + 7;

      int intervalList[15][2] = {
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

      int interval = intervalList[keyType][step > 0 ? 0 : 1];

      cmdSelectAll();
      startCmd();
      transpose(TRANSPOSE_BY_INTERVAL, dir, 0, interval, true, true, false);
      deselectAll();
      setLayoutAll(true);
      endCmd();
      }

//---------------------------------------------------------
//   Note::transposeDiatonic
//---------------------------------------------------------

void Note::transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals)
      {
      // compute note current absolute step
      int   alter;
      int   tick        = chord()->segment()->tick();
      int   key         = !staff() ? KEY_C : staff()->keymap()->key(tick).accidentalType();
      int   absStep     = pitch2absStepByKey(pitch(), tpc(), key, &alter);

      // get pitch and tcp corresponding to unaltered degree for this key
      int   newPitch    = absStep2pitchByKey(absStep + interval, key);
      int   newTpc      = step2tpcByKey((absStep+interval) % STEP_DELTA_OCTAVE, key);

      // if required, transfer original degree alteration to new pitch and tpc
      if(keepAlterations) {
            newPitch += alter;
            newTpc   += alter * TPC_DELTA_SEMITONE;
            }

      // check results are in ranges
      while (newPitch > 127)        newPitch -= PITCH_DELTA_OCTAVE;
      while (newPitch < 0)          newPitch += PITCH_DELTA_OCTAVE;
      while (newTpc > TPC_MAX)      newTpc   -= TPC_DELTA_ENHARMONIC;
      while (newTpc < TPC_MIN)      newTpc   += TPC_DELTA_ENHARMONIC;

      // if required, reduce double alterations
      if(!useDoubleAccidentals) {
            if(newTpc >= TPC_F_SS)  newTpc   -= TPC_DELTA_ENHARMONIC;
            if(newTpc <= TPC_B_BB)  newTpc   += TPC_DELTA_ENHARMONIC;
            }

      // store new data
      score()->undoChangePitch(this, newPitch, newTpc, line()+interval);
      }
}

