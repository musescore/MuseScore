#ifndef IMPORTMIDI_QUANT_H
#define IMPORTMIDI_QUANT_H

#include "importmidi_operation.h"


namespace Ms {

class MidiChord;
class MTrack;
class TimeSigMap;
class ReducedFraction;

namespace Quantize {

ReducedFraction userQuantNoteToFraction(MidiOperation::QuantValue quantNote);

ReducedFraction findQuantForRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end,
            const ReducedFraction &basicQuant);

ReducedFraction findQuantizedTupletChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart);

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findQuantizedTupletNoteOffTime(
            const ReducedFraction &offTime,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart);

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant);

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findMaxQuantizedTupletOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart);

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findOnTimeTupletQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart);

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant);

ReducedFraction findOffTimeTupletQuantError(
            const ReducedFraction &offTime,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart);

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant);

void setIfHumanPerformance(
            const std::multimap<int, MTrack> &tracks,
            const TimeSigMap *sigmap);

void quantizeChords(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const ReducedFraction &basicQuant);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
