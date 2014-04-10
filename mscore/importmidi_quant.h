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

ReducedFraction quantForLen(
            const ReducedFraction &basicQuant,
            const ReducedFraction &noteLen,
            const ReducedFraction &tupletRatio);

ReducedFraction findQuantForRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio);

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart);

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart);

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant);

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart);

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart);

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart);

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart);

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant);

ReducedFraction findBarStart(
            const ReducedFraction &time,
            const TimeSigMap *sigmap);

void quantizeChords(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents,
            const TimeSigMap *sigmap,
            const ReducedFraction &basicQuant);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
