#include "importmidi_fraction.h"
#include "libmscore/mscore.h"

#include <limits>


namespace Ms {

//-----------------------------------------------------------------------------
// https://www.securecoding.cert.org/confluence/display/seccode/
// INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow?showComments=false
//
// Access date: 2013.11.28

inline void checkAdditionOverflow(int a, int b)           // a + b
      {
      if ((b > 0 && a > (std::numeric_limits<int>::max() - b))
                  || (b < 0 && a < std::numeric_limits<int>::min() - b))
            {
            qDebug("ReducedFraction: addition overflow");
            abort();
            }
      }

inline void checkSubtractionOverflow(int a, int b)        // a - b
      {
      if ((b > 0 && a < std::numeric_limits<int>::min() + b)
                  || (b < 0 && a > std::numeric_limits<int>::max() + b))
            {
            qDebug("ReducedFraction: subtraction overflow");
            abort();
            }
      }

inline void checkMultiplicationOverflow(int a, int b)     // a * b
      {
      bool flag = false;

      if (a > 0) {
            if (b > 0) {
                  if (a > std::numeric_limits<int>::max() / b)
                        flag = true;
                  }
            else {
                  if (b < std::numeric_limits<int>::min() / a)
                        flag = true;
                  }
            }
      else {
            if (b > 0) {
                  if (a < std::numeric_limits<int>::min() / b)
                        flag = true;
                  }
            else {
                  if (a != 0 && b < std::numeric_limits<int>::max() / a)
                        flag = true;
                  }
            }

      if (flag) {
            qDebug("ReducedFraction: multiplication overflow");
            abort();
            }
      }

inline void checkDivisionOverflow(int a, int b)           // a / b
      {
      if ((b == 0) || ((a == std::numeric_limits<int>::min()) && (b == -1))) {
            qDebug("ReducedFraction: division overflow");
            abort();
            }
      }

inline void checkRemainderOverflow(int a, int b)          // a % b
      {
      if ((b == 0) || ((a == std::numeric_limits<int>::min()) && (b == -1))) {
            qDebug("ReducedFraction: remainder overflow");
            abort();
            }
      }

inline void checkUnaryNegationOverflow(int a)             // -a
      {
      if (a == std::numeric_limits<int>::min()) {
            qDebug("ReducedFraction: unary nagation overflow");
            abort();
            }
      }

//-----------------------------------------------------------------------------

namespace {

// greatest common divisor

int gcd(int a, int b)
      {
      checkUnaryNegationOverflow(a);
      if (b == 0)
            return a < 0 ? -a : a;
      checkRemainderOverflow(a, b);
      return gcd(b, a % b);
      }

// least common multiple

unsigned lcm(int a, int b)
      {
      const int tmp = gcd(a, b);
      checkMultiplicationOverflow(a, b);
      checkDivisionOverflow(a * b, tmp);
      return a * b / tmp;
      }

} // namespace

//-----------------------------------------------------------------------------


ReducedFraction::ReducedFraction()
      : numerator_(0)
      , denominator_(1)
      {
      }

ReducedFraction::ReducedFraction(int z, int n)
      : numerator_(z)
      , denominator_(n)
      {
      }

ReducedFraction::ReducedFraction(const Fraction &fraction)
      : numerator_(fraction.numerator())
      , denominator_(fraction.denominator())
      {
      }

ReducedFraction ReducedFraction::fromTicks(int ticks)
      {
      return ReducedFraction(ticks, MScore::division * 4).reduced();
      }

ReducedFraction ReducedFraction::reduced() const
      {
      const int tmp = gcd(numerator_, denominator_);
      checkDivisionOverflow(numerator_, tmp);
      checkDivisionOverflow(denominator_, tmp);
      return ReducedFraction(numerator_ / tmp, denominator_ / tmp);
      }

ReducedFraction ReducedFraction::absValue() const
      {
      return ReducedFraction(qAbs(numerator_), qAbs(denominator_));
      }

int ReducedFraction::ticks() const
      {
      int integral = numerator_ / denominator_;
      int newNumerator = numerator_ % denominator_;
      int division = MScore::division * 4;

      checkMultiplicationOverflow(newNumerator, division);
      checkAdditionOverflow(newNumerator * division, denominator_ / 2);
      const int tmp = newNumerator * division + denominator_ / 2;

      checkDivisionOverflow(tmp, denominator_);
      checkMultiplicationOverflow(integral, denominator_);
      checkAdditionOverflow(tmp / denominator_, integral * division);
      return tmp / denominator_ + integral * division;
      }

void ReducedFraction::reduce()
      {
      const int tmp = gcd(numerator_, denominator_);
      checkDivisionOverflow(numerator_, tmp);
      checkDivisionOverflow(denominator_, tmp);
      numerator_ /= tmp;
      denominator_ /= tmp;
      }

// helper function

int fractionPart(int lcmPart, int numerator, int denominator)
      {
      checkDivisionOverflow(lcmPart, denominator);
      const int part = lcmPart / denominator;
      checkMultiplicationOverflow(numerator, part);
      return numerator * part;
      }

ReducedFraction& ReducedFraction::operator+=(const ReducedFraction& val)
      {
      reduce();
      ReducedFraction value = val;
      value.reduce();

      const int tmp = lcm(denominator_, val.denominator_);
      numerator_ = fractionPart(tmp, numerator_, denominator_)
                  + fractionPart(tmp, val.numerator_, val.denominator_);
      denominator_ = tmp;
      return *this;
      }

ReducedFraction& ReducedFraction::operator-=(const ReducedFraction& val)
      {
      reduce();
      ReducedFraction value = val;
      value.reduce();

      const int tmp = lcm(denominator_, val.denominator_);
      numerator_ = fractionPart(tmp, numerator_, denominator_)
                  - fractionPart(tmp, val.numerator_, val.denominator_);
      denominator_ = tmp;
      return *this;
      }

ReducedFraction& ReducedFraction::operator*=(const ReducedFraction& val)
      {
      reduce();
      ReducedFraction value = val;
      value.reduce();

      checkMultiplicationOverflow(numerator_, val.numerator_);
      checkMultiplicationOverflow(denominator_, val.denominator_);
      numerator_ *= val.numerator_;
      denominator_ *= val.denominator_;
      return *this;
      }

ReducedFraction& ReducedFraction::operator*=(int val)
      {
      reduce();
      checkMultiplicationOverflow(numerator_, val);
      numerator_ *= val;
      return *this;
      }

ReducedFraction& ReducedFraction::operator/=(const ReducedFraction& val)
      {
      reduce();
      ReducedFraction value = val;
      value.reduce();

      checkMultiplicationOverflow(numerator_, val.denominator_);
      checkMultiplicationOverflow(denominator_, val.numerator_);
      numerator_ *= val.denominator_;
      denominator_  *= val.numerator_;
      return *this;
      }

ReducedFraction& ReducedFraction::operator/=(int val)
      {
      reduce();
      checkMultiplicationOverflow(denominator_, val);
      denominator_ *= val;
      return *this;
      }

bool ReducedFraction::operator<(const ReducedFraction& val) const
      {
      const int v = lcm(denominator_, val.denominator_);
      return fractionPart(v, numerator_, denominator_)
                  < fractionPart(v, val.numerator_, val.denominator_);
      }

bool ReducedFraction::operator<=(const ReducedFraction& val) const
      {
      const int v = lcm(denominator_, val.denominator_);
      return fractionPart(v, numerator_, denominator_)
                  <= fractionPart(v, val.numerator_, val.denominator_);
      }

bool ReducedFraction::operator>(const ReducedFraction& val) const
      {
      const int v = lcm(denominator_, val.denominator_);
      return fractionPart(v, numerator_, denominator_)
                  > fractionPart(v, val.numerator_, val.denominator_);
      }

bool ReducedFraction::operator>=(const ReducedFraction& val) const
      {
      const int v = lcm(denominator_, val.denominator_);
      return fractionPart(v, numerator_, denominator_)
                  >= fractionPart(v, val.numerator_, val.denominator_);
      }

bool ReducedFraction::operator==(const ReducedFraction& val) const
      {
      const int v = lcm(denominator_, val.denominator_);
      return fractionPart(v, numerator_, denominator_)
                  == fractionPart(v, val.numerator_, val.denominator_);
      }

bool ReducedFraction::operator!=(const ReducedFraction& val) const
      {
      const int v = lcm(denominator_, val.denominator_);
      return fractionPart(v, numerator_, denominator_)
                  != fractionPart(v, val.numerator_, val.denominator_);
      }


//-------------------------------------------------------------------------

ReducedFraction toMuseScoreTicks(int tick, int oldDivision)
      {
      checkMultiplicationOverflow(tick, MScore::division);
      checkAdditionOverflow(tick * MScore::division, oldDivision / 2);
      const int tmp = tick * MScore::division + oldDivision / 2;
      checkDivisionOverflow(tmp, oldDivision);

      return ReducedFraction::fromTicks(tmp / oldDivision);
      }

} // namespace Ms
