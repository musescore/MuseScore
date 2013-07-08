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

#ifndef __KEY_H__
#define __KEY_H__

#include "mscore.h"

namespace Ms {

class Xml;
class Score;
class XmlReader;

enum {
      KEY_C_B=-7,
      KEY_G_B, KEY_D_B, KEY_A_B, KEY_E_B, KEY_B_B, KEY_F,   KEY_C,
      KEY_G,   KEY_D,   KEY_A,   KEY_E,   KEY_B,   KEY_F_S, KEY_C_S,
      KEY_MIN = KEY_C_B,
      KEY_MAX = KEY_C_S,
      INVALID_KEY = KEY_MIN-1,
      NUM_OF_KEYS = KEY_MAX - KEY_MIN + 1
      };

// the delta in key value to reach the next (or prev) enharmonically equivalent key:
static const int KEY_DELTA_ENHARMONIC = 12;

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent {
      int _accidentalType;          // -7 -> +7
      int _naturalType;
      int _customType;
      bool _custom;
      bool _invalid;
      void enforceLimits();

   public:
      KeySigEvent();
      KeySigEvent(int);

      bool isValid() const { return !_invalid; }
      bool operator==(const KeySigEvent& e) const;
      bool operator!=(const KeySigEvent& e) const;
      void setCustomType(int v);
      void setAccidentalType(int v);
      void print() const;

      int accidentalType() const { return _accidentalType; }
      int naturalType() const    { return _naturalType;    }
      void setNaturalType(int v) { _naturalType = v;       }
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
            Q_ASSERT(val >= -2 && val <= 2);
            state[line] = (val + 2) | (tieContext ? TIE_CONTEXT : 0);
            }
      };

//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

class KeyList : public std::map<const int, KeySigEvent> {
   public:
      KeyList() {}
      KeySigEvent key(int tick) const;
      int nextKeyTick(int tick) const;
      void read(XmlReader&, Score*);
      void write(Xml&, const char* name) const;
      };

struct Interval;
extern int transposeKey(int oldKey, const Interval&);


}     // namespace Ms
#endif

