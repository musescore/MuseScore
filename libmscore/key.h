//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: key.h 5149 2011-12-29 08:38:43Z wschweer $
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

class Xml;
class Score;

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

typedef std::map<const int, KeySigEvent>::iterator iKeyList;
typedef std::map<const int, KeySigEvent>::const_iterator ciKeyList;

class KeyList : public std::map<const int, KeySigEvent> {
   public:
      KeyList() {}
      KeySigEvent key(int tick) const;
      void read(const QDomElement&, Score*);
      void write(Xml&, const char* name) const;
      };

struct Interval;
extern int transposeKey(int oldKey, const Interval&);

#endif

