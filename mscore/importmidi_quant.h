#ifndef IMPORTMIDI_QUANT_H
#define IMPORTMIDI_QUANT_H


namespace Ms {

class MidiChord;
class TimeSigMap;

namespace Quantize {

void applyAdaptiveQuant(std::multimap<int, MidiChord> &, const TimeSigMap *, int);

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    const TimeSigMap* sigmap,
                    int lastTick);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
