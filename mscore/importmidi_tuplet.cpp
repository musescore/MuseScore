#include "importmidi_tuplet.h"
#include "importmidi_tuplet_detect.h"
#include "importmidi_tuplet_filter.h"
#include "importmidi_tuplet_voice.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "preferences.h"

#include <set>


namespace Ms {
namespace MidiTuplet {

const std::map<int, TupletLimits>& tupletsLimits()
      {
      const static std::map<int, TupletLimits> values = {
            {2, {{2, 3}, 1, 2, 2, 2}},
            {3, {{3, 2}, 1, 3, 3, 2}},
            {4, {{4, 3}, 3, 4, 3, 3}},
            {5, {{5, 4}, 3, 4, 4, 4}},
            {7, {{7, 8}, 4, 6, 5, 5}},
            {9, {{9, 8}, 6, 7, 7, 7}}
            };
      return values;
      }

const TupletLimits& tupletLimits(int tupletNumber)
      {
      auto it = tupletsLimits().find(tupletNumber);

      Q_ASSERT_X(it != tupletsLimits().end(), "MidiTuplet::tupletValue", "Unknown tuplet");

      return it->second;
      }

const TupletInfo& tupletFromId(int id, const std::vector<TupletInfo> &tuplets)
      {
      auto it = std::find_if(tuplets.begin(), tuplets.end(),
                             [=](const TupletInfo &t) { return t.id == id; });

      Q_ASSERT_X(it != tuplets.end(), "MidiTuplet::tupletFromId", "Tuplet not found from id");

      return *it;
      }

TupletInfo& tupletFromId(int id, std::vector<TupletInfo> &tuplets)
      {
      return const_cast<TupletInfo &>(
                        tupletFromId(id, const_cast<const std::vector<TupletInfo> &>(tuplets)));
      }

bool hasNonTrivialChord(
            const ReducedFraction &chordOnTime,
            const QList<MidiNote> &notes,
            const ReducedFraction &tupletOnTime,
            const ReducedFraction &tupletLen)
      {
      if (chordOnTime == tupletOnTime) {
            for (const auto &note: notes) {
                  if (note.offTime - chordOnTime < tupletLen)
                        return true;
                  }
            }
      else {
            if (chordOnTime > tupletOnTime && chordOnTime < tupletOnTime + tupletLen)
                  return true;
            if (chordOnTime >= tupletOnTime + tupletLen)
                  return false;

            Q_ASSERT_X(chordOnTime < tupletOnTime, "MidiTuplet::hasNonTrivialChord",
                       "Chord on time was not compared correctly");

            for (const auto &note: notes) {
                  if (note.offTime < tupletOnTime + tupletLen)
                        return true;
                  }
            }
      return false;
      }

bool isTupletUseless(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &len,
            const ReducedFraction &maxChordLength,
            const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      bool haveIntersectionWithChord = false;
      const auto foundChords = MChord::findChordsForTimeRange(voice, onTime, onTime + len,
                                                              chords, maxChordLength);
      for (const auto &chordIt: foundChords) {
                        // ok, tuplet contains at least one chord
                        // check now does it have notes with len < tuplet.len
            if (hasNonTrivialChord(chordIt->first, chordIt->second.notes, onTime, len)) {
                  haveIntersectionWithChord = true;
                  break;
                  }
            }

      return !haveIntersectionWithChord;
      }

std::multimap<ReducedFraction, TupletData>::iterator
removeTupletIfEmpty(
            const std::multimap<ReducedFraction, TupletData>::iterator &tupletIt,
            std::multimap<ReducedFraction, TupletData> &tuplets,
            const ReducedFraction &maxChordLength,
            std::multimap<ReducedFraction, MidiChord> &chords)
      {
      const auto &tuplet = tupletIt->second;
      if (isTupletUseless(tuplet.voice, tuplet.onTime, tuplet.len, maxChordLength, chords)) {
            for (auto &chord: chords) {   // remove references to this tuplet in chords and notes
                  if (chord.first >= tuplet.onTime + tuplet.len)
                        break;
                  MidiChord &c = chord.second;
                  if (c.isInTuplet && c.tuplet == tupletIt)
                        c.isInTuplet = false;
                  for (auto &note: c.notes) {
                        if (note.isInTuplet && note.tuplet == tupletIt)
                              note.isInTuplet = false;
                        }
                  }
            return tuplets.erase(tupletIt);
            }

      return tupletIt;
      }

// tuplets with no chords are removed
// tuplets with single chord with chord.onTime = tuplet.onTime
//    and chord.len = tuplet.len are removed as well

void removeEmptyTuplets(MTrack &track)
      {
      auto &tuplets = track.tuplets;
      if (tuplets.empty())
            return;
      auto &chords = track.chords;
      std::map<int, ReducedFraction> maxChordLengths = MChord::findMaxChordLengths(chords);

      for (auto tupletIt = tuplets.begin(); tupletIt != tuplets.end(); ) {
            const auto fit = maxChordLengths.find(tupletIt->second.voice);

            Q_ASSERT_X(fit != maxChordLengths.end(),
                       "MidiTuplet::removeEmptyTuplets",
                       "Max chord length for voice was not set");

            auto it = removeTupletIfEmpty(tupletIt, tuplets, fit->second, chords);
            if (it != tupletIt) {
                  tupletIt = it;
                  continue;
                  }
            ++tupletIt;
            }
      }

int averagePitch(const std::map<ReducedFraction,
                 std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      if (chords.empty())
            return -1;

      int sumPitch = 0;
      int noteCounter = 0;
      for (const auto &chord: chords) {
            const auto &midiNotes = chord.second->second.notes;
            for (const auto &midiNote: midiNotes) {
                  sumPitch += midiNote.pitch;
                  ++noteCounter;
                  }
            }

      return qRound(sumPitch * 1.0 / noteCounter);
      }

void sortNotesByPitch(const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                      const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt)
      {
      struct {
            bool operator()(const MidiNote &n1, const MidiNote &n2)
                  {
                  return (n1.pitch > n2.pitch);
                  }
            } pitchComparator;

      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            auto &midiNotes = it->second.notes;
            std::sort(midiNotes.begin(), midiNotes.end(), pitchComparator);
            }
      }

void sortTupletsByAveragePitch(std::vector<TupletInfo> &tuplets)
      {
      struct {
            bool operator()(const TupletInfo &t1, const TupletInfo &t2)
                  {
                  return (averagePitch(t1.chords) > averagePitch(t2.chords));
                  }
            } averagePitchComparator;
      std::sort(tuplets.begin(), tuplets.end(), averagePitchComparator);
      }

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo &tuplet,
               const ReducedFraction &basicQuant)
      {
      ReducedFraction tupletEnd = tuplet.onTime + tuplet.len;

      for (const auto &chord: tuplet.chords) {
            const auto offTime = Quantize::findMaxQuantizedOffTime(*chord.second, basicQuant);
            if (offTime > tupletEnd)
                  tupletEnd = offTime;
            }

      Q_ASSERT_X(tupletEnd > tuplet.onTime, "MidiTuplet::tupletInterval", "off time <= on time");

      return std::make_pair(tuplet.onTime, tupletEnd);
      }

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo> &tuplets,
                    const ReducedFraction &basicQuant)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> tupletIntervals;
      for (const auto &tuplet: tuplets)
            tupletIntervals.push_back(tupletInterval(tuplet, basicQuant));

      return tupletIntervals;
      }

