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
findNonTupletChords(const std::vector<TupletInfo> &tuplets,
                    const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt)
      {
      const auto tupletChords = findTupletChords(tuplets);
      std::list<std::multimap<ReducedFraction, MidiChord>::iterator> nonTuplets;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            if (tupletChords.find(&*it) == tupletChords.end())
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
      if (prevChord.notes.isEmpty()) {
                        // normally this should not happen at all because of filtering of tuplets
            qDebug("Tuplets were not filtered correctly: same notes in different tuplets");
            }
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
            for (int i = 0; i != notes.size(); ++i) {
                  const auto &note = notes[i];
                  if (note.offTime - onTime <= tupletInfo.len) {
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

                              if (tupletError > regularError) {
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


void convertToData(std::multimap<ReducedFraction, TupletData> &tupletEvents,
                   const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tupletInfo: tuplets) {
            MidiTuplet::TupletData tupletData = {tupletInfo.chords.begin()->second->second.voice,
                                                 tupletInfo.onTime,
                                                 tupletInfo.len,
                                                 tupletInfo.tupletNumber,
                                                 {}};
            tupletEvents.insert({tupletData.onTime, tupletData});
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

void findTupletQuantizedOffTime(
            std::vector<TupletInfo> &tuplets,
            const ReducedFraction &barStart,
            const ReducedFraction &barEnd,
            const ReducedFraction &basicQuant)
      {
      for (auto &tuplet: tuplets) {
            const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
            const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;
            for (auto it = tuplet.chords.begin(); it != tuplet.chords.end(); ++it) {
                  MidiChord &midiChord = it->second->second;
                  for (auto &note: midiChord.notes) {
                        if (note.staccato) {
                                    // decrease tuplet error by enlarging staccato notes:
                                    // make note.len = tuplet note length
                              auto offTime = Quantize::findQuantizedNoteOffTime(
                                          *it->second, it->first + tupletNoteLen, basicQuant,
                                          tupletRatio, barStart);
                              auto next = std::next(it);
                              if (next != tuplet.chords.end()) {
                                    const auto nextOnTime = next->second->second.quantizedOnTime;

                                    Q_ASSERT_X(nextOnTime != ReducedFraction(-1, 1),
                                         "MidiTuplet::findTupletQuantizedOffTime",
                                         "Tuplet onTime is not quantized but it should at this time");

                                    if (offTime > nextOnTime)
                                          offTime = nextOnTime;
                                    }
                              note.quantizedOffTime = offTime;
                              }
                        else {
                              if (note.offTime <= tuplet.onTime + tuplet.len) {
                                    note.quantizedOffTime = Quantize::findQuantizedNoteOffTime(
                                                *it->second, note.offTime, basicQuant,
                                                tupletRatio, barStart);
                                    }
                              // outside bar quantization cases will be considered later
                              }
                        }
                  }
            }
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

void findTupletQuantizedOnTime(
            std::vector<TupletInfo> &tuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  if (chord.first < startBarTick) {
                        chord.second->second.quantizedOnTime = startBarTick;
                        }
                  else {
                        chord.second->second.quantizedOnTime = Quantize::findQuantizedChordOnTime(
                                          *chord.second, basicQuant,
                                          tupletLimits(tuplet.tupletNumber).ratio, startBarTick);
                        }

                  Q_ASSERT_X(chord.second->second.quantizedOnTime >= tuplet.onTime,
                             "MidiTuplet::findTupletQuantizedOnTime",
                             "Chord onTime value is less than tuplet begin time");
                  }
            }
      }

void findNonTupletQuantizedOnTime(
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant)
      {
      for (auto &nonTuplet: nonTuplets) {
            nonTuplet->second.quantizedOnTime = Quantize::findQuantizedChordOnTime(
                                                      *nonTuplet, basicQuant);
            }
      }

void findTuplets(
            const ReducedFraction &startBarTick,
            const ReducedFraction &endBarTick,
            const ReducedFraction &barFraction,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant,
            std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return;
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      if (!operations.tuplets.doSearch)
            return;
      auto startBarChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
      startBarChordIt = findTupletFreeChord(startBarChordIt, chords.end(), startBarTick);
      if (startBarChordIt == chords.end())      // no chords in this bar
            return;

      const auto endBarChordIt = chords.lower_bound(endBarTick);
                  // update start chord: use chords with onTime >= (start bar tick - tol)
      const auto tol = basicQuant / 2;
      startBarChordIt = MChord::findFirstChordInRange(chords, startBarTick - tol, endBarTick);

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
      auto nonTuplets = findNonTupletChords(tuplets, startNonTupletChordIt, endBarChordIt);
      if (tupletVoiceLimit() == 1)
            excludeExtraVoiceTuplets(tuplets, nonTuplets, basicQuant);

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

      assignVoices(tuplets, nonTuplets, startBarTick, endBarTick, basicQuant, chords);

      findTupletQuantizedOnTime(tuplets, startBarTick, basicQuant);
      findTupletQuantizedOffTime(tuplets, startBarTick, endBarTick, basicQuant);
      findNonTupletQuantizedOnTime(nonTuplets, basicQuant);

      convertToData(tupletEvents, tuplets);
      }

std::multimap<ReducedFraction, TupletData>
findAllTuplets(std::multimap<ReducedFraction, MidiChord> &chords,
               const TimeSigMap *sigmap,
               const ReducedFraction &lastTick,
               const ReducedFraction &basicQuant)
      {
      std::multimap<ReducedFraction, TupletData> tupletEvents;
      ReducedFraction startBarTick = {0, 1};

      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            const auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(i, 0));
            const auto barFraction = ReducedFraction(
                              sigmap->timesig(startBarTick.ticks()).timesig());
            findTuplets(startBarTick, endBarTick, barFraction,
                        chords, basicQuant, tupletEvents);
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      return tupletEvents;
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

} // namespace MidiTuplet
} // namespace Ms
