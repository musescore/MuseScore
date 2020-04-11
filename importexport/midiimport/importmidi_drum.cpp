#include "importmidi_drum.h"
#include "importmidi_inner.h"
#include "mscore/preferences.h"
#include "libmscore/staff.h"
#include "libmscore/drumset.h"
#include "importmidi_chord.h"
#include "importmidi_operations.h"
#include "libmscore/score.h"
#include "midi/midifile.h"

#include <set>


namespace Ms {

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


void splitChord(
            std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const QSet<int> &notesToMove,
            std::multimap<ReducedFraction, MidiChord> &chords)
      {
      MidiChord &chord = chordIt->second;
      auto &notes = chord.notes;

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
                       "MidiDrum::splitChord", "Old chord notes are empty");
            Q_ASSERT_X(!newChord.notes.isEmpty(),
                       "MidiDrum::splitChord", "New chord notes are empty");

            chordIt = chords.insert({chordIt->first, newChord});
            }
      }

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
                  auto &notes = chordIt->second.notes;
                              // search for the drumset pitches with voice = 1
                  QSet<int> notesToMove;
                  for (int i = 0; i != notes.size(); ++i) {
                        const int pitch = notes[i].pitch;
                        if (drumset->isValid(pitch) && drumset->voice(pitch) == 1)
                              notesToMove.insert(i);
                        }
                  if (notesToMove.isEmpty())
                        continue;

                  splitChord(chordIt, notesToMove, chords);
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
                        // chords are copied and then cleared -
                        // not very efficient way but it's more safe for possible
                        // future additions of new fields in MTrack
            newTrack.chords.clear();
            newTrack.name = smDrumset->name(pitch);

            Q_ASSERT(newTrack.tuplets.empty());
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
                  MidiChord newChord(chord);
                  newChord.notes.clear();
                  newChord.notes.push_back(note);
                  MTrack &newTrack = getNewTrack(newTracks, drumTrack, note.pitch);
                  newTrack.chords.insert({onTime, newChord});
                  }
            }

      return newTracks;
      }

void splitDrumTracks(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (!it->second.mtrack->drumTrack() || it->second.chords.empty())
                  continue;
            const auto &opers = midiImportOperations.data()->trackOpers;
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
            staff->setBracketType(0, BracketType::NORMAL);
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
