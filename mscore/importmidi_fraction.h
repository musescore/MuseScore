#ifndef IMPORTMIDI_FRACTION_H
#define IMPORTMIDI_FRACTION_H

#include "libmscore/fraction.h"


namespace Ms {

class ReducedFraction
      {
   public:
      ReducedFraction(int z = 0, int n = 1);
      ReducedFraction(const Fraction &);

      Fraction fraction() const { return f; }
      int numerator() const { return f.numerator(); }
      int denominator() const { return f.denominator(); }

      static ReducedFraction fromTicks(int v);
      ReducedFraction reduced() const;
      ReducedFraction absValue() const;
      int ticks() const;
      void reduce();

      ReducedFraction& operator+=(const ReducedFraction&);
      ReducedFraction& operator-=(const ReducedFraction&);
      ReducedFraction& operator*=(const ReducedFraction&);
      ReducedFraction& operator*=(int);
      ReducedFraction& operator/=(const ReducedFraction&);
      ReducedFraction& operator/=(int);

      ReducedFraction operator+(const ReducedFraction& v) const { return ReducedFraction(*this) += v; }
      ReducedFraction operator-(const ReducedFraction& v) const { return ReducedFraction(*this) -= v; }
      ReducedFraction operator*(const ReducedFraction& v) const { return ReducedFraction(*this) *= v; }
      ReducedFraction operator*(int v)                    const { return ReducedFraction(*this) *= v; }
      ReducedFraction operator/(const ReducedFraction& v) const { return ReducedFraction(*this) /= v; }
      ReducedFraction operator/(int v)                    const { return ReducedFraction(*this) /= v; }

      bool operator<(const ReducedFraction&) const;
      bool operator<=(const ReducedFraction&) const;
      bool operator>=(const ReducedFraction&) const;
      bool operator>(const ReducedFraction&) const;
      bool operator==(const ReducedFraction&) const;
      bool operator!=(const ReducedFraction&) const;

   private:
      Fraction f;
      };

} // namespace Ms


#endif // IMPORTMIDI_FRACTION_H
