#ifndef IMPORTMIDI_QUANT_H
#define IMPORTMIDI_QUANT_H


namespace Ms {

class MidiChord;
class TimeSigMap;

namespace MidiTuplet {
struct TupletData;
}

namespace Quantize {

void applyAdaptiveQuant(std::multimap<int, MidiChord> &, const TimeSigMap *, int);

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    const TimeSigMap* sigmap,
                    int lastTick);

void quantizeChordsAndTuplets(std::multimap<int, MidiTuplet::TupletData> &tupletEvents,
                              std::multimap<int, MidiChord> &chords,
                              const TimeSigMap* sigmap,
                              int lastTick);

int findQuantRaster(const std::multimap<int, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<int, MidiChord>::iterator &endChordIt,
                    int endBarTick);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
