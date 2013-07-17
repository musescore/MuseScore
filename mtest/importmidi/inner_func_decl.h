#ifndef INNER_FUNC_DECL_H
#define INNER_FUNC_DECL_H

#include "libmscore/fraction.h"


namespace Ms {

class MidiChord;
class Fraction;

namespace MidiTuplet {

std::pair<std::multimap<Fraction, MidiChord>::iterator, Fraction>
findBestChordForTupletNote(const Fraction &tupletNotePos,
                           const Fraction &quantValue,
                           const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                           const std::multimap<Fraction, MidiChord>::iterator &endChordIt);

bool isTupletAllowed(int tupletNumber,
                     const Fraction &tupletLen,
                     const Fraction &tupletOnTimeSumError,
                     const Fraction &regularSumError,
                     const Fraction &quantValue,
                     const std::map<int, std::multimap<Fraction, MidiChord>::iterator> &tupletChords);

std::vector<int> findTupletNumbers(const Fraction &divLen, const Fraction &barFraction);

Fraction findOnTimeRegularError(const Fraction &onTime, const Fraction &quantValue);

struct TupletInfo;

TupletInfo findTupletApproximation(const Fraction &tupletLen,
                                   int tupletNumber,
                                   const Fraction &quantValue,
                                   const Fraction &startTupletTime,
                                   const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                                   const std::multimap<Fraction, MidiChord>::iterator &endChordIt);

int separateTupletVoices(std::vector<TupletInfo> &tuplets,
                         const std::multimap<Fraction, MidiChord>::iterator &startBarChordIt,
                         const std::multimap<Fraction, MidiChord>::iterator &endBarChordIt,
                         std::multimap<Fraction, MidiChord> &chords,
                         const Fraction &endBarTick);

} // namespace MidiTuplet

namespace Meter {

struct MaxLevel;
struct DivisionInfo;

Meter::MaxLevel maxLevelBetween(const Fraction &startTickInBar,
                                const Fraction &endTickInBar,
                                const DivisionInfo &divInfo);

Meter::MaxLevel findMaxLevelBetween(const Fraction &startTickInBar,
                                    const Fraction &endTickInBar,
                                    const std::vector<DivisionInfo> &divsInfo);

} // namespace Meter

} // namespace Ms

#endif // INNER_FUNC_DECL_H
