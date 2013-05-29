#ifndef IMPORTMIDI_METER_H
#define IMPORTMIDI_METER_H


namespace Ms {

class Fraction;
class TDuration;

namespace Meter {

enum class DurationType
      {
      NOTE,
      REST
      };

bool isSimple(const Fraction &barFraction);
bool isCompound(const Fraction &barFraction);
bool isComplex(const Fraction &barFraction);
bool isDuple(const Fraction &barFraction);
bool isTriple(const Fraction &barFraction);
bool isQuadruple(const Fraction &barFraction);

QList<TDuration> toDurationList(int startTickInBar, int endTickInBar,
                                const Fraction &barFraction, DurationType durationType,
                                bool useDots);

} // namespace Meter
} // namespace Ms


#endif // IMPORTMIDI_METER_H
