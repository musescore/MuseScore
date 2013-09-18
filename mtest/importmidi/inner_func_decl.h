#ifndef INNER_FUNC_DECL_H
#define INNER_FUNC_DECL_H


namespace Ms {

class MidiChord;
class ReducedFraction;

namespace MidiTuplet {

struct TupletInfo;

std::pair<std::multimap<ReducedFraction, MidiChord>::iterator, ReducedFraction>
findBestChordForTupletNote(const ReducedFraction &tupletNotePos,
                           const ReducedFraction &quantValue,
                           const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                           const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt);

bool isTupletAllowed(const TupletInfo &tupletInfo);

std::vector<int> findTupletNumbers(const ReducedFraction &divLen, const ReducedFraction &barFraction);

ReducedFraction findQuantizationError(const ReducedFraction &onTime, const ReducedFraction &quantValue);

TupletInfo findTupletApproximation(const ReducedFraction &tupletLen,
                                   int tupletNumber,
                                   const ReducedFraction &quantValue,
                                   const ReducedFraction &startTupletTime,
                                   const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                                   const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt);

void splitFirstTupletChords(std::vector<TupletInfo> &tuplets,
                            std::multimap<ReducedFraction, MidiChord> &chords);

std::list<int> findTupletsWithCommonChords(std::list<int> &restTuplets,
                                           const std::vector<TupletInfo> &tuplets);

std::vector<int> findTupletsWithNoCommonChords(std::list<int> &commonTuplets,
                                               const std::vector<TupletInfo> &tuplets);

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
