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
            {2, {{2, 3}, 1, 2, 2}},
            {3, {{3, 2}, 1, 3, 3}},
            {4, {{4, 3}, 3, 4, 3}},
            {5, {{5, 4}, 3, 4, 4}},
            {7, {{7, 8}, 4, 6, 5}},
            {9, {{9, 8}, 6, 7, 7}}
            };
      return values;
      }

const TupletLimits& tupletLimits(int tupletNumber)
      {
      auto it = tupletsLimits().find(tupletNumber);

      Q_ASSERT_X(it != tupletsLimits().end(), "MidiTuplet::tupletValue", "Unknown tuplet");

      return it->second;
      }

// tuplets with no chords are removed
// tuplets with single chord with chord.onTime = tuplet.onTime
//    and chord.len = tuplet.len are removed as well

void removeEmptyTuplets(MTrack &track)
      {
      if (track.tuplets.empty())
            return;
      for (auto it = track.tuplets.begin(); it != track.tuplets.end(); ) {
            const auto &tupletData = it->second;
            bool ok = false;
            for (const auto &chord: track.chords) {
                  if (chord.first >= tupletData.onTime + tupletData.len)
                        break;
                  if (tupletData.voice != chord.second.voice)
                        continue;
                  const ReducedFraction &onTime = chord.first;
                  if (onTime >= tupletData.onTime
                              && onTime < tupletData.onTime + tupletData.len) {
                                    // tuplet contains at least one chord
                                    // check now for notes with len == tupletData.len
                        if (onTime == tupletData.onTime) {
                              for (const auto &note: chord.second.notes) {
                                    if (note.offTime - onTime < tupletData.len) {
                                          ok = true;
                                          break;
                                          }
                                    }
                              }
                        else {
                              ok = true;
                              break;
                              }
                        }
                  }
            if (!ok) {
                  it = track.tuplets.erase(it);
                  continue;
                  }
            ++it;
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
      return sumPitch / noteCounter;
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

std::multimap<ReducedFraction, MidiTuplet::TupletData>::const_iterator
findTupletForTimeRange(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &len,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      Q_ASSERT_X(len >= ReducedFraction(0, 1),
                 "MidiTuplet::findTupletForTimeRange", "Negative length of time range");

      if (tupletEvents.empty())
            return tupletEvents.end();

      auto it = tupletEvents.upper_bound(onTime + len);
      if (it != tupletEvents.begin())
            --it;
      while (true) {
            const auto &tupletData = it->second;
            const auto &tupletOffTime = tupletData.onTime + tupletData.len;
            if (tupletData.voice == voice
                        && onTime >= tupletData.onTime
                        && ((len > ReducedFraction(0, 1)
                             ? onTime + len <= tupletOffTime
                             : onTime < tupletOffTime)
                            )) {
                  return it;
                  }
            if (it == tupletEvents.begin())
                  break;
            --it;
            }
      return tupletEvents.end();
      }

std::multimap<ReducedFraction, MidiTuplet::TupletData>::const_iterator
findTupletContainsTime(
            int voice,
            const ReducedFraction &time,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      return findTupletForTimeRange(voice, time, ReducedFraction(0, 1), tupletEvents);
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

std::list<std::multimap<ReducedFraction, MidiChord>::iterator>
findNonTupletChords(
            const std::vector<TupletInfo> &tuplets,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
            const ReducedFraction &barStart)
      {
      const auto tupletChords = findTupletChords(tuplets);
      std::list<std::multimap<ReducedFraction, MidiChord>::iterator> nonTuplets;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
                        // because of tol chord on time can belong to previous bar
                        // so don't use it as non-tuplet chord
            if (tupletChords.find(&*it) == tupletChords.end() && it->first >= barStart)
                  nonTuplets.push_back(it);
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
            const ReducedFraction &basicQuant)
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
                              const auto tupletError = Quantize::findOffTimeQuantError(
                                                *firstChord->second, note.offTime, basicQuant,
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
                  if (firstChord->first >= startBarTick)
                        nonTuplets.push_back(firstChord->second);
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

                        auto tupletError = Quantize::findOnTimeQuantError(
                                                  *chordIt, basicQuant, tupletRatio, startBarTick);
                        auto regularError = Quantize::findOnTimeQuantError(*chordIt, basicQuant);

                        const auto offTime = MChord::maxNoteOffTime(chordIt->second.notes);
                        if (offTime < tuplet.onTime + tuplet.len) {
                              tupletError += Quantize::findOffTimeQuantError(
                                                *chordIt, offTime, basicQuant,
                                                tupletRatio, startBarTick);
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

#endif


void addTupletEvents(std::multimap<ReducedFraction, TupletData> &tupletEvents,
                     const std::vector<TupletInfo> &tuplets,
                     const std::vector<TiedTuplet> &backTiedTuplets)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            const auto &tupletInfo = tuplets[i];
            MidiTuplet::TupletData tupletData = {
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

            for (const TiedTuplet &tuplet: backTiedTuplets) {
                  if (tuplet.tupletIndex == i) {
                        MidiChord &midiChord = tuplet.chord->second;

                        Q_ASSERT_X(tuplet.voice == midiChord.voice,
                                   "MidiTuplet::addTupletEvents",
                                   "Tied tuplet and tied chord have different voices");

                        for (int j: tuplet.tiedNoteIndexes) {
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

std::map<std::pair<const ReducedFraction, MidiChord> *, int>
findMappedTupletChords(const std::vector<TupletInfo> &tuplets)
      {
                  // <chord address, tupletIndex>
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> tupletChords;
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            for (const auto &tupletChord: tuplets[i].chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert({&*tupletIt, i});
                  }
            }
      return tupletChords;
      }

// result: tied notes indexes

std::vector<int> findTiedNotes(
            const TupletInfo &tuplet,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      std::vector<int> tiedNotes;
      const auto onTime = Quantize::findQuantizedChordOnTime(*chordIt, basicQuant);
      if (onTime >= tuplet.onTime)
            return tiedNotes;

      for (int i = 0; i != chordIt->second.notes.size(); ++i) {
            const MidiNote &note = chordIt->second.notes[i];

            const auto tupletOffTime = Quantize::findQuantizedNoteOffTime(
                              *chordIt, note.offTime, basicQuant,
                              tupletLimits(tuplet.tupletNumber).ratio, startBarTick);

            if (tupletOffTime < startBarTick
                        || tupletOffTime <= tuplet.onTime
                        || tupletOffTime >= tuplet.onTime + tuplet.len)
                  continue;

            const auto regularOffTime = Quantize::findQuantizedNoteOffTime(
                                                *chordIt, note.offTime, basicQuant);
            const auto regularError = (note.offTime - regularOffTime).absValue();
            const auto tupletError = (note.offTime - tupletOffTime).absValue();
            if (tupletError > regularError)
                  continue;

            tiedNotes.push_back(i);
            }

      return tiedNotes;
      }

std::vector<TiedTuplet>
findBackTiedTuplets(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &prevBarStart,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      std::vector<TiedTuplet> tiedTuplets;
      const auto tupletChords = findMappedTupletChords(tuplets);
      const auto tupletIntervals = findTupletIntervals(tuplets, basicQuant);

      for (int i = 0; i != (int)tuplets.size(); ++i) {

            Q_ASSERT_X(!tuplets[i].chords.empty(),
                       "MidiTuplets::findBackTiedTuplets", "Tuplet chords are empty");

            auto chordIt = tuplets[i].chords.begin()->second;
            while (chordIt != chords.begin() && chordIt->first >= prevBarStart) {
                  --chordIt;
                  int voice = chordIt->second.voice;  // voice can be from prev bar also
                  bool used = false;
                              // check: if tuplet 'i' already tied then voices must match
                  for (const auto &t: tiedTuplets) {
                        if (t.tupletIndex == i && t.voice != voice) {
                              used = true;
                              break;
                              }
                        if (t.tupletIndex != i && t.voice == voice) {
                              if (haveIntersection(tupletIntervals[i],
                                                   tupletIntervals[t.tupletIndex])) {
                                    used = true;
                                    break;
                                    }
                              }
                        }
                  if (used)
                        continue;
                              // don't make back tie to the chord in overlapping tuplet
                  const auto tupletIt = tupletChords.find(&*chordIt);
                  if (tupletIt != tupletChords.end()) {
                        const auto onTime1 = tuplets[tupletIt->second].onTime;
                        const auto endTime1 = onTime1 + tuplets[tupletIt->second].len;
                        const auto onTime2 = tuplets[i].onTime;
                        const auto endTime2 = onTime1 + tuplets[i].len;
                        if (endTime1 > onTime2 && onTime1 < endTime2) // tuplet intersection
                              continue;
                        }
                  const auto tiedNotes = findTiedNotes(tuplets[i], chordIt, startBarTick, basicQuant);
                  if (!tiedNotes.empty()) {
                        tiedTuplets.push_back({i, voice, &*chordIt, tiedNotes});
                        break;
                        }
                  }
            }
      return tiedTuplets;
      }

void setBarIndex(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            int barIndex)
      {
      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  chord.second->second.barIndex = barIndex;
                  }
            }

      for (auto &chord: nonTuplets) {

            Q_ASSERT_X(chord->second.barIndex == -1,
                       "MidiTuplet::setBarIndex",
                       "Non-tuplet chord is already used in previous bar");

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
      filterTuplets(tuplets, basicQuant);

            // later notes will be sorted and their indexes become invalid
            // so assign staccato information to notes now
      markStaccatoTupletNotes(tuplets);

            // because of tol for non-tuplets we should use only chords with onTime >= bar start
      auto startNonTupletChordIt = startBarChordIt;
      while (startNonTupletChordIt->first < startBarTick)
            ++startNonTupletChordIt;
      auto nonTuplets = findNonTupletChords(tuplets, startNonTupletChordIt,
                                            endBarChordIt, startBarTick);
      if (tupletVoiceLimit() == 1)
            excludeExtraVoiceTuplets(tuplets, nonTuplets, basicQuant, startBarTick);

      resetTupletVoices(tuplets);  // because of tol some chords may have non-zero voices
      addChordsBetweenTupletNotes(tuplets, nonTuplets, startBarTick, basicQuant);
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      if (operations.useMultipleVoices) {
            splitFirstTupletChords(tuplets, chords);
            minimizeOffTimeError(tuplets, chords, nonTuplets, startBarTick, basicQuant);
            }

      cleanStaccatoOfNonTuplets(nonTuplets);

      Q_ASSERT_X(!doTupletsHaveCommonChords(tuplets),
                 "MIDI tuplets: findTuplets", "Tuplets have common chords but they shouldn't");
      Q_ASSERT_X((voiceLimit() == 1)
                        ? !haveOverlappingVoices(nonTuplets, tuplets, basicQuant)
                        : true,
                 "MIDI tuplets: findTuplets",
                 "Overlapping tuplet and non-tuplet voices for the case !useMultipleVoices");

      const auto prevBarStart = findPrevBarStart(startBarTick, endBarTick - startBarTick);
      const auto backTiedTuplets = findBackTiedTuplets(chords, tuplets, prevBarStart,
                                                       startBarTick, basicQuant);
      assignVoices(tuplets, nonTuplets, backTiedTuplets, startBarTick, basicQuant);
      setBarIndex(tuplets, nonTuplets, barIndex);

      addTupletEvents(tupletEvents, tuplets, backTiedTuplets);
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
            findTuplets(startBarTick, endBarTick, barFraction,
                        chords, basicQuant, tupletEvents, i - 1);
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      }

} // namespace MidiTuplet
} // namespace Ms