// find tuplets over which duration lies

std::vector<TupletData>
findTupletsInBarForDuration(
            int voice,
            const ReducedFraction &barStartTick,
            const ReducedFraction &durationOnTime,
            const ReducedFraction &durationLen,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      std::vector<TupletData> tupletsData;
      if (tupletEvents.empty())
            return tupletsData;
      auto tupletIt = tupletEvents.lower_bound(barStartTick);

      while (tupletIt != tupletEvents.end()
                && tupletIt->first < durationOnTime + durationLen) {
            if (tupletIt->second.voice == voice
                        && durationOnTime < tupletIt->first + tupletIt->second.len) {
                              // if tuplet and duration intersect each other
                  auto tupletData = tupletIt->second;
                              // convert tuplet onTime to local bar ticks
                  tupletData.onTime -= barStartTick;
                  tupletsData.push_back(tupletData);
                  }
            ++tupletIt;
            }
      return tupletsData;
      }

std::vector<std::multimap<ReducedFraction, TupletData>::const_iterator>
findTupletsForTimeRange(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &len,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      Q_ASSERT_X(len >= ReducedFraction(0, 1),
                 "MidiTuplet::findTupletForTimeRange", "Negative length of the time range");

      std::vector<std::multimap<ReducedFraction, TupletData>::const_iterator> result;

      if (tupletEvents.empty())
            return result;

      auto it = tupletEvents.upper_bound(onTime + len);
      if (it == tupletEvents.begin())
            return result;
      --it;
      while (true) {
            const auto &tupletData = it->second;
            if (tupletData.voice == voice) {
                  const auto interval = std::make_pair(onTime, onTime + len);
                  const auto tupletInterval = std::make_pair(
                                    tupletData.onTime, tupletData.onTime + tupletData.len);
                  if (haveIntersection(interval, tupletInterval)) {
                        result.push_back(it);
                        }
                  }
            if (it == tupletEvents.begin())
                  break;
            --it;
            }
      return result;
      }

