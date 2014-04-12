#ifndef IMPORTMIDI_TUPLET_DETECT_H
#define IMPORTMIDI_TUPLET_DETECT_H


namespace Ms {

class ReducedFraction;
class MidiChord;

namespace MidiTuplet {

struct TupletInfo;

std::vector<TupletInfo> detectTuplets(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &barFraction,
            const ReducedFraction &startBarTick,
            const ReducedFraction &tol,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
            const ReducedFraction &basicQuant);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_DETECT_H
