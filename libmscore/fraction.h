//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FRACTION_H__
#define __FRACTION_H__

#include "config.h"

namespace Ms {

//---------------------------------------------------------
//   Fraction
//---------------------------------------------------------

class Fraction {
      int _numerator   { 0 };
      int _denominator { 1 };

   public:

#if 0
      // implicit conversion from int to Fraction: this is convenient but may hide some potential bugs
      constexpr Fraction(int z=0, int n=1) : _numerator(z), _denominator(n) {}
#else
      // no implicit conversion from int to Fraction:
      constexpr Fraction()  {}
      constexpr Fraction(int z, int n) : _numerator(z), _denominator(n) {}
#endif
      int numerator() const      { return _numerator;           }
      int denominator() const    { return _denominator;         }
      int& rnumerator()          { return _numerator;           }
      int& rdenominator()        { return _denominator;         }

      void setNumerator(int v)   { _numerator = v;              }
      void setDenominator(int v) { _denominator = v;               }
      void set(int z, int n)     { _numerator = z; _denominator = n; }

      bool isZero() const        { return _numerator == 0;      }
      bool isNotZero() const     { return _numerator != 0;      }

      bool isValid() const       { return _denominator != 0;    }
      void reduce();
      Fraction reduced() const;
      Fraction absValue() const;

      // check if two fractions are identical (numerator & denominator)
      // == operator checks for equal value:
      bool identical(const Fraction& v) const {
            return (_numerator == v._numerator) && (_denominator == v._denominator);
            }

      int ticks() const;

      static Fraction fromTicks(int v);

      Fraction& operator+=(const Fraction&);
      Fraction& operator-=(const Fraction&);
      Fraction& operator*=(const Fraction&);
      Fraction& operator*=(int);
      Fraction& operator/=(const Fraction&);
//      Fraction& operator/=(int);

      Fraction operator+(const Fraction& v) const { return Fraction(*this) += v; }
      Fraction operator-(const Fraction& v) const { return Fraction(*this) -= v; }
      Fraction operator-() const                  { return Fraction(-_numerator, _denominator); }
      Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
      Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }
//      Fraction operator/(int v)             const { return Fraction(*this) /= v; }

      bool operator<(const Fraction&) const;
      bool operator<=(const Fraction&) const;
      bool operator>=(const Fraction&) const;
      bool operator>(const Fraction&) const;
      bool operator==(const Fraction&) const;
      bool operator!=(const Fraction&) const;

      QString print() const     { return QString("%1/%2").arg(_numerator).arg(_denominator); }
      QString toString() const  { return print(); }
      operator QVariant() const { return QVariant::fromValue(*this); }
      };

 inline Fraction operator*(const Fraction& f, int v) { return Fraction(f) *= v; }
 inline Fraction operator*(int v, const Fraction& f) { return Fraction(f) *= v; }

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Fraction);

#endif