std::multimap<ReducedFraction, TupletData>::const_iterator
findTupletContainsTime(
            int voice,
            const ReducedFraction &time,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      const auto tuplets = findTupletsForTimeRange(voice, time, ReducedFraction(0, 1), tupletEvents);
      if (tuplets.empty())
            return tupletEvents.end();

      Q_ASSERT_X(tuplets.size() == 1, "MidiTuplet::findTupletContainsTime",
                 "More than one tuplet was found for time moment");

      return tuplets.front();
      }

std::set<std::pair<const ReducedFraction, MidiChord> *>
findTupletChords(const std::vector<TupletInfo> &tuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> tupletChords;
      for (const auto &tupletInfo: tuplets) {
            for (const auto &tupletChord: tupletInfo.chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert(&*tupletIt);
                  }
            }
      return tupletChords;
      }

bool isChordBelongToThisBar(
            const ReducedFraction &chordOnTime,
            const ReducedFraction &barStart,
            int chordBarIndex,
            int currentBarIndex)
      {
      return chordOnTime >= barStart || chordBarIndex == currentBarIndex;
      }

std::list<std::multimap<ReducedFraction, MidiChord>::iterator>
findNonTupletChords(
            const std::vector<TupletInfo> &tuplets,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &barStart,
            int barIndex)
      {
      const auto tupletChords = findTupletChords(tuplets);
      std::list<std::multimap<ReducedFraction, MidiChord>::iterator> nonTuplets;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
                        // because of tol chord on time can belong to previous bar
                        // so don't use it as non-tuplet chord
            if (tupletChords.find(&*it) == tupletChords.end()
                        && isChordBelongToThisBar(it->first, barStart,
                                                  it->second.barIndex, barIndex)) {
                  nonTuplets.push_back(it);
                  }
            }
                  // some chords that belong to the current bar can be found
                  // even earlier; these chords have bar index == current bar index
      if (startBarChordIt != chords.begin()) {
            auto it = std::prev(startBarChordIt);
            while (true) {
                  if (it->second.barIndex == barIndex) {

                        Q_ASSERT_X(!it->second.isInTuplet, "MidiTuplet::findNonTupletChords",
                                   "Tuplet chord was assigned to the wrong (next) bar");

                        nonTuplets.push_back(it);
                        }
                  if (it == chords.begin() || it->second.barIndex < barIndex - 1)
                        break;
                  --it;
                  }
            }

      return nonTuplets;
      }

// do quantization before checking

bool hasIntersectionWithChord(
            const ReducedFraction &startTick,
            const ReducedFraction &endTick,
            const ReducedFraction &basicQuant,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chord)
      {
      const auto onTime = Quantize::findQuantizedChordOnTime(*chord, basicQuant);
      const auto offTime = Quantize::findMaxQuantizedOffTime(*chord, basicQuant);
      return (endTick > onTime && startTick < offTime);
      }

// split first tuplet chord, that belong to 2 tuplets, into 2 chords

