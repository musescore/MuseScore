#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H

#include "importmidi_fraction.h"


namespace Ms {

class MidiChord;
class TimeSigMap;
class MTrack;
class DurationElement;

namespace MidiTuplet {

struct TupletInfo;

struct TupletData
      {
      int voice;
      ReducedFraction onTime;
      ReducedFraction len;
      int tupletNumber;
      std::vector<DurationElement *> elements;
      };

struct TupletLimits
      {
            // ratio - for conversion from tuplet durations to regular durations
            // for example, 8th note in triplet * 3/2 = regular 8th note
      ReducedFraction ratio;
      int minNoteCount;
      int minNoteCountAddVoice;
      int minNoteCountStaccato;
      };

const TupletLimits& tupletLimits(int tupletNumber);
int tupletVoiceLimit();
void removeEmptyTuplets(MTrack &track);

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo &tuplet,
               const ReducedFraction &basicQuant);

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo> &tuplets,
                    const ReducedFraction &basicQuant);

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

std::multimap<ReducedFraction, MidiChord>::iterator
findTupletFreeChord(
            const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt,
            const ReducedFraction &startDivTick);

void findAllTuplets(
            std::multimap<ReducedFraction, TupletData> &tupletEvents,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const ReducedFraction &lastTick,
            const ReducedFraction &basicQuant);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
