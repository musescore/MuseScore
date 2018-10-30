//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __POINT_H__
#define __POINT_H__

#include "fraction.h"

#include <climits>

namespace Ms {

class Element;
class XmlReader;
class XmlWriter;

enum class Pid;

class Location {
      int _staff;
      int _voice;
      int _measure;
      Fraction _frac;
      int _graceIndex;
      int _note;
      bool _rel;

      static int track(const Element* e);
      static int measure(const Element* e);
      static int graceIndex(const Element* e);
      static int note(const Element* e);

   public:
      constexpr Location(int staff, int voice, int measure, Fraction frac, int graceIndex, int note, bool rel)
         : _staff(staff), _voice(voice), _measure(measure), _frac(frac), _graceIndex(graceIndex), _note(note), _rel(rel) {}

      static constexpr Location absolute() { return Location(INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, false); }
      static constexpr Location relative() { return Location(0, 0, 0, 0, INT_MIN, 0, true); }

      void toAbsolute(const Location& ref);
      void toRelative(const Location& ref);

      void write(XmlWriter& xml) const;
      void read(XmlReader& e);

      bool isAbsolute() const       { return !_rel;         }
      bool isRelative() const       { return _rel;          }

      int staff() const             { return _staff;        }
      void setStaff(int staff)      { _staff = staff;       }
      int voice() const             { return _voice;        }
      void setVoice(int voice)      { _voice = voice;       }
      int track() const;
      void setTrack(int track);
      int measure() const           { return _measure;      }
      void setMeasure(int measure)  { _measure = measure;   }
      Fraction frac() const         { return _frac;         }
      void setFrac(Fraction frac)   { _frac = frac;         }
      int graceIndex() const        { return _graceIndex;   }
      void setGraceIndex(int index) { _graceIndex = index;  }
      int note() const              { return _note;         }
      void setNote(int note)        { _note = note;         }

      void fillForElement(const Element* e, bool absfrac = true);
      void fillPositionForElement(const Element* e, bool absfrac = true);
      static Location forElement(const Element* e, bool absfrac = true);
      static Location positionForElement(const Element* e, bool absfrac = true);
      static QVariant getLocationProperty(Pid pid, const Element* start, const Element* end);

      bool operator==(const Location& other) const;
      bool operator!=(const Location& other) const { return !(*this == other); }
      };

}     // namespace Ms
#endif
