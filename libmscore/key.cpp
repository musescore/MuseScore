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

#include "key.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "pitchspelling.h"
#include "keylist.h"

namespace Ms {

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

KeySigEvent::KeySigEvent(const KeySigEvent& k)
      {
      _key        = k._key;
      _mode       = k._mode;
      _custom     = k._custom;
      _keySymbols = k._keySymbols;
      }

//---------------------------------------------------------
//   enforceLimits - ensure _key
//   is within acceptable limits (-7 .. +7).
//   see KeySig::layout()
//---------------------------------------------------------

void KeySigEvent::enforceLimits()
      {
      const char* msg = 0;
      if (_key < Key::MIN) {
            _key = Key::MIN;
            msg = "key < -7";
            }
      else if (_key > Key::MAX) {
            _key = Key::MAX;
            msg = "key > 7";
            }
      if (msg)
            qDebug("KeySigEvent: %s", msg);
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void KeySigEvent::print() const
      {
      qDebug("<KeySigEvent: ");
      if (!isValid())
            qDebug("invalid>");
      else {
            if (isAtonal())
                  qDebug("atonal>");
            else if (custom())
                  qDebug("custom>");
            else
                  qDebug("accidental %d>", int(_key));
            }
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeySigEvent::setKey(Key v)
      {
      _key      = v;
      _custom   = false;
      enforceLimits();
      }

//---------------------------------------------------------
//   KeySigEvent::operator==
//---------------------------------------------------------

bool KeySigEvent::operator==(const KeySigEvent& e) const
      {
      if (e._custom != _custom || e._mode != _mode)
            return false;
      if (_custom && !isAtonal()) {
            if (e._keySymbols.size() != _keySymbols.size())
                  return false;
            for (int i = 0; i < _keySymbols.size(); ++i) {
                  if (e._keySymbols[i].sym != _keySymbols[i].sym)
                        return false;
                  // TODO: position matters
                  }
            return true;
            }
      return e._key == _key;
      }

//---------------------------------------------------------
//   initLineList
//    preset lines list with accidentals for given key
//---------------------------------------------------------

void AccidentalState::init(Key key)
      {
//      memset(state, 2, 74);
      memset(state, 2, 75);
      for (int octave = 0; octave < 11; ++octave) {
            if (key > 0) {
                  for (int i = 0; i < int(key); ++i) {
                        int idx = tpc2step(20 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = 1 + 2;
                        }
                  }
            else {
                  for (int i = 0; i > int(key); --i) {
                        int idx = tpc2step(12 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = -1 + 2;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   transposeKey
//---------------------------------------------------------

Key transposeKey(Key key, const Interval& interval)
      {
      int tpc = int(key) + 14;
      tpc     = transposeTpc(tpc, interval, false);
      // check for valid key sigs
      if (tpc > 21)
            tpc -= 12; // no more than 7 sharps in keysig
      if (tpc < 7)
            tpc += 12; // no more than 7 flats in keysig
      return Key(tpc - 14);
      }

//---------------------------------------------------------
//   initFromSubtype
//    for backward compatibility
//---------------------------------------------------------

void KeySigEvent::initFromSubtype(int st)
      {
      union U {
            int subtype;
            struct {
                  int _key:4;
                  int _naturalType:4;
                  unsigned _customType:16;
                  bool _custom : 1;
                  bool _invalid : 1;
                  };
            };
      U a;
      a.subtype       = st;
      _key            = Key(a._key);
//      _customType     = a._customType;
      _custom         = a._custom;
      if (a._invalid)
            _key = Key::INVALID;
      enforceLimits();
      }

//---------------------------------------------------------
//   accidentalVal
//---------------------------------------------------------

AccidentalVal AccidentalState::accidentalVal(int line, bool &error) const
      {
      if (line < MIN_ACC_STATE || line >= MAX_ACC_STATE) {
            error = true;
            return AccidentalVal::NATURAL;
            }
      return AccidentalVal((state[line] & 0x0f) - 2);
      }

AccidentalVal AccidentalState::accidentalVal(int line) const
      {
      Q_ASSERT(line >= MIN_ACC_STATE && line < MAX_ACC_STATE);
      bool error = false;
      return accidentalVal(line, error);
      }

//---------------------------------------------------------
//   tieContext
//---------------------------------------------------------

bool AccidentalState::tieContext(int line) const
      {
      Q_ASSERT(line >= MIN_ACC_STATE && line < MAX_ACC_STATE);
      return state[line] & TIE_CONTEXT;
      }

//---------------------------------------------------------
//   setAccidentalVal
//---------------------------------------------------------

void AccidentalState::setAccidentalVal(int line, AccidentalVal val, bool tieContext)
      {
      Q_ASSERT(line >= MIN_ACC_STATE && line < MAX_ACC_STATE);
      // casts needed to work around a bug in Xcode 4.2 on Mac, see #25910
      Q_ASSERT(int(val) >= int(AccidentalVal::FLAT2) && int(val) <= int(AccidentalVal::SHARP2));
      state[line] = (int(val) + 2) | (tieContext ? TIE_CONTEXT : 0);
      }
}

