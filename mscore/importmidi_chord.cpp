#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_chord.h"
#include "importmidi_clef.h"
#include "importmidi_operations.h"
#include "importmidi_quant.h"
#include "libmscore/mscore.h"
#include "libmscore/sig.h"
#include "preferences.h"

#include <set>


namespace Ms {
namespace MChord {

bool isGrandStaffProgram(int program)
      {
      const static std::set<int> grandStaffPrograms = {
                  // Piano
              0, 1, 2, 3, 4, 5, 6, 7
                  // Chromatic Percussion
            , 8, 10, 11, 12, 13, 15
                  // Organ
            , 16, 17, 18, 19, 20, 21, 23
                  // Strings
            , 46
                  // Ensemble
            , 50, 51, 54
                  // Brass
            , 62, 63
                  // Synth Lead
            , 80, 81, 82, 83, 84, 85, 86, 87
                  // Synth Pad
            , 88, 89, 90, 91, 92, 93, 94, 95
                  // Synth Effects
            , 96, 97, 98, 99, 100, 101, 102, 103
            };

      return grandStaffPrograms.find(program) != grandStaffPrograms.end();
      }

std::multimap<ReducedFraction, MidiChord>::iterator
findFirstChordInRange(std::multimap<ReducedFraction, MidiChord> &chords,
                      const ReducedFraction &startRangeTick,
                      const ReducedFraction &endRangeTick)
      {
      auto iter = chords.lower_bound(startRangeTick);
      if (iter != chords.end() && iter->first >= endRangeTick)
            iter = chords.end();
      return iter;
      }

std::multimap<ReducedFraction, MidiChord>::const_iterator
findFirstChordInRange(const std::multimap<ReducedFraction, MidiChord> &chords,
                      const ReducedFraction &startRangeTick,
                      const ReducedFraction &endRangeTick)
      {
      auto iter = chords.lower_bound(startRangeTick);
      if (iter != chords.end() && iter->first >= endRangeTick)
            iter = chords.end();
      return iter;
      }

const ReducedFraction& minAllowedDuration()
      {
      const static auto minDuration = ReducedFraction::fromTicks(MScore::division) / 32;
      return minDuration;
      }

ReducedFraction minNoteOffTime(const QList<MidiNote> &notes)
      {
      if (notes.isEmpty())
            return {0, 1};
      auto it = notes.begin();
      ReducedFraction minOffTime = it->offTime;
      for (++it; it != notes.end(); ++it) {
            if (it->offTime < minOffTime)
                  minOffTime = it->offTime;
            }
      return minOffTime;
      }

ReducedFraction maxNoteOffTime(const QList<MidiNote> &notes)
      {
      ReducedFraction maxOffTime(0, 1);
      for (const auto &note: notes) {
            if (note.offTime > maxOffTime)
                  maxOffTime = note.offTime;
            }
      return maxOffTime;
      }

ReducedFraction minNoteLen(const std::pair<const ReducedFraction, MidiChord> &chord)
      {
      const auto minOffTime = minNoteOffTime(chord.second.notes);
      return minOffTime - chord.first;
      }

// remove overlapping notes with the same pitch

void removeOverlappingNotes(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.second.chords;

            Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, track.second.tuplets),
                       "MChord::removeOverlappingNotes", "Tuplet chord/note is outside tuplet "
                        "or non-tuplet chord/note is inside tuplet before overlaps remove");

            for (auto i1 = chords.begin(); i1 != chords.end(); ) {
                  auto &chord1 = i1->second;
                  const auto &onTime1 = i1->first;
                  for (auto note1It = chord1.notes.begin(); note1It != chord1.notes.end(); ) {
                        auto &note1 = *note1It;

                        for (auto i2 = std::next(i1); i2 != chords.end(); ++i2) {
                              const auto &onTime2 = i2->first;
                              if (onTime2 >= note1.offTime)
                                    break;
                              auto &chord2 = i2->second;
                              if (chord1.voice != chord2.voice)
                                    continue;
                              for (auto &note2: chord2.notes) {
                                    if (note2.pitch != note1.pitch)
                                          continue;

                                    qDebug("Midi import: overlapping events: %d+%d %d+%d",
                                           onTime1.ticks(), note1.offTime.ticks(),
                                           onTime2.ticks(), note2.offTime.ticks());

                                    note1.offTime = onTime2;

                                    if (!note1.isInTuplet && chord2.isInTuplet) {
                                          if (note1.offTime > chord2.tuplet->second.onTime) {
                                                note1.isInTuplet = true;
                                                note1.tuplet = chord2.tuplet;
                                                }
                                          }
                                    else if (note1.isInTuplet && !chord2.isInTuplet) {
                                          note1.isInTuplet = false;
                                          }

                                    i2 = std::prev(chords.end());
                                    break;
                                    }
                              }
                        if (note1.offTime - onTime1 < MChord::minAllowedDuration()) {
                              note1It = chord1.notes.erase(note1It);
                              continue;
                              }
                        ++note1It;
                        }
                  if (chord1.notes.isEmpty()) {
                        i1 = chords.erase(i1);
                        continue;
                        }
                  ++i1;
                  }

