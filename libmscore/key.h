//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __KEY_H__
#define __KEY_H__

#include "mscore.h"

namespace Ms {

class Xml;
class Score;
class XmlReader;

enum class Key : signed char {
      C_B=-7,
      G_B, D_B, A_B, E_B, B_B, F,   C,
      G,   D,   A,   E,   B,   F_S, C_S,
      MIN = Key::C_B,
      MAX = Key::C_S,
      INVALID = Key::MIN - 1,
      NUM_OF = Key::MAX - Key::MIN + 1
      };

// the delta in key value to reach the next (or prev) enharmonically equivalent key:
static const int KEY_DELTA_ENHARMONIC = 12;

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent {
      int _accidentalType { 0 };          // -7 -> +7
      int _customType     { 0 };
      bool _custom        { false };
      bool _invalid       { true };

      void enforceLimits();

   public:
      KeySigEvent() {}
      KeySigEvent(int at);

      bool isValid() const { return !_invalid; }
      bool operator==(const KeySigEvent& e) const;
      bool operator!=(const KeySigEvent& e) const;
      void setCustomType(int v);
      void setAccidentalType(int v);
      void print() const;

      int accidentalType() const { return _accidentalType; }
      int customType() const     { return _customType;     }
      bool custom() const        { return _custom;         }
      bool invalid() const       { return _invalid;        }
      void initFromSubtype(int);    // for backward compatibility
      void initLineList(char*);
      };

//---------------------------------------------------------
//   AccidentalState
///   Contains a state for every absolute staff line.
//---------------------------------------------------------

static const int TIE_CONTEXT = 0x10;

class AccidentalState {
      uchar state[75];    // (0 -- 4) | TIE_CONTEXT

   public:
      AccidentalState() {}
      void init(const KeySigEvent&);
      AccidentalVal accidentalVal(int line) const {
            Q_ASSERT(line >= 0 && line < 75);
            return AccidentalVal((state[line] & 0x0f) - 2);
            }
      bool tieContext(int line) const {
            Q_ASSERT(line >= 0 && line < 75);
            return state[line] & TIE_CONTEXT;
            }
      void setAccidentalVal(int line, AccidentalVal val, bool tieContext = false) {
            Q_ASSERT(line >= 0 && line < 75);
            Q_ASSERT(val >= AccidentalVal::FLAT2 && val <= AccidentalVal::SHARP2);
            state[line] = (int(val) + 2) | (tieContext ? TIE_CONTEXT : 0);
            }
      };

struct Interval;
extern int transposeKey(int oldKey, const Interval&);


}     // namespace Ms
#endif

