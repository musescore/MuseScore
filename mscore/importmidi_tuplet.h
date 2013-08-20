#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H

#include "importmidi_fraction.h"


namespace Ms {

class MidiChord;
class DurationElement;

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

ReducedFraction findOffTimeRaster(const ReducedFraction &noteOffTime,
                                  int voice,
                                  const ReducedFraction &regularQuant,
                                  const std::vector<TupletInfo> &tuplets);

ReducedFraction findOffTimeRaster(const ReducedFraction &noteOffTime,
                                  int voice,
                                  const ReducedFraction &regularQuant,
                                  const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents);

std::vector<TupletData> findTuplets(const ReducedFraction &startBarTick,
                                    const ReducedFraction &endBarTick,
                                    const ReducedFraction &barFraction,
                                    std::multimap<ReducedFraction, MidiChord> &chords);
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

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
