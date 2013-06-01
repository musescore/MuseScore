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


// list of levels (0, -1, ...) of all ticks in bar that are multiples of minDuration
// if minDuration <= 0 then minDuration will be set to min allowed note length (1/128)
std::vector<int> metricLevelsOfBar(const Fraction &barFraction, int minDuration);

QList<TDuration> toDurationList(int startTickInBar, int endTickInBar,
                                const Fraction &barFraction, DurationType durationType,
                                bool useDots);

} // namespace Meter
} // namespace Ms


#endif // IMPORTMIDI_METER_H
