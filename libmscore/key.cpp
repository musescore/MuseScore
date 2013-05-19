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

namespace Ms {

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

KeySigEvent::KeySigEvent()
      {
      _accidentalType = 0;
      _naturalType    = 0;
      _customType     = 0;
      _custom         = false;
      _invalid        = true;
      }

KeySigEvent::KeySigEvent(int n)
      {
      _accidentalType = n;
      _naturalType    = 0;
      _customType     = 0;
      _custom         = false;
      _invalid        = false;
      enforceLimits();
      }

//---------------------------------------------------------
//   enforceLimits - ensure _accidentalType and _naturalType
//   are within acceptable limits (-7 .. +7).
//   see KeySig::layout()
//---------------------------------------------------------

void KeySigEvent::enforceLimits()
      {
      const char* msg = 0;
      if (_accidentalType < -7) {
            _accidentalType = -7;
            msg = "accidentalType < -7";
            }
      else if (_accidentalType > 7) {
            _accidentalType = 7;
            msg = "accidentalType > 7";
            }
      if (_naturalType < -7) {
            _naturalType = -7;
            msg = "naturalType < -7";
            }
      else if (_naturalType > 7) {
            _naturalType = 7;
            msg = "naturalType > 7";
            }
      if (msg)
            qDebug("KeySigEvent: %s\n", msg);
      }

//---------------------------------------------------------
//   setCustomType
//---------------------------------------------------------

void KeySigEvent::setCustomType(int v)
      {
      _accidentalType = 0;
      _customType     = v;
      _custom         = true;
      _invalid        = false;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void KeySigEvent::print() const
      {
      qDebug("<KeySigEvent: ");
      if (_invalid)
            qDebug("invalid>");
      else {
            if (_custom)
                  qDebug("nat %d custom %d>", _naturalType, _customType);
            else
                  qDebug("nat %d accidental %d>", _naturalType, _accidentalType);
            }
      }

//---------------------------------------------------------
//   setAccidentalType
//---------------------------------------------------------

void KeySigEvent::setAccidentalType(int v)
      {
      _accidentalType = v;
      _custom         = false;
      _invalid        = false;
      enforceLimits();
      }

//---------------------------------------------------------
//   KeySigEvent::operator==
//---------------------------------------------------------

bool KeySigEvent::operator==(const KeySigEvent& e) const
      {
      if ((e._invalid != _invalid) || (e._custom != _custom))
            return false;
      if (_custom)
            return e._customType == _customType;
      else
            return e._accidentalType == _accidentalType;
      }

//---------------------------------------------------------
//   KeySigEvent::operator!=
//---------------------------------------------------------

bool KeySigEvent::operator!=(const KeySigEvent& e) const
      {
      if ((e._invalid != _invalid) || (e._custom != _custom))
            return true;
      if (_custom)
            return e._customType != _customType;
      else
            return e._accidentalType != _accidentalType;
      }

//---------------------------------------------------------
//   initLineList
//    preset lines list with accidentals for given key
//---------------------------------------------------------

void AccidentalState::init(const KeySigEvent& ks)
      {
      int type = ks.accidentalType();

      memset(state, 2, 74);
      for (int octave = 0; octave < 11; ++octave) {
            if (type > 0) {
                  for (int i = 0; i < type; ++i) {
                        int idx = tpc2step(20 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = 1 + 2;
                        }
                  }
            else {
                  for (int i = 0; i > type; --i) {
                        int idx = tpc2step(12 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = -1 + 2;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   key
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

KeySigEvent KeyList::key(int tick) const
      {
      if (empty())
            return KeySigEvent();
      auto i = upper_bound(tick);
      if (i == begin())
            return KeySigEvent();
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return 0, if no such a key sig
//---------------------------------------------------------

int KeyList::nextKeyTick(int tick) const
      {
      if (empty())
            return 0;
      auto i = upper_bound(tick+1);
      if (i == end())
            return 0;
      return i->first;
      }

//---------------------------------------------------------
//   KeyList::write
//---------------------------------------------------------

void KeyList::write(Xml& xml, const char* name) const
      {
      xml.stag(name);
      for (auto i = begin(); i != end(); ++i) {
            if (i->second.custom())
                  xml.tagE("key tick=\"%d\" custom=\"%d\"", i->first, i->second.customType());
            else
                  xml.tagE("key tick=\"%d\" idx=\"%d\"", i->first, i->second.accidentalType());
            }
      xml.etag();
      }

//---------------------------------------------------------
//   KeyList::read
//---------------------------------------------------------

void KeyList::read(XmlReader& e, Score* cs)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "key") {
                  KeySigEvent ke;
                  int tick = e.intAttribute("tick", 0);
                  if (e.hasAttribute("custom"))
                        ke.setCustomType(e.intAttribute("custom"));
                  else
                        ke.setAccidentalType(e.intAttribute("idx"));
                  (*this)[cs->fileDivision(tick)] = ke;
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   transposeKey
//---------------------------------------------------------

int transposeKey(int key, const Interval& interval)
      {
      int tpc = key+14;
      tpc = transposeTpc(tpc, interval, false);
      // check for valid key sigs
      if (tpc > 21) tpc-=12; // no more than 7 sharps in keysig
      if (tpc < 7) tpc+=12; // no more than 7 flats in keysig
      return (tpc-14);
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
                  int _accidentalType:4;
                  int _naturalType:4;
                  unsigned _customType:16;
                  bool _custom : 1;
                  bool _invalid : 1;
                  };
            };
      U a;
      a.subtype       = st;
      _accidentalType = a._accidentalType;
      _naturalType    = a._naturalType;
      _customType     = a._customType;
      _custom         = a._custom;
      _invalid        = a._invalid;
      enforceLimits();
      }

}

