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
      int _numerator;
      int _denominator;

   public:
      constexpr Fraction(int z = 0, int n = 1) : _numerator(z), _denominator(n) {}
      int numerator() const      { return _numerator;           }
      int denominator() const    { return _denominator;         }
      int& rnumerator()          { return _numerator;           }
      int& rdenominator()        { return _denominator;         }

      void setNumerator(int v)   { _numerator = v;              }
      void setDenominator(int v) { _denominator = v;               }
      void set(int z, int n)     { _numerator = z; _denominator = n; }

      bool isZero() const        { return _numerator == 0;      }

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
      Fraction& operator/=(int);

      Fraction operator+(const Fraction& v) const { return Fraction(*this) += v; }
      Fraction operator-(const Fraction& v) const { return Fraction(*this) -= v; }
      Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
      Fraction operator*(int v)             const { return Fraction(*this) *= v; }
      Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }
      Fraction operator/(int v)             const { return Fraction(*this) /= v; }

      bool operator<(const Fraction&) const;
      bool operator<=(const Fraction&) const;
      bool operator>=(const Fraction&) const;
      bool operator>(const Fraction&) const;
      bool operator==(const Fraction&) const;
      bool operator!=(const Fraction&) const;

      QString print() const { return QString("%1/%2").arg(_numerator).arg(_denominator); }
      QString toString() const { return print(); }
      operator QVariant() const { return QVariant::fromValue(*this); }
      };

#ifdef SCRIPT_INTERFACE

//---------------------------------------------------------
//   FractionWrapper
//---------------------------------------------------------

class FractionWrapper : public QObject {
      Q_OBJECT
      Q_PROPERTY(int numerator READ numerator)
      Q_PROPERTY(int denominator READ denominator)
      Q_PROPERTY(int ticks READ ticks)

      Fraction f;

   public slots:
      void setFraction(Fraction _f) { f = _f; }

   public:
      FractionWrapper(const FractionWrapper& w) : QObject() { f = w.f; }
      FractionWrapper() {}
      FractionWrapper(const Fraction& _f) : f(_f) {}

      Fraction fraction() const { return f; }
      int numerator() const   { return f.numerator(); }
      int denominator() const { return f.denominator(); }
      int ticks() const       { return f.ticks(); }
      };


#endif // SCRIPT_INTERFACE

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Fraction);

#ifdef SCRIPT_INTERFACE
Q_DECLARE_METATYPE(Ms::FractionWrapper);
#endif

#endif