void splitTupletChord(const std::vector<TupletInfo>::iterator &lastMatch,
                      std::multimap<ReducedFraction, MidiChord> &chords)
      {
      auto &chordEvent = lastMatch->chords.begin()->second;
      MidiChord &prevChord = chordEvent->second;
      const auto onTime = chordEvent->first;
      MidiChord newChord = prevChord;
                        // erase all notes except the first one
      auto beg = newChord.notes.begin();
      newChord.notes.erase(++beg, newChord.notes.end());
                        // erase the first note
      prevChord.notes.erase(prevChord.notes.begin());
      chordEvent = chords.insert({onTime, newChord});

      Q_ASSERT_X(!prevChord.notes.isEmpty(),
                 "MidiTuplet::splitTupletChord",
                 "Tuplets were not filtered correctly: same notes in different tuplets");
      }

void splitFirstTupletChords(std::vector<TupletInfo> &tuplets,
                            std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            auto lastMatch = tuplets.end();
            const auto nowChordIt = now->chords.begin();
            for (auto prev = tuplets.begin(); prev != now; ++prev) {
                  auto prevChordIt = prev->chords.begin();
                  if (now->firstChordIndex == 0
                              && prev->firstChordIndex == 0
                              && nowChordIt->second == prevChordIt->second) {
                        lastMatch = prev;
                        }
                  }
            if (lastMatch != tuplets.end())
                  splitTupletChord(lastMatch, chords);
            }
      }

// first tuplet notes with offTime quantization error,
// that is greater for tuplet quant rather than for regular quant,
// are removed from tuplet, except that was the last note

// if noteLen <= tupletLen
//     remove notes with big offTime quant error;
//     and here is a tuning for clearer tuplet processing:
//         if tuplet has only one (first) chord -
//             we can remove all notes and erase tuplet;
//         if tuplet has multiple chords -
//             we should leave at least one note in the first chord

void minimizeOffTimeError(
            std::vector<TupletInfo> &tuplets,
            std::multimap<ReducedFraction, MidiChord> &chords,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant,
            int barIndex)
      {
      for (auto it = tuplets.begin(); it != tuplets.end(); ) {
            TupletInfo &tupletInfo = *it;
            const auto firstChord = tupletInfo.chords.begin();
            if (firstChord == tupletInfo.chords.end() || tupletInfo.firstChordIndex != 0) {
                  ++it;
                  continue;
                  }
            auto onTime = firstChord->second->first;
                        // because of tol onTime can be less than start bar tick
            if (onTime < startBarTick)
                  onTime = startBarTick;
            MidiChord &midiChord = firstChord->second->second;
            auto &notes = midiChord.notes;

            std::vector<int> removedIndexes;
            std::vector<int> leavedIndexes;
            const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;

            for (int i = 0; i != notes.size(); ++i) {
                  const auto &note = notes[i];
                  if (note.offTime - onTime <= tupletInfo.len
                              && note.offTime - onTime > tupletNoteLen) {
                                    // if note is longer than tuplet note length
                                    // then it's simpler to move it outside the tuplet
                                    // if the error is not larger than error inside tuplet
                        if ((tupletInfo.chords.size() == 1
                                    && notes.size() > (int)removedIndexes.size())
                                 || (tupletInfo.chords.size() > 1
                                    && notes.size() > (int)removedIndexes.size() + 1))
                              {
                              const auto tupletError = Quantize::findOffTimeTupletQuantError(
                                                onTime, note.offTime, tupletInfo.len,
                                                tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
                              const auto regularError = Quantize::findOffTimeQuantError(
                                                *firstChord->second, note.offTime, basicQuant);

                              if (tupletError >= regularError) {
                                    removedIndexes.push_back(i);
                                    continue;
                                    }
                              }
                        }
                  leavedIndexes.push_back(i);
                  }
            if (!removedIndexes.empty()) {
                  MidiChord newTupletChord;
                  for (int i: leavedIndexes)
                        newTupletChord.notes.push_back(notes[i]);

                  QList<MidiNote> newNotes;
                  for (int i: removedIndexes)
                        newNotes.push_back(notes[i]);
                  notes = newNotes;
                  if (isChordBelongToThisBar(firstChord->first, startBarTick,
                                             firstChord->second->second.barIndex, barIndex)) {
                        nonTuplets.push_back(firstChord->second);
                        }
                  if (!newTupletChord.notes.empty())
                        firstChord->second = chords.insert({onTime, newTupletChord});
                  else {
                        tupletInfo.chords.erase(tupletInfo.chords.begin());
                        if (tupletInfo.chords.empty()) {
                              it = tuplets.erase(it);   // remove tuplet without chords
                              continue;
                              }
                        }
                  }
            ++it;
            }
      }

void addChordsBetweenTupletNotes(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      for (TupletInfo &tuplet: tuplets) {
            for (auto it = nonTuplets.begin(); it != nonTuplets.end(); ) {
                  const auto &chordIt = *it;
                  const auto &onTime = chordIt->first;
                  if (onTime > tuplet.onTime
                              && hasIntersectionWithChord(tuplet.onTime, tuplet.onTime + tuplet.len,
                                                          basicQuant, chordIt)) {
                        const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;

                        auto tupletError = Quantize::findOnTimeTupletQuantError(
                                                  *chordIt, tuplet.len, tupletRatio, startBarTick);
                        auto regularError = Quantize::findOnTimeQuantError(*chordIt, basicQuant);

                        const auto offTime = MChord::maxNoteOffTime(chordIt->second.notes);
                        if (offTime < tuplet.onTime + tuplet.len) {
                              tupletError += Quantize::findOffTimeTupletQuantError(
                                             onTime, offTime, tuplet.len, tupletRatio, startBarTick);
                              regularError += Quantize::findOffTimeQuantError(
                                                *chordIt, offTime, basicQuant);
                              }
                        if (tupletError < regularError) {
                              tuplet.chords.insert({onTime, chordIt});
                              it = nonTuplets.erase(it);
                              continue;
                              }
                        }
                  ++it;
                  }
            }
      }


#ifdef QT_DEBUG

bool doTupletsHaveCommonChords(const std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return false;
      std::set<std::pair<const ReducedFraction, MidiChord> *> chordsI;
      for (const auto &tuplet: tuplets) {
            for (const auto &chord: tuplet.chords) {
                  if (chordsI.find(&*chord.second) != chordsI.end())
                        return true;
                  chordsI.insert(&*chord.second);
                  }
            }
      return false;
      }

bool areBarIndexesSet(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chord: chords) {
            if (chord.second.barIndex == -1)
                  return false;
            }
      return true;
      }

