#include "importmidi_drum.h"
#include "importmidi_inner.h"
#include "mscore/preferences.h"
#include "libmscore/staff.h"
#include "libmscore/drumset.h"
#include "importmidi_chord.h"
#include "importmidi_tuplet.h"
#include "importmidi_operations.h"
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
            if (chords.empty())
                  continue;
            const Drumset* const drumset = mtrack.mtrack->drumTrack() ? smDrumset : 0;
            if (!drumset)
                  continue;
                              // all chords of drum track should have voice == 0
                              // because allowedVoices == V_1 (see MidiImportOperations)
                              // also, all chords should have different onTime values
            Q_ASSERT_X(MChord::areOnTimeValuesDifferent(chords),
                       "MidiDrum::splitDrumVoices",
                       "onTime values of chords are equal but should be different");
            Q_ASSERT_X(haveNonZeroVoices(chords),
                       "MidiDrum::splitDrumVoices",
                       "All voices of drum track should be zero here");

            for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
                  const ReducedFraction onTime = chordIt->first;
                  MidiChord &chord = chordIt->second;
                  auto &notes = chord.notes;
                              // search for the drumset pitches with voice = 1
                  QSet<int> notesToMove;
                  for (int i = 0; i != notes.size(); ++i) {
                        const int pitch = notes[i].pitch;
                        if (drumset->isValid(pitch) && drumset->voice(pitch) == 1)
                              notesToMove.insert(i);
                        }
                  if (notesToMove.isEmpty())
                        continue;

                  if (notesToMove.size() == notes.size()) {  // just move chord to another voice
                        chord.voice = 1;
                        }
                  else {            // split chord
                        MidiChord newChord(chord);
                        newChord.notes.clear();
                        newChord.voice = 1;
                        QList<MidiNote> updatedOldNotes;

                        for (int i = 0; i != notes.size(); ++i) {
                              if (notesToMove.contains(i))
                                    newChord.notes.append(notes[i]);
                              else
                                    updatedOldNotes.append(notes[i]);
                              }

                        notes = updatedOldNotes;

                        Q_ASSERT_X(!notes.isEmpty(),
                                   "MidiDrum::splitDrumVoices", "Old chord notes are empty");
                        Q_ASSERT_X(!newChord.notes.isEmpty(),
                                   "MidiDrum::splitDrumVoices", "New chord notes are empty");

                        chordIt = chords.insert({onTime, newChord});
                        }
                  }
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
      Q_ASSERT(MidiTuplet::areAllTupletsDifferent(drumTrack.tuplets));

      std::map<int, MTrack> newTracks;         // <percussion note pitch, track>
      if (drumTrack.chords.empty())
            return newTracks;

      for (const auto &chordEvent: drumTrack.chords) {
            const auto &onTime = chordEvent.first;
            const MidiChord &chord = chordEvent.second;

            for (const auto &note: chord.notes) {
                  MidiChord newChord(chord);
                  newChord.notes.clear();
                  newChord.notes.push_back(note);
                  MTrack &newTrack = getNewTrack(newTracks, drumTrack, note.pitch);

                  if (chord.isInTuplet) {
                        auto newTupletIt = MidiTuplet::findTupletContainingTime(
                                                newChord.voice, onTime, newTrack.tuplets, true);
                        if (newTupletIt == newTrack.tuplets.end()) {
                              MidiTuplet::TupletData newTupletData = chord.tuplet->second;
                              newTupletData.voice = newChord.voice;
                              newTupletIt = newTrack.tuplets.insert({chord.tuplet->first,
                                                                     newTupletData});
                              }
                                    // hack to remove constness of iterator
                        newChord.tuplet = newTrack.tuplets.erase(newTupletIt, newTupletIt);
                        }
                  if (note.isInTuplet) {
                        auto newTupletIt = MidiTuplet::findTupletContainingTime(
                                          newChord.voice, note.offTime, newTrack.tuplets, false);
                        if (newTupletIt == newTrack.tuplets.end()) {
                              MidiTuplet::TupletData newTupletData = note.tuplet->second;
                              newTupletData.voice = newChord.voice;
                              newTupletIt = newTrack.tuplets.insert({note.tuplet->first,
                                                                     newTupletData});
                              }
                                    // hack to remove constness of iterator
                        newChord.notes.front().tuplet = newTrack.tuplets.erase(
                                                            newTupletIt, newTupletIt);
                        }
                  newTrack.chords.insert({onTime, newChord});
                  }
            }

      for (auto &track: newTracks)
            {
            Q_ASSERT(MidiTuplet::areAllTupletsDifferent(track.second.tuplets));

            MidiTuplet::removeEmptyTuplets(track.second);
            }

      return newTracks;
      }

void splitDrumTracks(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (!it->second.mtrack->drumTrack() || it->second.chords.empty())
                  continue;
            const auto &opers = preferences.midiImportOperations.data()->trackOpers;
            if (!opers.doStaffSplit.value(it->second.indexOfOperation))
                  continue;
            const std::map<int, MTrack> newTracks = splitDrumTrack(it->second);
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

} // namespace MidiDrum
} // namespace Ms
