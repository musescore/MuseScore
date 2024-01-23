//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

// everything contained in .h file for performance reasons

#ifndef __FRACTION_H__
#define __FRACTION_H__

#include "config.h"
#include "mscore.h"

namespace Ms {

//---------------------------------------------------------
//   gcd
//    greatest common divisor. always returns a positive val
//    however, since int / uint = uint by C++ rules,
//    return int to avoid accidental implicit unsigned cast
//---------------------------------------------------------

static int_least64_t gcd(int_least64_t a, int_least64_t b)
      {
      int bp;
      if (b > a) { bp = b; b = a; a = bp; } // Saves one % if true
      while (b != 0) {
            bp = b; b = a % b; a = bp;
            }

      return (a >= 0 ? a : -a);
      }


//---------------------------------------------------------
//   Fraction
//---------------------------------------------------------

class Fraction {

      // ensure 64 bit to avoid overflows in comparisons
      int_least64_t _numerator   { 0 };
      int_least64_t _denominator { 1 };

   public:

#if 0
      // implicit conversion from int to Fraction: this is convenient but may hide some potential bugs
      constexpr Fraction(int z=0, int n=1) : _numerator(z), _denominator(n) {}
#else
      // no implicit conversion from int to Fraction:
      constexpr Fraction()  {}
      constexpr Fraction(int z, int n) : _numerator { n < 0 ? -z : z }, _denominator { n < 0 ? -n : n } { }
#endif
      int numerator() const      { return _numerator;           }
      int denominator() const    { return _denominator;         }
      int_least64_t& rnumerator()          { return _numerator;           }
      int_least64_t& rdenominator()        { return _denominator;         }

      void setNumerator(int v)   { _numerator = v;              }
      void setDenominator(int v) {
            if (v < 0) { _numerator = -_numerator; _denominator = -v; }
            else _denominator = v;
            }
      void set(int z, int n)     {
            if (n < 0)  { _numerator = -z; _denominator = -n; }
            else { _numerator = z; _denominator = n; }
            }

      bool isZero() const        { return _numerator == 0;      }
      bool isNotZero() const     { return _numerator != 0;      }
      bool negative() const      { return _numerator < 0;       }

      bool isValid() const       { return _denominator != 0;    }

      // check if two fractions are identical (numerator & denominator)
      // == operator checks for equal value:
      bool identical(const Fraction& v) const {
            return (_numerator == v._numerator) &&
                   (_denominator == v._denominator);
            }

      Fraction absValue() const  {
            return Fraction(qAbs(_numerator), _denominator); }

      Fraction inverse() const  {
            return Fraction(_denominator, _numerator); }


      // --- reduction --- //

      void reduce()
            {
            const int g = gcd(_numerator, _denominator);
            if (g) {
                  _numerator /= g;
                  _denominator /= g;
                  }
            }

      Fraction reduced() const
            {
            const int g = gcd(_numerator, _denominator);
            if (g)
                  return Fraction(_numerator / g, _denominator / g);
            return Fraction(_numerator, _denominator);
            }      

      // --- comparison --- //

      bool operator<(const Fraction& val) const
            {
            return _numerator * val._denominator < val._numerator * _denominator;
            }

      bool operator<=(const Fraction& val) const
            {
            return _numerator * val._denominator <= val._numerator * _denominator;
            }

      bool operator>=(const Fraction& val) const
            {
            return _numerator * val._denominator >= val._numerator * _denominator;
            }

      bool operator>(const Fraction& val) const
            {
            return _numerator * val._denominator > val._numerator * _denominator;
            }

      bool operator==(const Fraction& val) const
            {
            return _numerator * val._denominator == val._numerator * _denominator;
            }

      bool operator!=(const Fraction& val) const
            {
            return _numerator * val._denominator != val._numerator * _denominator;
            }

      // --- arithmetic --- //