bool areAllTupletsReferenced(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      std::set<std::pair<ReducedFraction, int>> referencedTuplets;      // <onTime, voice>
                  // first - check tuplet events for uniqueness
      for (const auto &tuplet: tupletEvents) {
            const auto &t = tuplet.second;
            const auto result = referencedTuplets.insert({t.onTime, t.voice});
            Q_ASSERT_X(result.second == true, "MidiTuplet::areAllTupletsReferenced",
                       "Not unique tuplets in tupletEvents");
            }
      referencedTuplets.clear();

      for (const auto &chord: chords) {
            if (chord.second.isInTuplet) {
                  const auto &tuplet = chord.second.tuplet->second;
                  referencedTuplets.insert({tuplet.onTime, tuplet.voice});
                  }
            for (const auto &note: chord.second.notes) {
                  if (note.isInTuplet) {
                        const auto &tuplet = note.tuplet->second;
                        referencedTuplets.insert({tuplet.onTime, tuplet.voice});
                        }
                  }
            }

      if (referencedTuplets.size() < tupletEvents.size()) {
            qDebug() << "Referenced tuplets count ("
                     << referencedTuplets.size() <<
                        ") < tuplet events count ("
                     << tupletEvents.size() << ")";
            }
      if (referencedTuplets.size() > tupletEvents.size()) {
            qDebug() << "Referenced tuplets count ("
                     << referencedTuplets.size() <<
                        ") > tuplet events count ("
                     << tupletEvents.size() << ")";
            }

      return tupletEvents.size() == referencedTuplets.size();
      }

// this check is not full but often if use invalid iterator for comparison
// it causes crash or something

bool areTupletReferencesValid(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chord: chords) {
            const MidiChord &c = chord.second;
            if (c.isInTuplet) {
                  const auto &tuplet = c.tuplet->second;
                  if (tuplet.onTime < ReducedFraction(0, 1)
                              || tuplet.len < ReducedFraction(0, 1))
                        return false;
                  }
            for (const auto &note: c.notes) {
                  if (note.isInTuplet) {
                        const auto &tuplet = note.tuplet->second;
                        if (tuplet.onTime < ReducedFraction(0, 1)
                                    || tuplet.len < ReducedFraction(0, 1))
                              return false;
                        }
                  }
            }
      return true;
      }

