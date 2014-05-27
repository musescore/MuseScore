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

#ifndef __PITCHSPELLING_H__
#define __PITCHSPELLING_H__

#include "mscore.h"

namespace Ms {

class MidiNote;
class Note;
class Event;

const int   INVALID_PITCH      = -1;

// a list of tpc's, with legal ranges
enum Tpc : signed char {
      INVALID = -2,
      F_BB, C_BB, G_BB, D_BB, A_BB, E_BB, B_BB,
      F_B,  C_B,  G_B,  D_B,  A_B,  E_B,  B_B,
      F,    C,    G,    D,    A,    E,    B,
      F_S,  C_S,  G_S,  D_S,  A_S,  E_S,  B_S,
      F_SS, C_SS, G_SS, D_SS, A_SS, E_SS, B_SS,
      MIN = F_BB,
      MAX = B_SS
      };

const int   TPC_DELTA_SEMITONE      = 7;  // the delta in tpc value to go 1 semitone up or down
const int   TPC_DELTA_ENHARMONIC    = 12; // the delta in tpc value to reach the next (or prev) enharmonic spelling
const int   TPC_FIRST_STEP          = 3;  // the step of the first valid tpc (= F = step 3)
const int   PITCH_DELTA_OCTAVE      = 12; // the delta in pitch value to go 1 octave up or down
const int   STEP_DELTA_OCTAVE       = 7;  // the number of steps in an octave
const int   STEP_DELTA_TPC          = 4;  // the number of steps in a tpc step (= a fifth = 4 steps)

//---------------------------------------------------------
//   pitch2tpc
//    Returns a default tpc for a given midi pitch.
//    Midi pitch 60 is middle C.
//---------------------------------------------------------

// pitch2tpc(pitch) replaced by pitch2tpc(pitch, Key::KEY_C, Prefer::NEAREST)

enum class Prefer : char { FLATS=8, NEAREST=11, SHARPS=13 };
enum class NoteSpellingType : char { STANDARD = 0, GERMAN, SOLFEGGIO };

extern int pitch2tpc(int pitch, int key, Prefer prefer);

extern void spell(QList<Event>& notes, int);
extern void spell(QList<Note*>& notes);
extern int computeWindow(const QList<Note*>& notes, int start, int end);
extern int tpc(int idx, int pitch, int opt);
extern QString tpc2name(int tpc, NoteSpellingType spelling, bool lowerCase);
extern void tpc2name(int tpc, NoteSpellingType spelling, bool lowerCase, QString& s, QString& acc);
extern void tpc2name(int tpc, NoteSpellingType spelling, bool lowerCase, QString& s, int& acc);
extern int step2tpc(const QString& stepName, AccidentalVal alter);
extern int step2tpc(int step);
extern int step2tpc(int step, AccidentalVal alter);
extern int step2tpcByKey(int step, int key);
extern int tpc2pitch(int tpc);
extern int tpc2step(int tpc);
extern int tpc2stepByKey(int tpc, int key, int* pAlter);
extern int tpc2alterByKey(int tpc, int key);
extern int pitch2absStepByKey(int pitch, int tpc, int key, int* pAlter);
extern int absStep2pitchByKey(int step, int key);

//---------------------------------------------------------
//   tpc2alter
//---------------------------------------------------------

inline static AccidentalVal tpc2alter(int tpc) {
      return AccidentalVal(((tpc+1) / 7) - 2);
      }

extern QString tpc2stepName(int tpc);
extern bool tpcIsValid(int val);


}     // namespace Ms
#endif

