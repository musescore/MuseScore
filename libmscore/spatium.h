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

#ifndef __SPATIUM_H__
#define __SPATIUM_H__

namespace Ms {

//---------------------------------------------------------
//   Spatium
//    - a unit of measure
//    - the distance between two note lines
//    - used for many layout items
//---------------------------------------------------------

class Spatium {
      qreal _val;

   public:
      constexpr Spatium() : _val(0.0) {}
      explicit Spatium(qreal v)       { _val = v; }

      constexpr qreal val() const     { return _val; }

      bool operator>(const Spatium& a) const  { return _val > a._val; }
      bool operator<(const Spatium& a) const  { return _val < a._val; }
      bool operator==(const Spatium& a) const { return qFuzzyCompare(_val, a._val); }
      bool operator!=(const Spatium& a) const { return !qFuzzyCompare(_val, a._val); }
      bool isZero() const                     { return qFuzzyIsNull(_val); }

      Spatium& operator+=(const Spatium& a) {
            _val += a._val;
            return *this;
            }
      Spatium& operator-=(const Spatium& a) {
            _val -= a._val;
            return *this;
            }
      Spatium& operator/=(qreal d) {
            _val /= d;
            return *this;
            }
      qreal operator/(const Spatium& b) {
            return _val / b._val;
            }
      Spatium& operator*=(int d) {
            _val *= d;
            return *this;
            }
      Spatium& operator*=(qreal d) {
            _val *= d;
            return *this;
            }
      Spatium operator-() const { return Spatium(-_val); }
      operator QVariant() const { return QVariant::fromValue(*this); }

      static double toDouble(const Spatium& v) { return v._val; }
      };

inline Spatium operator+(const Spatium& a, const Spatium& b)
      {
      Spatium r(a);
      r += b;
      return r;
      }
inline Spatium operator-(const Spatium& a, const Spatium& b)
      {
      Spatium r(a);
      r -= b;
      return r;
      }
inline Spatium operator/(const Spatium& a, qreal b)
      {
      Spatium r(a);
      r /= b;
      return r;
      }
inline Spatium operator*(const Spatium& a, int b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(int a, const Spatium& b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }
inline Spatium operator*(const Spatium& a, qreal b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(qreal a, const Spatium& b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Spatium);

#endif

