#ifndef IMPORTMIDI_QUANT_H
#define IMPORTMIDI_QUANT_H


namespace Ms {

class MidiChord;
class TimeSigMap;
class ReducedFraction;

namespace MidiTuplet {
struct TupletData;
}

namespace Quantize {

void quantizeChords(std::multimap<ReducedFraction, MidiChord> &chords,
                    const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents,
                    const TimeSigMap *sigmap);

ReducedFraction fixedQuantRaster();

ReducedFraction reduceRasterIfDottedNote(const ReducedFraction &noteLen,
                                         const ReducedFraction &raster);

ReducedFraction quantizeValue(const ReducedFraction &value,
                              const ReducedFraction &raster);

ReducedFraction findRegularQuantRaster(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &startBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &endChordIt,
            const ReducedFraction &endBarTick);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
