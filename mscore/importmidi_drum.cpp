#include "importmidi_drum.h"
#include "importmidi_inner.h"
#include "preferences.h"
#include "libmscore/staff.h"
#include "libmscore/drumset.h"
#include "importmidi_chord.h"
#include "importmidi_tuplet.h"
#include "importmidi_operations.h"
#include "importmidi_voice.h"
#include "libmscore/score.h"
#include "midi/midifile.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace MidiDrum {


#ifdef QT_DEBUG

bool haveNonZeroVoices(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (const auto &chordEvent: chords) {
            if (chordEvent.second.voice != 0)
                  return false;
            }
      return true;
      }

#endif


void splitDrumVoices(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
            auto &chords = mtrack.chords;
            const Drumset* const drumset = mtrack.mtrack->drumTrack() ? smDrumset : 0;
            if (!drumset)
                  continue;
            bool changed = false;
                              // all chords of drum track should have voice == 0
                              // because allowedVoices == V_1 (see MidiImportOperations)
                              // also, all chords should have different onTime values
            Q_ASSERT_X(MChord::areOnTimeValuesDifferent(chords),
                       "MidiDrum::splitDrumVoices",
                       "onTime values of chords are equal but should be different");
            Q_ASSERT_X(haveNonZeroVoices(chords),
                       "MidiDrum::splitDrumVoices",
                       "All voices of drum track should be zero here");

            std::multimap<ReducedFraction,
                 std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator> insertedTuplets;
            const ReducedFraction maxChordLength = MChord::findMaxChordLength(chords);

            for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
                  const auto &notes = chordIt->second.notes;
                              // search for the drumset pitches with voice = 1
                  QSet<int> notesToMove;
                  for (int i = 0; i != notes.size(); ++i) {
                        if (drumset->isValid(notes[i].pitch)
                                    && drumset->voice(notes[i].pitch) == 1) {
                              notesToMove.insert(i);
                              }
                        }
                  if (MidiVoice::splitChordToVoice(chordIt, notesToMove, 1,
                                                   chords, mtrack.tuplets,
                                                   insertedTuplets, maxChordLength, true)) {
                        changed = true;
                        }
                  }

            if (changed)
                  MidiTuplet::removeEmptyTuplets(mtrack);

            Q_ASSERT_X(MidiTuplet::areAllTupletsReferenced(mtrack.chords, mtrack.tuplets),
                       "MidiDrum::splitDrumVoices",
                       "Not all tuplets are referenced in chords or notes after drum split");
            Q_ASSERT_X(MidiVoice::areVoicesSame(mtrack.chords),
                       "MidiDrum::splitDrumVoices", "Different voices of chord and tuplet "
                       "after drum split");
            }
      }

MTrack& getNewTrack(std::map<int, MTrack> &newTracks,
                    const MTrack &drumTrack,
                    int pitch)
      {
      auto newTrackIt = newTracks.find(pitch);
      if (newTrackIt == newTracks.end()) {
            newTrackIt = newTracks.insert({pitch, drumTrack}).first;
            MTrack &newTrack = newTrackIt->second;
                        // chords and tuplets are copied and then cleared -
                        // not very efficient way but it's more safe for possible
                        // future additions of new fields in MTrack
            newTrack.chords.clear();
            newTrack.tuplets.clear();
            newTrack.name = smDrumset->name(pitch);
            }
      return newTrackIt->second;
      }