            MidiTuplet::removeEmptyTuplets(track.second);

            Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, track.second.tuplets),
                       "MChord::removeOverlappingNotes", "Tuplet chord/note is outside tuplet "
                        "or non-tuplet chord/note is inside tuplet after overlaps remove");
            }
      }


#ifdef QT_DEBUG

bool areOnTimeValuesDifferent(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      std::set<ReducedFraction> onTimes;
      for (const auto &chordEvent: chords) {
            if (onTimes.find(chordEvent.first) == onTimes.end())
                  onTimes.insert(chordEvent.first);
            else
                  return false;
            }
      return true;
      }

bool areSingleNoteChords(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chordEvent: chords) {
            if (chordEvent.second.notes.size() > 1)
                  return false;
            }
      return true;
      }

bool areNotesLongEnough(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chord: chords) {
            if (minNoteLen(chord) < minAllowedDuration())
                  return false;
            }
      return true;
      }

bool areBarIndexesSuccessive(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      int barIndex = 0;
      for (const auto &chord: chords) {
            const MidiChord &c = chord.second;
            if (c.barIndex < 0)
                  return false;
            if (c.barIndex < barIndex)
                  return false;
            barIndex = c.barIndex;
            }
      return true;
      }

bool isLastTickValid(const ReducedFraction &lastTick,
                     const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chord: chords) {
            if (maxNoteOffTime(chord.second.notes) > lastTick)
                  return false;
            }
      return true;
      }

bool isLastTickValid(const ReducedFraction &lastTick,
                     const std::multimap<int, MTrack> &tracks)
      {
      for (const auto &track: tracks) {
            if (!(isLastTickValid(lastTick, track.second.chords)))
                  return false;
            }
      return true;
      }

bool areBarIndexesSet(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chord: chords) {
            if (chord.second.barIndex == -1)
                  return false;
            }
      return true;
      }

#endif


// based on quickthresh algorithm
//
// http://www.cycling74.com/docs/max5/refpages/max-ref/quickthresh.html
// (link date 9 July 2013)
//
// here are default values for audio, in milliseconds
// for midi there will be another values, in ticks

// all notes received in the left inlet within this time period are collected into a chord
// threshTime = 40 ms

// if there are any incoming values within this amount of time
// at the end of the base thresh time,
// the threshold is extended to allow more notes to be added to the chord
// fudgeTime = 10 ms

// this is an extension value of the base thresh time, which is used if notes arrive
// in the object's inlet in the "fudge" time zone
// threshExtTime = 20 ms

