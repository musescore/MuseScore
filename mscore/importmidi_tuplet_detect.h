#ifndef IMPORTMIDI_TUPLET_DETECT_H
#define IMPORTMIDI_TUPLET_DETECT_H


namespace Ms {

class ReducedFraction;
class MidiChord;

namespace MidiTuplet {

struct TupletInfo;

std::vector<TupletInfo> detectTuplets(
            const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
            const ReducedFraction &startBarTick,
            const ReducedFraction &barFraction,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant,
            int barIndex);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_DETECT_H
