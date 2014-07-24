#include "importmidi_fraction.h"
#include "libmscore/mscore.h"

#include <limits>


namespace Ms {


#ifdef QT_DEBUG

//---------------------------------------------------------------------------------------
// https://www.securecoding.cert.org/confluence/display/seccode/
// INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow?showComments=false
//
// Access date: 2013.11.28

bool isAdditionOverflow(int a, int b)           // a + b
      {
      return ((b > 0 && a > (std::numeric_limits<int>::max() - b))
                || (b < 0 && a < std::numeric_limits<int>::min() - b));
      }

bool isSubtractionOverflow(int a, int b)        // a - b
      {
      return ((b > 0 && a < std::numeric_limits<int>::min() + b)
                  || (b < 0 && a > std::numeric_limits<int>::max() + b));
      }

bool isMultiplicationOverflow(int a, int b)     // a * b
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

      return flag;
      }

bool isDivisionOverflow(int a, int b)           // a / b
      {
      return ((b == 0) || ((a == std::numeric_limits<int>::min()) && (b == -1)));
      }

bool isRemainderOverflow(int a, int b)          // a % b
      {
      return ((b == 0) || ((a == std::numeric_limits<int>::min()) && (b == -1)));
      }

bool isUnaryNegationOverflow(int a)             // -a
      {
      return (a == std::numeric_limits<int>::min());
      }

#endif

//---------------------------------------------------------------------------------------


