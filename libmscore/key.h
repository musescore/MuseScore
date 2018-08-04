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

#ifndef __KEY__H__
#define __KEY__H__

namespace Ms {

class XmlWriter;
class Score;
class XmlReader;
enum class AccidentalVal : signed char;
enum class ClefType : signed char;

//---------------------------------------------------------
//   Key
//---------------------------------------------------------

enum class Key {
      C_B = -7,
      G_B,
      D_B,
      A_B,
      E_B,
      B_B,
      F,
      C,    // == 0
      G,
      D,
      A,
      E,
      B,
      F_S,
      C_S,
      MIN     = Key::C_B,
      MAX     = Key::C_S,
      INVALID = Key::MIN - 1,
      NUM_OF  = Key::MAX - Key::MIN + 1,
      DELTA_ENHARMONIC = 12
      };

//---------------------------------------------------------
//   KeyMode
//---------------------------------------------------------

enum class KeyMode {
      UNKNOWN = -1,
      NONE,
      MAJOR,
      MINOR
      };

static inline bool operator< (Key a, Key b) { return static_cast<int>(a) < static_cast<int>(b); }
static inline bool operator> (Key a, Key b) { return static_cast<int>(a) > static_cast<int>(b); }
static inline bool operator> (Key a, int b) { return static_cast<int>(a) > b; }
static inline bool operator< (Key a, int b) { return static_cast<int>(a) < b; }
static inline bool operator== (const Key a, const Key b) { return int(a) == int(b); }  
static inline bool operator!= (const Key a, const Key b) { return static_cast<int>(a) != static_cast<int>(b); }
static inline Key operator+= (Key& a, const Key& b) { return a = Key(static_cast<int>(a) + static_cast<int>(b)); }
static inline Key operator-= (Key& a, const Key& b) { return a = Key(static_cast<int>(a) - static_cast<int>(b)); }

enum class SymId;

//---------------------------------------------------------
//   KeySym
//    position of one symbol in KeySig
//---------------------------------------------------------

struct KeySym {
      SymId sym;
      QPointF spos;     // position in spatium units
      QPointF pos;      // actual pixel position on screen (set by layout)
      };

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent {
      Key _key            { Key::INVALID     };          // -7 -> +7
      KeyMode _mode       { KeyMode::UNKNOWN };
      bool _custom        { false            };
      QList<KeySym> _keySymbols;

      void enforceLimits();

   public:
      KeySigEvent() {}
      KeySigEvent(const KeySigEvent&);

      bool operator==(const KeySigEvent& e) const;

      void setKey(Key v);
      void print() const;

      Key key() const            { return _key;                    }
      KeyMode mode() const       { return _mode;                   }
      void setMode(KeyMode m)    { _mode = m;                      }
      bool custom() const        { return _custom;                 }
      void setCustom(bool val)   { _custom = val; _key = Key::C;   }
      bool isValid() const       { return _key != Key::INVALID;    }
      bool isAtonal() const      { return _mode == KeyMode::NONE;  }
      void initFromSubtype(int);    // for backward compatibility
      QList<KeySym>& keySymbols()             { return _keySymbols; }
      const QList<KeySym>& keySymbols() const { return _keySymbols; }
      };

//---------------------------------------------------------
//   AccidentalState
///   Contains a state for every absolute staff line.
//---------------------------------------------------------

static const int TIE_CONTEXT = 0x10;
static const int MIN_ACC_STATE = 0;
static const int MAX_ACC_STATE = 75;

class AccidentalState {
      uchar state[MAX_ACC_STATE];    // (0 -- 4) | TIE_CONTEXT

   public:
      AccidentalState() {}
      void init(Key key);
      void init(const KeySigEvent&, ClefType);
      AccidentalVal accidentalVal(int line, bool &error) const;
      AccidentalVal accidentalVal(int line) const;
      bool tieContext(int line) const;
      void setAccidentalVal(int line, AccidentalVal val, bool tieContext = false);
      };

struct Interval;
extern Key transposeKey(Key oldKey, const Interval&);


}     // namespace Ms
#endif

