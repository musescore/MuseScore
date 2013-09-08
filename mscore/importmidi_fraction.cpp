#include "importmidi_fraction.h"
#include "libmscore/mscore.h"


namespace Ms {

// this helps to decrease the risk of overflow of int multiplication

void reduceIfBigFraction(ReducedFraction &f)
      {
      const int SIZE_LIMIT = 100;
      if (f.numerator() > SIZE_LIMIT || f.denominator() > SIZE_LIMIT)
            f.reduce();
      }

//---------------------------------------------------


ReducedFraction::ReducedFraction()
      : f(0, 1)
      {
      }

ReducedFraction::ReducedFraction(int z, int n)
      : f(z, n)
      {
      }

ReducedFraction::ReducedFraction(const Fraction &fraction)
      : f(fraction)
      {
      }

ReducedFraction ReducedFraction::fromTicks(int v)
      {
      return ReducedFraction(Fraction::fromTicks(v));
      }

ReducedFraction ReducedFraction::reduced() const
      {
      return ReducedFraction(f.reduced());
      }

ReducedFraction ReducedFraction::absValue() const
      {
      return ReducedFraction(f.absValue());
      }

int ReducedFraction::ticks() const
      {
      return f.ticks();
      }

void ReducedFraction::reduce()
      {
      f.reduce();
      }

ReducedFraction& ReducedFraction::operator+=(const ReducedFraction& val)
      {
      reduceIfBigFraction(*this);
      ReducedFraction value = val;
      reduceIfBigFraction(value);
      f += value.fraction();
      return *this;
      }

ReducedFraction& ReducedFraction::operator-=(const ReducedFraction& val)
      {
      reduceIfBigFraction(*this);
      ReducedFraction value = val;
      reduceIfBigFraction(value);
      f -= value.fraction();
      return *this;
      }

ReducedFraction& ReducedFraction::operator*=(const ReducedFraction& val)
      {
      reduceIfBigFraction(*this);
      ReducedFraction value = val;
      reduceIfBigFraction(value);
      f *= value.fraction();
      return *this;
      }

ReducedFraction& ReducedFraction::operator*=(int val)
      {
      reduceIfBigFraction(*this);
      f *= val;
      return *this;
      }

ReducedFraction& ReducedFraction::operator/=(const ReducedFraction& val)
      {
      reduceIfBigFraction(*this);
      ReducedFraction value = val;
      reduceIfBigFraction(value);
      f /= value.fraction();
      return *this;
      }

ReducedFraction& ReducedFraction::operator/=(int val)
      {
      reduceIfBigFraction(*this);
      f /= val;
      return *this;
      }

bool ReducedFraction::operator<(const ReducedFraction& val) const
      {
      return f < val.fraction();
      }

bool ReducedFraction::operator<=(const ReducedFraction& val) const
      {
      return f <= val.fraction();
      }

bool ReducedFraction::operator>=(const ReducedFraction& val) const
      {
      return f >= val.fraction();
      }

bool ReducedFraction::operator>(const ReducedFraction& val) const
      {
      return f > val.fraction();
      }

bool ReducedFraction::operator==(const ReducedFraction& val) const
      {
      return f == val.fraction();
      }

bool ReducedFraction::operator!=(const ReducedFraction& val) const
      {
      return f != val.fraction();
      }

//-------------------------------------------------------------------------

ReducedFraction toMuseScoreTicks(int tick, int oldDivision)
      {
      return ReducedFraction::fromTicks((tick * MScore::division + oldDivision / 2) / oldDivision);
      }

} // namespace Ms
