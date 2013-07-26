#ifndef IMPORTMIDI_QUANT_H
#define IMPORTMIDI_QUANT_H


namespace Ms {

class MidiChord;
class TimeSigMap;
class Fraction;

namespace MidiTuplet {
struct TupletData;
}

namespace Quantize {

void applyAdaptiveQuant(std::multimap<Fraction, MidiChord> &,
                        const TimeSigMap *,
                        const Fraction &);

void applyGridQuant(std::multimap<Fraction, MidiChord> &chords,
                    const TimeSigMap *sigmap,
                    const Fraction &lastTick);

void quantizeChordsAndTuplets(std::multimap<Fraction, MidiTuplet::TupletData> &tupletEvents,
                              std::multimap<Fraction, MidiChord> &inputChords,
                              const TimeSigMap *sigmap,
                              const Fraction &lastTick);

Fraction fixedQuantRaster();

Fraction reduceRasterIfDottedNote(const Fraction &len, const Fraction &raster);
Fraction quantizeValue(const Fraction &value, const Fraction &raster);

Fraction findRegularQuantRaster(const std::multimap<Fraction, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<Fraction, MidiChord>::iterator &endChordIt,
                    const Fraction &endBarTick);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