void collectChords(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.second.chords;
            if (chords.empty())
                  continue;

            const auto &opers = preferences.midiImportOperations.data()->trackOpers;
            const auto minAllowedDur = minAllowedDuration();

            const auto threshTime = (opers.isHumanPerformance.value())
                                          ? minAllowedDur * 2 : minAllowedDur / 2;
            const auto fudgeTime = threshTime / 4;
            const auto threshExtTime = threshTime / 2;

            ReducedFraction currentChordStart(-1, 1);    // invalid
            ReducedFraction curThreshTime(-1, 1);
                        // if note onTime goes after max chord offTime
                        // then this is not a chord but arpeggio
            ReducedFraction maxOffTime(-1, 1);

                              // chords here should consist of a single note
                              // because notes are not united into chords yet
            Q_ASSERT_X(areSingleNoteChords(chords),
                       "MChord: collectChords", "Some chords have more than one note");

            for (auto it = chords.begin(); it != chords.end(); ) {
                  const auto &note = it->second.notes[0];

                              // short events with len < minAllowedDuration must be cleaned up
                  Q_ASSERT_X(note.offTime - it->first >= minAllowedDuration(),
                             "MChord: collectChords", "Note length is less than min allowed duration");

                  if (it->first < currentChordStart + curThreshTime) {

                                    // this branch should not be executed when it == chords.begin()
                        Q_ASSERT_X(it != chords.begin(),
                                   "MChord: collectChords", "it == chords.begin()");

                        if (it->first <= maxOffTime - minAllowedDur) {
                                          // add current note to the previous chord
                              auto prev = std::prev(it);

                              bool hasNoteWithThisPitch = false;
                              for (const auto &n: prev->second.notes) {
                                    if (n.pitch == note.pitch) {
                                          hasNoteWithThisPitch = true;
                                          break;
                                          }
                                    }
                              if (!hasNoteWithThisPitch) {
                                    prev->second.notes.push_back(note);
                                    if (note.offTime > maxOffTime)
                                          maxOffTime = note.offTime;
                                    }
                              if (it->first >= currentChordStart + curThreshTime - fudgeTime
                                          && curThreshTime == threshTime) {
                                    curThreshTime += threshExtTime;
                                    }
                              it = chords.erase(it);
                              continue;
                              }
                        }
                  currentChordStart = it->first;
                  maxOffTime = note.offTime;
                  curThreshTime = threshTime;
                  ++it;
                  }

            Q_ASSERT_X(areOnTimeValuesDifferent(chords),
                       "MChord: collectChords",
                       "onTime values of chords are equal but should be different");
            Q_ASSERT_X(areNotesLongEnough(chords),
                       "MChord::collectChords", "There are too short notes");
            }
      }

void sortNotesByPitch(std::multimap<ReducedFraction, MidiChord> &chords)
      {
      struct {
            bool operator()(const MidiNote &note1, const MidiNote &note2)
                  {
                  return note1.pitch < note2.pitch;
                  }
            } pitchSort;

      for (auto &chordEvent: chords) {
                        // in each chord sort notes by pitches
            auto &notes = chordEvent.second.notes;
            qSort(notes.begin(), notes.end(), pitchSort);
            }
      }

void sortNotesByLength(std::multimap<ReducedFraction, MidiChord> &chords)
      {
      struct {
            bool operator()(const MidiNote &note1, const MidiNote &note2)
                  {
                  return note1.offTime < note2.offTime;
                  }
            } lenSort;

      for (auto &chordEvent: chords) {
                        // in each chord sort notes by lengths
            auto &notes = chordEvent.second.notes;
            qSort(notes.begin(), notes.end(), lenSort);
            }
      }

// find notes of each chord that have different durations
// and separate them into different chords
// so all notes inside every chord will have equal lengths

void splitUnequalChords(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            std::vector<std::pair<ReducedFraction, MidiChord>> newChordEvents;
            auto &chords = track.second.chords;
            sortNotesByLength(chords);
            for (auto &chordEvent: chords) {
                  auto &chord = chordEvent.second;
                  auto &notes = chord.notes;
                  ReducedFraction offTime;
                  for (auto it = notes.begin(); it != notes.end(); ) {
                        if (it == notes.begin())
                              offTime = it->offTime;
                        else {
                              ReducedFraction newOffTime = it->offTime;
                              if (newOffTime != offTime) {
                                    MidiChord newChord(chord);
                                    newChord.notes.clear();
                                    for (int j = it - notes.begin(); j > 0; --j)
                                          newChord.notes.push_back(notes[j - 1]);
                                    newChordEvents.push_back({chordEvent.first, newChord});
                                    it = notes.erase(notes.begin(), it);
                                    continue;
                                    }
                              }
                        ++it;
                        }
                  }
            for (const auto &event: newChordEvents)
                  chords.insert(event);
            }
      }

ReducedFraction findMinDuration(const ReducedFraction &onTime,
                                const QList<MidiChord> &midiChords,
                                const ReducedFraction &length)
      {
      ReducedFraction len = length;
      for (const auto &chord: midiChords) {
            for (const auto &note: chord.notes) {
                  if ((note.offTime - onTime < len)
                              && (note.offTime - onTime != ReducedFraction(0, 1)))
                        len = note.offTime - onTime;
                  }
            }
      return len;
      }