std::map<int, MTrack> splitDrumTrack(const MTrack &drumTrack)
      {
      std::map<int, MTrack> newTracks;         // <percussion note pitch, track>
      if (drumTrack.chords.empty())
            return newTracks;

      for (const auto &chordEvent: drumTrack.chords) {
            const auto &onTime = chordEvent.first;
            const MidiChord &chord = chordEvent.second;

            for (const auto &note: chord.notes) {
                  MidiChord newChord;
                  newChord.voice = chord.voice;
                  newChord.notes.push_back(note);
                  MTrack &newTrack = getNewTrack(newTracks, drumTrack, note.pitch);
                  newTrack.chords.insert({onTime, newChord});

                  const auto tupletIt = MidiTuplet::findTupletContainingTime(
                                    chord.voice, onTime, drumTrack.tuplets);
                  if (tupletIt != drumTrack.tuplets.end()) {
                        const auto newTupletIt = MidiTuplet::findTupletContainingTime(
                                          newChord.voice, onTime, newTrack.tuplets);
                        if (newTupletIt == newTrack.tuplets.end()) {
                              MidiTuplet::TupletData newTupletData = tupletIt->second;
                              newTupletData.voice = newChord.voice;
                              newTrack.tuplets.insert({tupletIt->first, newTupletData});
                              }
                        }
                  }
            }

      for (auto &track: newTracks)
            MidiTuplet::removeEmptyTuplets(track.second);

      return newTracks;
      }

void splitDrumTracks(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (!it->second.mtrack->drumTrack())
                  continue;
            const auto &opers = preferences.midiImportOperations.data()->trackOpers;
            if (!opers.doStaffSplit.value(it->second.indexOfOperation))
                  continue;
            const auto newTracks = splitDrumTrack(it->second);
            const int trackIndex = it->first;
            it = tracks.erase(it);
            for (auto i = newTracks.rbegin(); i != newTracks.rend(); ++i)
                  it = tracks.insert({trackIndex, i->second});
            }
      }

void setBracket(Staff *&staff, int &counter)
      {
      if (staff && counter > 1) {
            staff->setBracket(0, BracketType::NORMAL);
            staff->setBracketSpan(0, counter);
            }
      if (counter)
            counter = 0;
      if (staff)
            staff = nullptr;
      }

void setStaffBracketForDrums(QList<MTrack> &tracks)
      {
      int counter = 0;
      Staff *firstDrumStaff = nullptr;
      int opIndex = -1;

      for (const MTrack &track: tracks) {
            if (track.mtrack->drumTrack()) {
                  if (opIndex != track.indexOfOperation) {
                        opIndex = track.indexOfOperation;
                        setBracket(firstDrumStaff, counter);
                        firstDrumStaff = track.staff;
                        }
                  ++counter;
                  }
            else
                  setBracket(firstDrumStaff, counter);
            }
      setBracket(firstDrumStaff, counter);
      }

ReducedFraction endOfBarForTick(const ReducedFraction &tick, const TimeSigMap *sigmap)
      {
      int bar, beat, tickInBar;
      sigmap->tickValues(tick.ticks(), &bar, &beat, &tickInBar);
      return ReducedFraction::fromTicks(sigmap->bar2tick(bar + 1, 0));
      }

ReducedFraction findOptimalNoteOffTime(
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap)
      {
      const auto &onTime = chordIt->first;
      auto barEnd = endOfBarForTick(onTime, sigmap);
                  // let's find new offTime = min(next chord onTime, barEnd)
      auto offTime = barEnd;
      auto next = std::next(chordIt);
      while (next != chords.end()) {
            if (next->first > barEnd)
                  break;
            if (next->second.voice == chordIt->second.voice) {
                  offTime = next->first;
                  break;
                  }
            ++next;
            }

      return offTime;
      }

void removeRests(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap)
      {
      const auto &opers = preferences.midiImportOperations.data()->trackOpers;

      for (auto &trackItem: tracks) {
            MTrack &track = trackItem.second;
            if (!track.mtrack->drumTrack())
                  continue;
            if (!opers.simplifyDurations.value(track.indexOfOperation))
                  continue;
            bool changed = false;
                        // all chords here with the same voice should have different onTime values
            for (auto it = track.chords.begin(); it != track.chords.end(); ++it) {
                  const auto newOffTime = findOptimalNoteOffTime(it, track.chords, sigmap);
                  for (auto &note: it->second.notes) {
                        if (note.offTime != newOffTime) {
                              note.offTime = newOffTime;
                              if (!changed)
                                    changed = true;
                              }
                        }
                  }
            if (changed)
                  MidiTuplet::removeEmptyTuplets(track);
            }
      }

} // namespace MidiDrum
} // namespace Ms