namespace {

// greatest common divisor

int gcd(int a, int b)
      {

      Q_ASSERT_X(!isUnaryNegationOverflow(a),
                 "ReducedFraction, gcd", "Unary negation overflow");

      if (b == 0)
            return a < 0 ? -a : a;

      Q_ASSERT_X(!isRemainderOverflow(a, b),
                 "ReducedFraction, gcd", "Remainder overflow");

      return gcd(b, a % b);
      }

// least common multiple

unsigned lcm(int a, int b)
      {
      const int tmp = gcd(a, b);

      Q_ASSERT_X(!isDivisionOverflow(a, tmp),
                 "ReducedFraction, lcm", "Division overflow");
      Q_ASSERT_X(!isMultiplicationOverflow(a / tmp, b),
                 "ReducedFraction, lcm", "Multiplication overflow");

      return a / tmp * b;
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

      Q_ASSERT_X(!isDivisionOverflow(numerator_, tmp),
                 "ReducedFraction::reduced", "Division overflow");
      Q_ASSERT_X(!isDivisionOverflow(denominator_, tmp),
                 "ReducedFraction::reduced", "Division overflow");

      return ReducedFraction(numerator_ / tmp, denominator_ / tmp);
      }

int ReducedFraction::ticks() const
      {
      int integral = numerator_ / denominator_;
      int newNumerator = numerator_ % denominator_;
      int division = MScore::division * 4;

      Q_ASSERT_X(!isMultiplicationOverflow(newNumerator, division),
                 "ReducedFraction::ticks", "Multiplication overflow");
      Q_ASSERT_X(!isAdditionOverflow(newNumerator * division, denominator_ / 2),
                 "ReducedFraction::ticks", "Addition overflow");

      const int tmp = newNumerator * division + denominator_ / 2;

      Q_ASSERT_X(!isDivisionOverflow(tmp, denominator_),
                 "ReducedFraction::ticks", "Division overflow");
      Q_ASSERT_X(!isMultiplicationOverflow(integral, denominator_),
                 "ReducedFraction::ticks", "Multiplication overflow");
      Q_ASSERT_X(!isAdditionOverflow(tmp / denominator_, integral * division),
                 "ReducedFraction::ticks", "Addition overflow");

      return tmp / denominator_ + integral * division;
      }

void ReducedFraction::reduce()
      {
      if (numerator_ == 0) {
            denominator_ = 1;
            return;
            }
      const int tmp = gcd(numerator_, denominator_);

      Q_ASSERT_X(!isDivisionOverflow(numerator_, tmp),
                 "ReducedFraction::reduce", "Division overflow");
      Q_ASSERT_X(!isDivisionOverflow(denominator_, tmp),
                 "ReducedFraction::reduce", "Division overflow");

      numerator_ /= tmp;
      denominator_ /= tmp;
}

void ReducedFraction::preventOverflow()
      {
      static const int reduceLimit = 10000;
      if (numerator_ >= reduceLimit || denominator_ >= reduceLimit)
            reduce();
      }

// helper function

int fractionPart(int lcmPart, int numerator, int denominator)
      {

      Q_ASSERT_X(!isDivisionOverflow(lcmPart, denominator),
                 "ReducedFraction::fractionPart", "Division overflow");

      const int part = lcmPart / denominator;

      Q_ASSERT_X(!isMultiplicationOverflow(numerator, part),
                 "ReducedFraction::fractionPart", "Multiplication overflow");

      return numerator * part;
      }

ReducedFraction& ReducedFraction::operator+=(const ReducedFraction& val)
      {
      preventOverflow();
      ReducedFraction value = val;
      value.preventOverflow();

      const int tmp = lcm(denominator_, val.denominator_);
      numerator_ = fractionPart(tmp, numerator_, denominator_)
                  + fractionPart(tmp, val.numerator_, val.denominator_);
      denominator_ = tmp;
      return *this;
      }

ReducedFraction& ReducedFraction::operator-=(const ReducedFraction& val)
      {
      preventOverflow();
      ReducedFraction value = val;
      value.preventOverflow();

      const int tmp = lcm(denominator_, val.denominator_);
      numerator_ = fractionPart(tmp, numerator_, denominator_)
                  - fractionPart(tmp, val.numerator_, val.denominator_);
      denominator_ = tmp;
      return *this;
      }

ReducedFraction& ReducedFraction::operator*=(const ReducedFraction& val)
      {
      preventOverflow();
      ReducedFraction value = val;
      value.preventOverflow();

      Q_ASSERT_X(!isMultiplicationOverflow(numerator_, val.numerator_),
                 "ReducedFraction::operator*=", "Multiplication overflow");
      Q_ASSERT_X(!isMultiplicationOverflow(denominator_, val.denominator_),
                 "ReducedFraction::operator*=", "Multiplication overflow");

      numerator_ *= val.numerator_;
      denominator_ *= val.denominator_;
      return *this;
      }

ReducedFraction& ReducedFraction::operator*=(int val)
      {
      preventOverflow();

      Q_ASSERT_X(!isMultiplicationOverflow(numerator_, val),
                 "ReducedFraction::operator*=", "Multiplication overflow");

      numerator_ *= val;
      return *this;
      }

ReducedFraction& ReducedFraction::operator/=(const ReducedFraction& val)
      {
      preventOverflow();
      ReducedFraction value = val;
      value.preventOverflow();

      Q_ASSERT_X(!isMultiplicationOverflow(numerator_, val.denominator_),
                 "ReducedFraction::operator/=", "Multiplication overflow");
      Q_ASSERT_X(!isMultiplicationOverflow(denominator_, val.numerator_),
                 "ReducedFraction::operator/=", "Multiplication overflow");

      numerator_ *= val.denominator_;
      denominator_  *= val.numerator_;
      return *this;
      }

ReducedFraction& ReducedFraction::operator/=(int val)
      {
      preventOverflow();

      Q_ASSERT_X(!isMultiplicationOverflow(denominator_, val),
                 "ReducedFraction::operator/=", "Multiplication overflow");

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

      Q_ASSERT_X(!isMultiplicationOverflow(tick, MScore::division),
                 "ReducedFraction::toMuseScoreTicks", "Multiplication overflow");
      Q_ASSERT_X(!isAdditionOverflow(tick * MScore::division, oldDivision / 2),
                 "ReducedFraction::toMuseScoreTicks", "Addition overflow");

      const int tmp = tick * MScore::division + oldDivision / 2;

      Q_ASSERT_X(!isDivisionOverflow(tmp, oldDivision),
                 "ReducedFraction::toMuseScoreTicks", "Division overflow");

      return ReducedFraction::fromTicks(tmp / oldDivision);
      }

} // namespace Ms