bool areTupletNonTupletChordsDistinct(
            const std::vector<TupletInfo> &tuplets,
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> chords;
      for (const TupletInfo &tuplet: tuplets) {
            for (const auto &chord: tuplet.chords) {
                  chords.insert(&*chord.second);
                  }
            }
      for (const auto &nonTuplet: nonTuplets) {
            const auto it = chords.find(&*nonTuplet);
            if (it != chords.end())
                  return false;
            }
      return true;
      }

#endif


void addTupletEvents(std::multimap<ReducedFraction, TupletData> &tupletEvents,
                     const std::vector<TupletInfo> &tuplets,
                     const std::list<TiedTuplet> &backTiedTuplets)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            const auto &tupletInfo = tuplets[i];
            TupletData tupletData = {
                  tupletInfo.chords.begin()->second->second.voice,
                  tupletInfo.onTime,
                  tupletInfo.len,
                  tupletInfo.tupletNumber,
                  {}
                  };

            const auto it = tupletEvents.insert({tupletData.onTime, tupletData});
            for (auto &chord: tupletInfo.chords) {
                  MidiChord &midiChord = chord.second->second;
                  midiChord.tuplet = it;

                  Q_ASSERT_X(!midiChord.isInTuplet,
                             "MidiTuplet::addTupletEvents",
                             "Chord is already in tuplet but it shouldn't");

                  midiChord.isInTuplet = true;
                  }

            for (const TiedTuplet &tiedTuplet: backTiedTuplets) {
                  if (tiedTuplet.tupletId == tupletInfo.id) {
                        MidiChord &midiChord = tiedTuplet.chord->second;

#ifdef QT_DEBUG
                        QString message = "Tied tuplet and tied chord have different voices, "
                                          "tuplet voice = ";
                        message += QString::number(tiedTuplet.voice) + ", chord voice = ";
                        message += QString::number(midiChord.voice) + ", bar number (from 1) = ";
                        message += QString::number(midiChord.barIndex + 1);
#endif
                        Q_ASSERT_X(tiedTuplet.voice == midiChord.voice,
                                   "MidiTuplet::addTupletEvents", message.toAscii().data());

                        for (int j: tiedTuplet.tiedNoteIndexes) {
                              midiChord.notes[j].tuplet = it;
                              midiChord.notes[j].isInTuplet = true;
                              }
                        break;
                        }
                  }

            for (auto &chord: tupletInfo.chords) {
                  MidiChord &midiChord = chord.second->second;
                  for (auto &note: midiChord.notes) {
                        if (note.offTime <= tupletInfo.onTime + tupletInfo.len) {

                              Q_ASSERT_X(!note.isInTuplet,
                                         "MidiTuplet::addTupletEvents",
                                         "Note is already in tuplet but it shouldn't");

                              note.tuplet = it;
                              note.isInTuplet = true;
                              }
                        }
                  }
            }
      }

// check is the chord already in tuplet in prev bar or division
// it's possible because we use (startDivTick - tol) as a start tick

std::multimap<ReducedFraction, MidiChord>::iterator
findTupletFreeChord(
            const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt,
            const ReducedFraction &startDivTick)
      {
      auto result = startChordIt;
      for (auto it = startChordIt; it != endChordIt && it->first < startDivTick; ++it) {
            if (it->second.isInTuplet)
                  result = std::next(it);
            }
      return result;
      }

void markStaccatoTupletNotes(std::vector<TupletInfo> &tuplets)
      {
      for (auto &tuplet: tuplets) {
            for (const auto &staccato: tuplet.staccatoChords) {
                  const auto it = tuplet.chords.find(staccato.first);
                  if (it != tuplet.chords.end()) {
                        MidiChord &midiChord = it->second->second;
                        midiChord.notes[staccato.second].staccato = true;
                        }
                  }
            tuplet.staccatoChords.clear();
            }
      }

// if notes with staccato were in tuplets previously - remove staccato

void cleanStaccatoOfNonTuplets(
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      for (auto &nonTuplet: nonTuplets) {
            for (auto &note: nonTuplet->second.notes) {
                  if (note.staccato)
                        note.staccato = false;
                  }
            }
      }

