//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __DURATIONTYPE_H__
#define __DURATIONTYPE_H__

#include "fraction.h"
#include "note.h"

namespace Ms {

//---------------------------------------------------------
//   TDuration
//---------------------------------------------------------

class TDuration {
   public:
      enum class DurationType : char {
            V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHT, V_16TH,
            V_32ND, V_64TH, V_128TH, V_256TH, V_512TH, V_1024TH,
            V_ZERO, V_MEASURE,  V_INVALID
            };
   private:
      DurationType _val;
      int _dots;

   public:
      TDuration() : _val(DurationType::V_INVALID), _dots(0) {}
      TDuration(const Fraction&);
      TDuration(const QString&);
      TDuration(DurationType t) : _val(t), _dots(0) {}
      DurationType type() const             { return _val; }
      bool isValid() const                  { return _val != DurationType::V_INVALID; }
      bool isZero() const                   { return _val == DurationType::V_ZERO; }
      void setVal(int tick);
      void setType(DurationType t);
      void setType(const QString&);

      int ticks() const;
      bool operator==(const TDuration& t) const    { return t._val == _val && t._dots == _dots; }
      bool operator==(const DurationType& t) const { return t == _val; }
      bool operator!=(const TDuration& t) const    { return t._val != _val || t._dots != _dots; }
      bool operator<(const TDuration& t) const;
      bool operator>(const TDuration& t) const;
      bool operator>=(const TDuration& t) const;
      bool operator<=(const TDuration& t) const;
      TDuration& operator-=(const TDuration& t);
      TDuration operator-(const TDuration& t) const { return TDuration(*this) -= t; }
      TDuration& operator+=(const TDuration& t);
      TDuration operator+(const TDuration& t) const { return TDuration(*this) += t; }

      QString name() const;
      NoteHead::Type headType() const;
      int hooks() const;
      bool hasStem() const;
      TDuration shift(int val) const;
      int dots() const    { return _dots; }
      void setDots(int v);
      Fraction fraction() const;
      void print() const;
      };

extern QList<TDuration> toDurationList(
            Fraction, bool useDots, int maxDots = 2, bool printRestRemains = true);

}     // namespace Ms
#endif

