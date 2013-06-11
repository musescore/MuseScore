#ifndef IMPORTMIDI_METER_H
#define IMPORTMIDI_METER_H


namespace Ms {

class Fraction;
class TDuration;
struct TupletData;

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

int beatLength(const Fraction &barFraction);

            // division lengths of bar, each can be a tuplet length
std::vector<int> divisionsOfBarForTuplets(const Fraction &barFraction);

            // duration and all tuplets should belong to the same voice
            // nested tuplets are not allowed
QList<TDuration> toDurationList(int startTickInBar,
                                int endTickInBar,
                                const Fraction &barFraction,
                                const std::vector<TupletData> &tupletsInBar,
                                DurationType durationType,
                                bool useDots);

} // namespace Meter
} // namespace Ms


#endif // IMPORTMIDI_METER_H
