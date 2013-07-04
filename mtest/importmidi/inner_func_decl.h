#ifndef INNER_FUNC_DECL_H
#define INNER_FUNC_DECL_H


namespace Ms {

class MidiChord;
class Fraction;

namespace MidiTuplet {

std::pair<std::multimap<int, MidiChord>::iterator, int>
findBestChordForTupletNote(int tupletNotePos,
                           int quantValue,
                           const std::multimap<int, MidiChord>::iterator &startChordIt,
                           const std::multimap<int, MidiChord>::iterator &endChordIt);

bool isTupletAllowed(int tupletNumber,
                     int tupletLen,
                     int tupletOnTimeSumError,
                     int regularSumError,
                     int quantValue,
                     const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords);

std::vector<int> findTupletNumbers(int divLen, const Fraction &barFraction);

int findOnTimeRegularError(int onTime, int quantValue);

struct TupletInfo;

TupletInfo findTupletApproximation(int tupletNumber,
                                   int tupletNoteLen,
                                   int quantValue,
                                   int startTupletTime,
                                   const std::multimap<int, MidiChord>::iterator &startChordIt,
                                   const std::multimap<int, MidiChord>::iterator &endChordIt);

} // namespace Quantize
} // namespace Ms

#endif // INNER_FUNC_DECL_H
