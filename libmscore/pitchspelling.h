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
enum class Key;

const int   INVALID_PITCH      = -1;

#if 0
enum {
      STEP_NONE      = -1,
      STEP_C,
      STEP_D,
      STEP_E,
      STEP_F,
      STEP_G,
      STEP_A,
      STEP_B
      };
#endif

// a list of tpc's, with legal ranges, not really an enum, so no way to cnvert into a class
enum Tpc : signed char {
      TPC_INVALID = -9,
      TPC_F_BBB, TPC_C_BBB, TPC_G_BBB, TPC_D_BBB, TPC_A_BBB, TPC_E_BBB, TPC_B_BBB,
      TPC_F_BB,  TPC_C_BB,  TPC_G_BB,  TPC_D_BB,  TPC_A_BB,  TPC_E_BB,  TPC_B_BB,
      TPC_F_B,   TPC_C_B,   TPC_G_B,   TPC_D_B,   TPC_A_B,   TPC_E_B,   TPC_B_B,
      TPC_F,     TPC_C,     TPC_G,     TPC_D,     TPC_A,     TPC_E,     TPC_B,
      TPC_F_S,   TPC_C_S,   TPC_G_S,   TPC_D_S,   TPC_A_S,   TPC_E_S,   TPC_B_S,
      TPC_F_SS,  TPC_C_SS,  TPC_G_SS,  TPC_D_SS,  TPC_A_SS,  TPC_E_SS,  TPC_B_SS,
      TPC_F_SSS, TPC_C_SSS, TPC_G_SSS, TPC_D_SSS, TPC_A_SSS, TPC_E_SSS, TPC_B_SSS,
      TPC_MIN = TPC_F_BBB,
      TPC_MAX = TPC_B_SSS
      };

const int   TPC_DELTA_SEMITONE      = 7;  // the delta in tpc value to go 1 semitone up or down
const int   TPC_DELTA_ENHARMONIC    = 12; // the delta in tpc value to reach the next (or prev) enharmonic spelling
//const int   TPC_FIRST_STEP          = 3;  // the step of the first valid tpc (= F = step 3)
const int   PITCH_DELTA_OCTAVE      = 12; // the delta in pitch value to go 1 octave up or down
const int   STEP_DELTA_OCTAVE       = 7;  // the number of steps in an octave
//const int   STEP_DELTA_TPC          = 4;  // the number of steps in a tpc step (= a fifth = 4 steps)
const int   TPCS_PER_STEP           = (Tpc::TPC_MAX - Tpc::TPC_MIN + 1) / STEP_DELTA_OCTAVE;

//---------------------------------------------------------
//   pitch2tpc
//    Returns a default tpc for a given midi pitch.
//    Midi pitch 60 is middle C.
//---------------------------------------------------------

// pitch2tpc(pitch) replaced by pitch2tpc(pitch, Key::C, Prefer::NEAREST)

enum class Prefer : char { FLATS=8, NEAREST=11, SHARPS=13 };
enum class NoteSpellingType : char { STANDARD = 0, GERMAN, GERMAN_PURE, SOLFEGGIO, FRENCH };
enum class NoteCaseType : signed char { AUTO = -1, CAPITAL = 0, LOWER, UPPER };

extern int pitch2tpc(int pitch, Key, Prefer prefer);

extern int computeWindow(const std::vector<Note*>& notes, int start, int end);
extern int tpc(int idx, int pitch, int opt);
extern QString tpc2name(int tpc, NoteSpellingType spelling, NoteCaseType noteCase, bool explicitAccidental = false);
extern void tpc2name(int tpc, NoteSpellingType noteSpelling, NoteCaseType noteCase, QString& s, QString& acc, bool explicitAccidental = false);
extern void tpc2name(int tpc, NoteSpellingType noteSpelling, NoteCaseType noteCase, QString& s, AccidentalVal& acc);
extern int step2tpc(const QString& stepName, AccidentalVal alter);
extern int step2tpc(int step);
extern int step2tpc(int step, AccidentalVal alter);
extern int step2tpcByKey(int step, Key);
extern int tpc2pitch(int tpc);
extern int tpc2step(int tpc);
extern int tpc2stepByKey(int tpc, Key, int& alter);
extern int tpc2alterByKey(int tpc, Key);
extern int pitch2absStepByKey(int pitch, int tpc, Key, int& alter);
extern int absStep2pitchByKey(int step, Key);
extern int tpc2degree(int tpc, Key key);
extern int tpcInterval(int startTpc, int interval, int alter);
extern int step2pitchInterval(int step, int alter);
extern int function2Tpc(const QString& s, Key key);

//---------------------------------------------------------
//   tpc2alter
//---------------------------------------------------------

inline static AccidentalVal tpc2alter(int tpc) {
      return AccidentalVal(((tpc - Tpc::TPC_MIN) / TPC_DELTA_SEMITONE) + int(AccidentalVal::MIN));
      }

extern QString tpc2stepName(int tpc);
extern bool tpcIsValid(int val);
inline bool pitchIsValid(int pitch) { return pitch >= 0 && pitch <= 127; }

}     // namespace Ms
#endif