      Fraction& operator+=(const Fraction& val)
            {
            if (_denominator == val._denominator)
                  _numerator += val._numerator;  // Common enough use case to be handled separately for efficiency
            else {
                  const int g = gcd(_denominator, val._denominator);
                  if (g) {
                        const int m1 = val._denominator / g; // This saves one division over straight lcm
                        _numerator = _numerator * m1 + val._numerator * (_denominator / g);
                        _denominator = m1 * _denominator;
                        }
                  }
            return *this;
            }

      Fraction& operator-=(const Fraction& val)
            {
            if (_denominator == val._denominator)
                  _numerator -= val._numerator; // Common enough use case to be handled separately for efficiency
            else {
                  const int g = gcd(_denominator, val._denominator);
                  if (g) {
                        const int m1 = val._denominator / g; // This saves one division over straight lcm
                        _numerator = _numerator * m1 - val._numerator * (_denominator / g);
                        _denominator = m1 * _denominator;
                        }
                  }
            return *this;
            }

      Fraction& operator*=(const Fraction& val)
            {
            _numerator *= val._numerator;
            _denominator  *= val._denominator;
            if (val._denominator != 1) reduce(); // We should be free to fully reduce here
            return *this;
            }

      Fraction& operator*=(int val)
            {
            _numerator *= val;
            return *this;
            }

      Fraction& operator/=(const Fraction& val)
            {
            const int sign = (val._numerator >= 0 ? 1 : -1);
            _numerator   *= (sign*val._denominator);
            _denominator *= (sign*val._numerator);
            if (val._numerator != sign) reduce();
            return *this;
            }


      Fraction& operator/=(int val)
            {
            _denominator *= val;
            if (_denominator < 0) {
                  _denominator = -_denominator;
                  _numerator = -_numerator;
                  }
            reduce();
            return *this;
            }



      Fraction operator+(const Fraction& v) const { return Fraction(*this) += v; }
      Fraction operator-(const Fraction& v) const { return Fraction(*this) -= v; }
      Fraction operator-() const                  { return Fraction(-_numerator, _denominator); }
      Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
      Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }
      Fraction operator/(int v)             const { return Fraction(*this) /= v; }

      double toDouble() { return (double)_numerator / _denominator; }

      //---------------------------------------------------------
      //   fromTicks
      //---------------------------------------------------------

      static Fraction fromTicks(int ticks)
            {
            if (ticks == -1)
                  return Fraction(-1,1);  // HACK
            return Fraction(ticks, MScore::division * 4).reduced();
            }

      //---------------------------------------------------------
      //   eps
      ///   A very small fraction, corresponds to 1 MIDI tick
      //---------------------------------------------------------

      static Fraction eps() { return Fraction(1, MScore::division * 4); }

      //---------------------------------------------------------
      //   ticks
      //---------------------------------------------------------

      int ticks() const
            {
            if ((_numerator == -1 && _denominator == 1) || _denominator == 0)        // HACK
                  return -1;

            // MScore::division     - ticks per quarter note
            // MScore::division * 4 - ticks per whole note
            // result: rounded (MScore::division * 4 * _numerator * 1.0 / _denominator) value
            const int sgn = (_numerator < 0) ? -1 : 1;
            const auto result = sgn * (static_cast<int_least64_t>(sgn * _numerator) * MScore::division * 4 + (_denominator/2)) / _denominator;
            return static_cast<int>(result);
            }



      QString print() const     { return QString("%1/%2").arg(_numerator).arg(_denominator); }
      QString toString() const  { return print(); }
      static Fraction fromString(const QString& str) {
            const int i = str.indexOf('/');
            return (i == -1) ? Fraction(str.toInt(), 1) : Fraction(str.leftRef(i).toInt(), str.midRef(i+1).toInt());
            }
      operator QVariant() const { return QVariant::fromValue(*this); }
      };

 inline Fraction operator*(const Fraction& f, int v) { return Fraction(f) *= v; }
 inline Fraction operator*(int v, const Fraction& f) { return Fraction(f) *= v; }

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Fraction);

#endif
