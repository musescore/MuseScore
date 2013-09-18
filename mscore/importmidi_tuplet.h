#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H

#include "importmidi_fraction.h"


namespace Ms {

class MidiChord;
class DurationElement;
class TimeSigMap;
class MTrack;
class Staff;

namespace MidiTuplet {

struct TupletInfo;

struct TupletData
      {
      int voice;
      ReducedFraction onTime;
      ReducedFraction len;
      int tupletNumber;
      ReducedFraction tupletQuant;
      std::vector<DurationElement *> elements;
      };

// conversion ratios from tuplet durations to regular durations
// for example, 8th note in triplet * 3/2 = regular 8th note

const std::map<int, ReducedFraction> &tupletRatios();

std::vector<TupletData>
findTupletsInBarForDuration(int voice,
                            const ReducedFraction &barStartTick,
                            const ReducedFraction &durationOnTime,
                            const ReducedFraction &durationLen,
                            const std::multimap<ReducedFraction, TupletData> &tupletEvents);

std::multimap<ReducedFraction, MidiTuplet::TupletData>::const_iterator
findTupletForTimeRange(int voice,
                       const ReducedFraction &onTime,
                       const ReducedFraction &len,
                       const std::multimap<ReducedFraction, TupletData> &tupletEvents);

std::multimap<ReducedFraction, MidiTuplet::TupletData>::const_iterator
findTupletContainsTime(int voice,
                       const ReducedFraction &time,
                       const std::multimap<ReducedFraction, TupletData> &tupletEvents);

std::multimap<ReducedFraction, TupletData>
findAllTuplets(std::multimap<ReducedFraction, MidiChord> &chords,
               const TimeSigMap *sigmap,
               const ReducedFraction &lastTick);

void removeEmptyTuplets(MTrack &track);

void addElementToTuplet(int voice,
                        const ReducedFraction &onTime,
                        const ReducedFraction &len,
                        DurationElement *el,
                        std::multimap<ReducedFraction, TupletData> &tuplets);

void createTuplets(Staff *staff,
                   const std::multimap<ReducedFraction, TupletData> &tuplets);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