void mergeChordsWithEqualOnTimeAndVoice(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.second.chords;
                        // the key is pair<onTime, voice>
            std::map<std::pair<ReducedFraction, int>,
                     std::multimap<ReducedFraction, MidiChord>::iterator> onTimes;

            for (auto it = chords.begin(); it != chords.end(); ) {
                  const auto &onTime = it->first;
                  const int voice = it->second.voice;
                  auto fit = onTimes.find({onTime, voice});
                  if (fit == onTimes.end()) {
                        onTimes.insert({{onTime, voice}, it});
                        }
                  else {
                        auto &oldNotes = fit->second->second.notes;
                        auto &newNotes = it->second.notes;
                        oldNotes.append(newNotes);
                        it = chords.erase(it);
                        continue;
                        }
                  ++it;
                  }
            }
      }

int chordAveragePitch(const QList<MidiNote> &notes, int beg, int end)
      {
      Q_ASSERT_X(!notes.isEmpty(), "MChord::chordAveragePitch", "Empty notes");
      Q_ASSERT_X(end > 0 && beg >= 0 && end > beg,
                 "MChord::chordAveragePitch", "Invalid note indexes");

      int sum = 0;
      for (int i = beg; i != end; ++i)
            sum += notes[i].pitch;
      return qRound(sum * 1.0 / (end - beg));
      }

int chordAveragePitch(const QList<MidiNote> &notes)
      {
      Q_ASSERT_X(!notes.isEmpty(), "MChord::chordAveragePitch", "Empty notes");

      return chordAveragePitch(notes, 0, notes.size());
      }

// it's an optimization function: we can don't check chords
// with (on time + max chord len) < given time moment
// because chord cannot be longer than found max length

ReducedFraction findMaxChordLength(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      ReducedFraction maxChordLength;

      for (const auto &chord: chords) {
            const auto offTime = maxNoteOffTime(chord.second.notes);
            if (offTime - chord.first > maxChordLength)
                  maxChordLength = offTime - chord.first;
            }
      return maxChordLength;
      }

std::vector<std::multimap<ReducedFraction, MidiChord>::const_iterator>
findChordsForTimeRange(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &offTime,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &maxChordLength)
      {
      std::vector<std::multimap<ReducedFraction, MidiChord>::const_iterator> result;

      if (chords.empty())
            return result;

      auto it = chords.lower_bound(offTime);
      if (it == chords.begin())
            return result;
      --it;

      while (it->first + maxChordLength > onTime) {
            const MidiChord &chord = it->second;
            if (chord.voice == voice) {
                  const auto chordInterval = std::make_pair(it->first, maxNoteOffTime(chord.notes));
                  const auto durationInterval = std::make_pair(onTime, offTime);

                  if (MidiTuplet::haveIntersection(chordInterval, durationInterval))
                        result.push_back(it);
                  }
            if (it == chords.begin())
                  break;
            --it;
            }

      return result;
      }

void setBarIndexes(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant,
            const ReducedFraction &lastTick,
            const TimeSigMap *sigmap)
      {
      if (chords.empty())
            return;
      auto it = chords.begin();
      for (int barIndex = 0;; ++barIndex) {       // iterate over all measures by indexes
            const auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(barIndex + 1, 0));
            if (endBarTick <= it->first)
                  continue;
            for (; it != chords.end(); ++it) {
                  const auto onTime = Quantize::findQuantizedChordOnTime(*it, basicQuant);
#ifdef QT_DEBUG
                  const auto barStart = ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
                  Q_ASSERT_X(!(it->first >= barStart && onTime < barStart),
                             "MChord::setBarIndexes", "quantized on time cannot be in previous bar");
#endif
                  if (onTime < endBarTick) {
                        it->second.barIndex = barIndex;
                        continue;
                        }
                  break;
                  }
            if (it == chords.end() || endBarTick > lastTick)
                  break;
            }

      Q_ASSERT_X(areBarIndexesSet(chords),
                 "MChord::setBarIndexes", "Not all bar indexes were set");
      Q_ASSERT_X(areBarIndexesSuccessive(chords),
                 "MChord::setBarIndexes", "Bar indexes are not successive");
      }

} // namespace MChord
} // namespace Ms
