#include "importmidi_beat.h"

#include "importmidi_chord.h"
#include "importmidi_fraction.h"
#include "importmidi_inner.h"
#include "importmidi_quant.h"
#include "importmidi_meter.h"
#include "thirdparty/beatroot/BeatTracker.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/sig.h"

#include <set>
#include <functional>


namespace Ms {
namespace MidiBeat {

double findChordSalience1(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            double ticksPerSec)
      {
      ReducedFraction duration(0, 1);
      int pitch = std::numeric_limits<int>::max();
      int velocity = 0;

      for (const MidiNote &note: chord.second.notes) {
            if (note.offTime - chord.first > duration)
                  duration = note.offTime - chord.first;
            if (note.pitch < pitch)
                  pitch = note.pitch;
            velocity += note.velo;
            }
      const double durationInSeconds = duration.ticks() / ticksPerSec;

      const double c4 = 84;
      const int pmin = 48;
      const int pmax = 72;

      if (pitch < pmin)
            pitch = pmin;
      else if (pitch > pmax)
            pitch = pmax;

      if (velocity <= 0)
            velocity = 1;

      return durationInSeconds * (c4 - pitch) * std::log(velocity);
      }

double findChordSalience2(
            const std::pair<const ReducedFraction, MidiChord> &chord, double)
      {
      int velocity = 0;
      for (const MidiNote &note: chord.second.notes) {
            velocity += note.velo;
            }
      if (velocity <= 0)
            velocity = 1;

      return velocity;
      }

::EventList prepareChordEvents(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::function<double(const std::pair<const ReducedFraction, MidiChord> &,
                                       double)> &findChordSalience,
            double ticksPerSec)
      {
      ::EventList events;
      double minSalience = std::numeric_limits<double>::max();
      for (const auto &chord: chords) {
            ::Event e;
            e.time = chord.first.ticks() / ticksPerSec;
            e.salience = findChordSalience(chord, ticksPerSec);
            if (e.salience < minSalience)
                  minSalience = e.salience;
            events.push_back(e);
            }
                  // all saliences should be non-negative
      if (minSalience < 0) {
            for (auto &e: events) {
                  e.salience -= minSalience;
                  }
            }

      return events;
      }

std::set<ReducedFraction>
prepareHumanBeatSet(const std::vector<double> &beatTimes,
                    const std::multimap<ReducedFraction, MidiChord> &chords,
                    double ticksPerSec,
                    size_t beatsInBar)
      {
      std::set<ReducedFraction> beatSet;
      if (chords.empty())
            return beatSet;

      for (const auto &beatTime: beatTimes)
            beatSet.insert(MidiTempo::time2Tick(beatTime, ticksPerSec));
      {
                  // first beat time can be larger than first chord onTime
                  // so insert additional beats at the beginning to cover all chords
      const auto &firstOnTime = chords.begin()->first;
      auto firstBeat = *beatSet.begin();
      if (firstOnTime < firstBeat) {
            if (beatSet.size() > 1) {
                  const auto beatLen = *std::next(beatSet.begin()) - firstBeat;
                  size_t counter = 0;
                  do {
                        firstBeat -= beatLen;
                        beatSet.insert(firstBeat);
                        ++counter;
                        } while (firstBeat > firstOnTime || counter % beatsInBar);
                  }
            }
      }
      {
                  // last beat time can be smaller than thelast chord onTime
                  // so insert additional beats at the beginning to cover all chords
      const auto &lastOnTime = std::prev(chords.end())->first;
      auto lastBeat = *(std::prev(beatSet.end()));
      if (lastOnTime > lastBeat) {
            if (beatSet.size() > 1) {
                  const auto beatLen = lastBeat - *std::prev(beatSet.end(), 2);
                  size_t counter = 0;
                  do {
                        lastBeat += beatLen;
                        beatSet.insert(lastBeat);
                        ++counter;
                        } while (lastBeat < lastOnTime || counter % beatsInBar);
                  }
            }
      }

      return beatSet;
      }

double findMatchRank(const std::set<ReducedFraction> &beatSet,
                     const ::EventList &events,
                     const std::vector<int> &levels,
                     size_t beatsInBar,
                     double ticksPerSec)
{
      std::map<ReducedFraction, double> saliences;
      for (const auto &e: events) {
            saliences.insert({MidiTempo::time2Tick(e.time, ticksPerSec), e.salience});
            }
      std::vector<ReducedFraction> beatsOfBar;
      double matchFrac = 0;
      size_t matchCount = 0;
      size_t beatCount = 0;

      for (const auto &beat: beatSet) {
            beatsOfBar.push_back(beat);
            ++beatCount;
            if (beatCount == beatsInBar) {
                  beatCount = 0;
                  size_t relationCount = 0;
                  size_t relationMatches = 0;
                  for (size_t i = 0; i != beatsOfBar.size() - 1; ++i) {
                        const auto s1 = saliences.find(beatsOfBar[i]);
                        for (size_t j = i + 1; j != beatsOfBar.size(); ++j) {
                              ++relationCount;    // before s1 search check
                              if (s1 == saliences.end())
                                    continue;
                              const auto s2 = saliences.find(beatsOfBar[j]);
                              if (s2 == saliences.end())
                                    continue;
                              if ((s1->second < s2->second) == (levels[i] < levels[j]))
                                    ++relationMatches;
                              }
                        }
                  if (relationCount) {
                        matchFrac += relationMatches * 1.0 / relationCount;
                        ++matchCount;
                        }
                  beatsOfBar.clear();
                  }
            }
      if (matchCount)
            matchFrac /= matchCount;

      return matchFrac;
}

void findBeatLocations(
            const std::multimap<ReducedFraction, MidiChord> &allChords,
            const TimeSigMap *sigmap,
            double ticksPerSec)
      {
      const size_t MIN_BEAT_COUNT = 8;
      const auto barFraction = ReducedFraction(sigmap->timesig(0).timesig());
      const auto beatLen = Meter::beatLength(barFraction);
      const auto div = barFraction / beatLen;
      const size_t beatsInBar = div.numerator() / div.denominator();
      std::vector<Meter::DivisionInfo> divsInfo = { Meter::metricDivisionsOfBar(barFraction) };
      const std::vector<int> levels = Meter::metricLevelsOfBar(barFraction, divsInfo, beatLen);

      Q_ASSERT_X(levels.size() == beatsInBar,
                 "MidiBeat::findBeatLocations", "Wrong count of bar levels");

      const std::vector<std::function<double(const std::pair<const ReducedFraction, MidiChord> &,
                                             double)>> salienceFuncs
                  = {findChordSalience1, findChordSalience2};
      std::map<double, std::set<ReducedFraction>, std::greater<double>> beatResults;

      for (const auto &func: salienceFuncs) {
            const auto events = prepareChordEvents(allChords, func, ticksPerSec);
            const auto beatTimes = BeatTracker::beatTrack(events);

            if (beatTimes.size() > MIN_BEAT_COUNT) {
                  const auto beatSet = prepareHumanBeatSet(beatTimes, allChords,
                                                           ticksPerSec, beatsInBar);
                  const double matchRank = findMatchRank(beatSet, events,
                                                         levels, beatsInBar, ticksPerSec);
                  beatResults.insert({matchRank, std::move(beatSet)});
                  }
            }
      if (!beatResults.empty()) {
            preferences.midiImportOperations.setHumanBeats(beatResults.begin()->second);
            }
      }

void adjustChordsToBeats(std::multimap<int, MTrack> &tracks,
                         ReducedFraction &lastTick)
      {
      const auto &opers = preferences.midiImportOperations;
      const auto &beats = opers.getHumanBeats();
      if (beats && !beats->empty() && opers.trackOperations(0).quantize.humanPerformance) {
            for (auto trackIt = tracks.begin(); trackIt != tracks.end(); ++trackIt) {
                  auto &chords = trackIt->second.chords;
                  if (chords.empty())
                        continue;
                             // do chord alignment according to recognized beats
                  std::multimap<ReducedFraction, MidiChord> newChords;
                  lastTick = {0, 1};

                  auto chordIt = chords.begin();
                  auto it = beats->begin();
                  auto beatStart = *it;
                  auto correctedBeatStart = ReducedFraction(0, 1);

                  for (; it != beats->end(); ++it) {
                        const auto &beatEnd = *it;
                        if (beatEnd == beatStart)
                              continue;
                        const auto desiredBeatLen = ReducedFraction::fromTicks(MScore::division);
                        const auto scale = desiredBeatLen / (beatEnd - beatStart);

                        for (; chordIt != chords.end() && chordIt->first < beatEnd; ++chordIt) {
                                          // quantize to prevent ReducedFraction overflow
                              const auto onTimeInBeat = Quantize::quantizeValue(
                                                      (chordIt->first - beatStart) * scale,
                                                      MChord::minAllowedDuration());
                              MidiChord &chord = chordIt->second;
                              const auto newChordOnTime = correctedBeatStart + onTimeInBeat;
                              for (auto &note: chord.notes) {
                                    note.offTime = newChordOnTime + Quantize::quantizeValue(
                                                      (note.offTime - chordIt->first) * scale,
                                                      MChord::minAllowedDuration());
                                    if (note.offTime > lastTick)
                                          lastTick = note.offTime;
                                    }
                              newChords.insert({newChordOnTime, chord});
                              }
                        if (chordIt == chords.end())
                              break;

                        beatStart = beatEnd;
                        correctedBeatStart += desiredBeatLen;
                        }

                  std::swap(chords, newChords);
                  }
            }
      }

} // namespace MidiBeat
} // namespace Ms