ReducedFraction findOnTimeBetweenChords(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction onTime(-1, 1);
      auto quant = basicQuant;

      const auto range = chords.equal_range(chord.first);
      auto chordIt = chords.end();

      for (auto it = range.first; it != range.second; ++it) {
            if (it->second.voice == chord.second.voice) {
                  chordIt = it;
                  break;
                  }
            }

      Q_ASSERT_X(chordIt != chords.end(),
                 "MidiTuplet::findOnTimeBetweenChords", "Chord iterator was not found");

            // chords with equal voices here can have equal on time values
            // so skip such chords

      const int voice = chordIt->second.voice;
      while (true) {
            onTime = Quantize::findMinQuantizedOnTime(*chordIt, quant);
            bool changed = false;

            if (chordIt != chords.begin()) {
                  auto it = std::prev(chordIt);
                  while (true) {
                        if (it->first < chord.first && it->second.voice == voice) {
                              if (onTime < it->first) {

                                    Q_ASSERT_X(quant >= MChord::minAllowedDuration() * 2,
                                               "MidiTuplet::findOnTimeBetweenChords",
                                               "Too small quantization value");

                                    quant /= 2;
                                    changed = true;
                                    }
                              break;
                              }
                        if (it == chords.begin() || it->first < chord.first - basicQuant * 2)
                              break;
                        --it;
                        }
                  }

            if (!changed) {
                  for (auto it = std::next(chordIt);
                              it != chords.end() && it->first < chord.first + basicQuant * 2; ++it) {
                        if (it->first == chord.first || it->second.voice != voice)
                              continue;
                        if (onTime > it->first) {

                              Q_ASSERT_X(quant >= MChord::minAllowedDuration() * 2,
                                         "MidiTuplet::findOnTimeBetweenChords",
                                         "Too small quantization value");

                              quant /= 2;
                              changed = true;
                              }
                        break;
                        }
                  }

            if (!changed)
                  break;
            }

      Q_ASSERT_X(onTime != ReducedFraction(-1, 1), "MidiTuplet::findOnTimeBetweenChords",
                 "On time for chord interval was not found");

      return onTime;
      }

void setBarIndexes(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            int barIndex,
            const ReducedFraction &endBarTick,
            const ReducedFraction &basicQuant)
      {
      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  chord.second->second.barIndex = barIndex;
                  }
            }

      for (auto &chord: nonTuplets) {
            const auto onTime = findOnTimeBetweenChords(*chord, chords, basicQuant);
            if (onTime >= endBarTick)
                  chord->second.barIndex = barIndex + 1;
            else
                  chord->second.barIndex = barIndex;
            }
      }

ReducedFraction findPrevBarStart(const ReducedFraction &barStart,
                                 const ReducedFraction &barLen)
      {
      auto prevBarStart = barStart - barLen;
      if (prevBarStart < ReducedFraction(0, 1))
            prevBarStart = ReducedFraction(0, 1);
      return prevBarStart;
      }

// indexes of each new bar should be -1 except possibly chords at the end of prev bar

