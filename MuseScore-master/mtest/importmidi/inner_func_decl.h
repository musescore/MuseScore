#ifndef INNER_FUNC_DECL_H
#define INNER_FUNC_DECL_H

#include <set>


namespace Ms {

class MidiChord;
class ReducedFraction;

namespace MidiTuplet {

struct TupletInfo;

bool isTupletAllowed(const TupletInfo &tupletInfo);

std::vector<int> findTupletNumbers(const ReducedFraction &divLen, const ReducedFraction &barFraction);

TupletInfo findTupletApproximation(const ReducedFraction &tupletLen,
                                   int tupletNumber,
                                   const ReducedFraction &quantValue,
                                   const ReducedFraction &startTupletTime,
                                   const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                                   const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt);

void splitFirstTupletChords(std::vector<TupletInfo> &tuplets,
                            std::multimap<ReducedFraction, MidiChord> &chords);

std::set<int> findLongestUncommonGroup(const std::vector<TupletInfo> &tuplets,
                                       const ReducedFraction &basicQuant);

} // namespace MidiTuplet

namespace Meter {

struct MaxLevel;
struct DivisionInfo;

Meter::MaxLevel maxLevelBetween(const ReducedFraction &startTickInBar,
                                const ReducedFraction &endTickInBar,
                                const DivisionInfo &divInfo);

Meter::MaxLevel findMaxLevelBetween(const ReducedFraction &startTickInBar,
                                    const ReducedFraction &endTickInBar,
                                    const std::vector<DivisionInfo> &divsInfo);

} // namespace Meter

} // namespace Ms

#endif // INNER_FUNC_DECL_H
