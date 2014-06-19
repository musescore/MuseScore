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

// first beat time can be larger than first chord onTime
// so insert additional beats at the beginning to cover all chords

void addFirstBeats(
            std::set<ReducedFraction> &beatSet,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            size_t beatsInBar)
      {
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

// last beat time can be smaller than the last chord onTime
// so insert additional beats at the end to cover all chords

void addLastBeats(
            std::set<ReducedFraction> &beatSet,
            size_t beatsInBar,
            const std::multimap<ReducedFraction, MidiChord> &chords)
      {
                  // theoretically it's possible that every chord have off time
                  // at the end of the piece - so check all chords for max off time
      ReducedFraction lastOffTime(0, 1);
      for (const auto &chord: chords) {
            for (const auto &note: chord.second.notes) {
                  if (note.offTime > lastOffTime)
                        lastOffTime = note.offTime;
                  }
            }

      auto lastBeat = *(std::prev(beatSet.end()));
      if (lastOffTime > lastBeat) {
            if (beatSet.size() > 1) {
                  const auto beatLen = lastBeat - *std::prev(beatSet.end(), 2);
                  size_t counter = 0;
                  do {
                        lastBeat += beatLen;
                        beatSet.insert(lastBeat);
                        ++counter;
                        } while (lastBeat < lastOffTime || counter % beatsInBar);
                  }
            }
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

      addFirstBeats(beatSet, chords, beatsInBar);
      addLastBeats(beatSet, beatsInBar, chords);

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

void removeEvery2ndBeat(std::set<ReducedFraction> &beatSet)
      {
      auto it = beatSet.begin();
      while (it != beatSet.end() && std::next(it) != beatSet.end())
            it = beatSet.erase(std::next(it));
      if (it == beatSet.end()) {
                        // insert additional beat at the end to cover all chords
            const auto beatLen = *std::prev(it) - *std::prev(it, 2);
            beatSet.insert(*std::prev(it) + beatLen);
            }
      }

// we can use ReducedFraction for time signature (bar fraction)
// because it reduces itself only if numerator or denominator
// is greater than some big number (see ReducedFraction class definition)

std::vector<ReducedFraction> findTimeSignatures(const ReducedFraction &timeSigFromMidiFile)
      {
      std::vector<ReducedFraction> fractions{ReducedFraction(4, 4), ReducedFraction(3, 4)};
      bool match = false;
      for (const ReducedFraction &f: fractions) {
            if (f.isIdenticalTo(timeSigFromMidiFile)) {
                  match = true;
                  break;
                  }
            }
      if (!match) {     // some special time sig in MIDI file - use only it
            fractions.clear();
            fractions.push_back(timeSigFromMidiFile);
            }
      return fractions;
      }

void findBeatLocations(
            const std::multimap<ReducedFraction, MidiChord> &allChords,
            TimeSigMap *sigmap,
            double ticksPerSec)
      {
      const size_t MIN_BEAT_COUNT = 8;
      const auto barFractions = findTimeSignatures(ReducedFraction(sigmap->timesig(0).timesig()));
      const std::vector<
                 std::function<double(const std::pair<const ReducedFraction, MidiChord> &, double)>
                       >
                 salienceFuncs = {findChordSalience1, findChordSalience2};

                  // <match rank, beat set + bar fraction, comparator>
      std::map<double,
               std::pair<std::set<ReducedFraction>, ReducedFraction>,
               std::greater<double>
              > beatResults;

      for (const auto &func: salienceFuncs) {
            const auto events = prepareChordEvents(allChords, func, ticksPerSec);
            const auto beatTimes = BeatTracker::beatTrack(events);
            if (beatTimes.size() <= MIN_BEAT_COUNT)
                  continue;

            for (const ReducedFraction &barFraction: barFractions) {
                  const auto beatLen = Meter::beatLength(barFraction);
                  const auto div = barFraction / beatLen;
                  const size_t beatsInBar = div.numerator() / div.denominator();

                  const std::vector<Meter::DivisionInfo> divsInfo
                                    = { Meter::metricDivisionsOfBar(barFraction) };
                  const auto levels = Meter::metricLevelsOfBar(barFraction, divsInfo, beatLen);

                  Q_ASSERT_X(levels.size() == beatsInBar,
                             "MidiBeat::findBeatLocations", "Wrong count of bar levels");

                              // beat set - first case
                  auto beatSet = prepareHumanBeatSet(beatTimes, allChords,
                                                     ticksPerSec, beatsInBar);
                  double matchRank = findMatchRank(beatSet, events,
                                                   levels, beatsInBar, ticksPerSec);
                  beatResults.insert({matchRank, {beatSet, barFraction}});

                              // second case - remove every 2nd beat
                  removeEvery2ndBeat(beatSet);
                  matchRank = findMatchRank(beatSet, events, levels, beatsInBar, ticksPerSec);
                  beatResults.insert({matchRank, {beatSet, barFraction}});
                  }
            }
      if (!beatResults.empty()) {
            const auto &beatSet = beatResults.begin()->second.first;
            const auto &barFraction = beatResults.begin()->second.second;
            sigmap->clear();
            sigmap->add(0, barFraction.fraction());
            preferences.midiImportOperations.setHumanBeats(beatSet);
            }
      }

void scaleOffTimes(
            QList<MidiNote> &notes,
            const std::set<ReducedFraction> *beats,
            const std::set<ReducedFraction>::const_iterator &onTimeBeatEndIt,
            const ReducedFraction &newOnTimeBeatStart,
            const ReducedFraction &newBeatLen,
            ReducedFraction &lastTick)
      {
      for (auto &note: notes) {
            int beatCount = 0;      // beat count between note on time and off time

            Q_ASSERT_X(onTimeBeatEndIt != beats->begin(),
                       "MidiBeat::scaleOffTimes",
                       "End beat iterator cannot be the first beat iterator");

            auto bStart = *std::prev(onTimeBeatEndIt);
            for (auto bit = onTimeBeatEndIt; bit != beats->end(); ++bit) {
                  const auto &bEnd = *bit;

                  Q_ASSERT_X(bEnd > bStart,
                             "MidiBeat::scaleOffTimes",
                             "Beat end <= beat start for note off time that is incorrect");

                  if (note.offTime >= bStart && note.offTime < bEnd) {
                        const auto scale = newBeatLen / (bEnd - bStart);
                        auto newOffTimeInBeat = (note.offTime - bStart) * scale;
                        newOffTimeInBeat = Quantize::quantizeValue(
                                                newOffTimeInBeat, MChord::minAllowedDuration());
                        const auto desiredBeatStart = newOnTimeBeatStart + newBeatLen * beatCount;
                        note.offTime = desiredBeatStart + newOffTimeInBeat;
                        if (note.offTime > lastTick)
                              lastTick = note.offTime;
                        break;
                        }

                  bStart = bEnd;
                  ++beatCount;
                  }
            }
      }

void adjustChordsToBeats(std::multimap<int, MTrack> &tracks,
                         ReducedFraction &lastTick)
      {
      const auto &opers = preferences.midiImportOperations;
      const auto *beats = opers.getHumanBeats();

      if (beats && !beats->empty() && opers.trackOperations(0).quantize.humanPerformance) {

            Q_ASSERT_X(beats->size() > 1, "MidiBeat::adjustChordsToBeats", "Human beat count < 2");

            const auto newBeatLen = ReducedFraction::fromTicks(MScore::division);
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
                  auto newBeatStart = ReducedFraction(0, 1);

                  for (++it; it != beats->end(); ++it) {
                        const auto &beatEnd = *it;

                        Q_ASSERT_X(beatEnd > beatStart, "MidiBeat::adjustChordsToBeats",
                                   "Beat end <= beat start that is incorrect");

                        const auto scale = newBeatLen / (beatEnd - beatStart);

                        for (; chordIt != chords.end() && chordIt->first < beatEnd; ++chordIt) {
                              auto newOnTimeInBeat = (chordIt->first - beatStart) * scale;
                                          // quantize to prevent ReducedFraction overflow
                              newOnTimeInBeat = Quantize::quantizeValue(
                                                 newOnTimeInBeat, MChord::minAllowedDuration());
                              scaleOffTimes(chordIt->second.notes, beats, it,
                                            newBeatStart, newBeatLen, lastTick);
                              const auto newOnTime = newBeatStart + newOnTimeInBeat;
                              newChords.insert({newOnTime, chordIt->second});
                              }

                        if (chordIt == chords.end())
                              break;

                        beatStart = beatEnd;
                        newBeatStart += newBeatLen;
                        }

                  std::swap(chords, newChords);
                  }
            }
      }

} // namespace MidiBeat
} // namespace Ms