void findTuplets(
            const ReducedFraction &startBarTick,
            const ReducedFraction &endBarTick,
            const ReducedFraction &barFraction,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant,
            std::multimap<ReducedFraction, TupletData> &tupletEvents,
            int barIndex)
      {
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return;
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      if (!operations.tuplets.doSearch)
            return;

      const auto tol = basicQuant / 2;
      auto startBarChordIt = MChord::findFirstChordInRange(chords, startBarTick - tol, endBarTick);
      startBarChordIt = findTupletFreeChord(startBarChordIt, chords.end(), startBarTick);

      if (startBarChordIt == chords.end())      // no chords in this bar
            return;
      const auto endBarChordIt = chords.lower_bound(endBarTick);
      std::vector<TupletInfo> tuplets = detectTuplets(chords, barFraction, startBarTick, tol,
                                                      endBarChordIt, startBarChordIt, basicQuant);
      if (tuplets.empty())
            return;

      filterTuplets(tuplets, basicQuant);

            // later notes will be sorted and their indexes become invalid
            // so assign staccato information to notes now
      if (operations.simplifyDurations)
            markStaccatoTupletNotes(tuplets);

            // because of tol for non-tuplets we should use only chords with onTime >= bar start
            // or with current bar index (can be set from prev bar)
      auto startNonTupletChordIt = startBarChordIt;
      while (startNonTupletChordIt != chords.end()
                        && startNonTupletChordIt->first < startBarTick
                        && startNonTupletChordIt->second.barIndex < barIndex) {
            ++startNonTupletChordIt;
            }

      auto nonTuplets = findNonTupletChords(tuplets, startNonTupletChordIt,
                                            endBarChordIt, chords, startBarTick, barIndex);

      resetTupletVoices(tuplets);  // because of tol some chords may have non-zero voices
      addChordsBetweenTupletNotes(tuplets, nonTuplets, startBarTick, basicQuant);
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      if (tupletVoiceLimit() > 1) {
            splitFirstTupletChords(tuplets, chords);
            minimizeOffTimeError(tuplets, chords, nonTuplets, startBarTick, basicQuant, barIndex);
            }

      if (operations.simplifyDurations)
            cleanStaccatoOfNonTuplets(nonTuplets);

      Q_ASSERT_X(!doTupletsHaveCommonChords(tuplets),
                 "MIDI tuplets: findTuplets", "Tuplets have common chords but they shouldn't");

      const auto prevBarStart = findPrevBarStart(startBarTick, endBarTick - startBarTick);
      auto backTiedTuplets = findBackTiedTuplets(chords, tuplets, prevBarStart,
                                                 startBarTick, basicQuant, barIndex);
                  // backTiedTuplets can be changed here (incompatible are removed)
      assignVoices(tuplets, nonTuplets, backTiedTuplets,
                   chords, startBarTick, basicQuant, barIndex);

      Q_ASSERT_X(areTupletNonTupletChordsDistinct(tuplets, nonTuplets),
                 "MIDI tuplets: findTuplets", "Tuplets have common chords with non-tuplets");

      setBarIndexes(tuplets, nonTuplets, chords, barIndex, endBarTick, basicQuant);

      addTupletEvents(tupletEvents, tuplets, backTiedTuplets);
      }

void setAllTupletOffTimes(
            std::multimap<ReducedFraction, TupletData> &tupletEvents,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap)
      {
      for (auto &chordEvent: chords) {
            MidiChord &chord = chordEvent.second;
            for (MidiNote &note: chord.notes) {
                  if (note.isInTuplet)
                        continue;
                  const auto barEnd = ReducedFraction::fromTicks(
                                    sigmap->bar2tick(chord.barIndex + 1, 0));
                  if (note.offTime > barEnd) {
                        const auto it = findTupletContainsTime(
                                          chord.voice, note.offTime, tupletEvents);
                        if (it != tupletEvents.end()) {
                              note.isInTuplet = true;
                                          // hack to remove constness of iterator
                              note.tuplet = tupletEvents.erase(it, it);
                              }
                        }
                  }
            }
      }

void findAllTuplets(
            std::multimap<ReducedFraction, TupletData> &tupletEvents,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const ReducedFraction &lastTick,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction startBarTick = {0, 1};

      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            const auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(i, 0));
            const auto barFraction = ReducedFraction(
                              sigmap->timesig(startBarTick.ticks()).timesig());
                        // look for tuplets
            findTuplets(startBarTick, endBarTick, barFraction,
                        chords, basicQuant, tupletEvents, i - 1);
                        // if bar indexes == -1 (no tuplets, for example) - set them
            const auto startBarChordIt = MChord::findFirstChordInRange(
                                          chords, startBarTick, endBarTick);
            if (startBarChordIt != chords.end()) {
                  const auto endBarChordIt = chords.lower_bound(endBarTick);
                  for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
                        if (it->second.barIndex == -1)
                              it->second.barIndex = i - 1;
                        }
                  }

            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
                  // check if there are not detected off times inside tuplets
      setAllTupletOffTimes(tupletEvents, chords, sigmap);

      Q_ASSERT_X(areBarIndexesSet(chords),
                 "MidiTuplet::findAllTuplets", "Not all bar indexes were set");
      Q_ASSERT_X(areAllTupletsReferenced(chords, tupletEvents),
                 "MidiTuplet::findAllTuplets",
                 "Not all tuplets are referenced in chords or notes");
      }

} // namespace MidiTuplet
} // namespace Ms
