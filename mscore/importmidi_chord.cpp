#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_chord.h"
#include "importmidi_clef.h"
#include "libmscore/mscore.h"


namespace Ms {
namespace MChord {


ReducedFraction minAllowedDuration()
      {
      const static auto minDuration = ReducedFraction::fromTicks(MScore::division) / 32;
      return minDuration;
      }

ReducedFraction maxNoteLen(const QList<MidiNote> &notes)
      {
      ReducedFraction maxLen(0, 1);
      for (const auto &note: notes) {
            if (note.len > maxLen)
                  maxLen = note.len;
            }
      return maxLen;
      }

int findAveragePitch(const QList<MidiNote> &notes)
      {
      int avgPitch = 0;
      for (const auto &note: notes)
            avgPitch += note.pitch;
      return notes.size() ? avgPitch / notes.size() : 0;
      }

int findAveragePitch(const std::map<ReducedFraction, MidiChord>::const_iterator &startChordIt,
                     const std::map<ReducedFraction, MidiChord>::const_iterator &endChordIt)
      {
      int avgPitch = 0;
      int counter = 0;
      for (auto it = startChordIt; it != endChordIt; ++it) {
            avgPitch += findAveragePitch(it->second.notes);
            ++counter;
            }
      if (counter)
            avgPitch /= counter;
      if (avgPitch == 0)
            avgPitch = MidiClef::midPitch();
      return avgPitch;
      }

// remove overlapping notes with the same pitch

void removeOverlappingNotes(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.second.chords;
            for (auto it = chords.begin(); it != chords.end(); ++it) {
                  auto &firstChord = it->second;
                  const auto &firstOnTime = it->first;
                  for (auto &note1: firstChord.notes) {
                        auto ii = it;
                        ++ii;
                        for (; ii != chords.end(); ++ii) {
                              auto &secondChord = ii->second;
                              const auto &secondOnTime = ii->first;
                              for (auto &note2: secondChord.notes) {
                                    if (note2.pitch != note1.pitch)
                                          continue;
                                    if (secondOnTime >= (firstOnTime + note1.len))
                                          continue;
                                    qDebug("Midi import: overlapping events: %d+%d %d+%d",
                                           firstOnTime.ticks(), note1.len.ticks(),
                                           secondOnTime.ticks(), note2.len.ticks());
                                    note1.len = secondOnTime - firstOnTime;
                                    ii = chords.end();
                                    --ii;
                                    break;
                                    }
                              }
                        if (note1.len <= ReducedFraction(0, 1)) {
                              qDebug("Midi import: duration <= 0: drop note at %d",
                                     firstOnTime.ticks());
                              continue;
                              }
                        }
                  } // for note1
            }
      }


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

            ReducedFraction threshTime = minAllowedDuration() / 2;
            ReducedFraction fudgeTime = threshTime / 4;
            ReducedFraction threshExtTime = threshTime / 2;

            ReducedFraction startTime(-1, 1);    // invalid
            ReducedFraction curThreshTime(-1, 1);
                        // if intersection of note durations is less than min(minNoteDuration, threshTime)
                        // then this is not a chord
            ReducedFraction tol(-1, 1);       // invalid
            ReducedFraction beg(-1, 1);
            ReducedFraction end(-1, 1);
                        // chords here consist of a single note
                        // because notes are not united into chords yet
            for (auto it = chords.begin(); it != chords.end(); ) {
                  const auto &note = it->second.notes[0];
                              // this should not be executed when it == chords.begin()
                  if (it->first <= startTime + curThreshTime) {
                        if (it->first > beg)
                              beg = it->first;
                        if (it->first + note.len < end)
                              end = it->first + note.len;
                        if (note.len < tol)
                              tol = note.len;
                        if (end - beg >= tol) {
                              // add current note to the previous chord
                              auto prev = it;
                              --prev;
                              prev->second.notes.push_back(note);
                              if (it->first >= startTime + curThreshTime - fudgeTime)
                                    curThreshTime += threshExtTime;
                              it = chords.erase(it);
                              continue;
                              }
                        }
                  else {
                        startTime = it->first;
                        beg = startTime;
                        end = startTime + note.len;
                        tol = threshTime;
                        if (curThreshTime != threshTime)
                              curThreshTime = threshTime;
                        }
                  ++it;
                  }
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
                  return note1.len < note2.len;
                  }
            } lenSort;

      for (auto &chordEvent: chords) {
                        // in each chord sort notes by pitches
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
                  ReducedFraction len;
                  for (auto it = notes.begin(); it != notes.end(); ) {
                        if (it == notes.begin())
                              len = it->len;
                        else {
                              ReducedFraction newLen = it->len;
                              if (newLen != len) {
                                    MidiChord newChord;
                                    newChord.voice = chord.voice;
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

ReducedFraction findMinDuration(const QList<MidiChord> &midiChords,
                                const ReducedFraction &length)
      {
      ReducedFraction len = length;
      for (const auto &chord: midiChords) {
            for (const auto &note: chord.notes) {
                  if ((note.len < len) && (note.len != ReducedFraction(0, 1)))
                        len = note.len;
                  }
            }
      return len;
      }

} // namespace MChord
} // namespace Ms
