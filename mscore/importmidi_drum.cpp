#include "importmidi_drum.h"
#include "importmidi_inner.h"
#include "preferences.h"
#include "libmscore/staff.h"
#include "libmscore/drumset.h"
#include "importmidi_chord.h"
#include "importmidi_tuplet.h"


namespace Ms {

extern Preferences preferences;

namespace MidiDrum {

void splitDrumVoices(std::multimap<int, MTrack> &tracks)
      {
      for (auto &trackItem: tracks) {
            MTrack &track = trackItem.second;
            std::vector<std::pair<ReducedFraction, MidiChord>> newChordEvents;
            auto &chords = track.chords;
            const Drumset* const drumset = track.mtrack->drumTrack() ? smDrumset : 0;
            if (!drumset)
                  continue;
                              // all chords of drum track have voice == 0
                              // because useMultipleVoices == false (see MidiImportOperations)
            for (auto chordIt = chords.begin(); chordIt != chords.end(); ) {
                  auto &chord = chordIt->second;
                  auto &notes = chord.notes;
                  MidiChord newChord;
                  for (auto it = notes.begin(); it != notes.end(); ) {
                        if (drumset->isValid(it->pitch) && drumset->voice(it->pitch) != 0) {
                              newChord.voice = drumset->voice(it->pitch);
                              newChord.notes.push_back(*it);

                              it = notes.erase(it);
                              continue;
                              }
                        ++it;
                        }
                  if (!newChord.notes.isEmpty()) {
                        newChordEvents.push_back({chordIt->first, newChord});

                        const auto tupletIt = MidiTuplet::findTupletContainsTime(
                                          chordIt->second.voice, chordIt->first, track.tuplets);
                        const auto newTupletIt = MidiTuplet::findTupletContainsTime(
                                          newChord.voice, chordIt->first, track.tuplets);
                        if (tupletIt != track.tuplets.end()
                                    && newTupletIt == track.tuplets.end()) {
                              MidiTuplet::TupletData newTupletData = tupletIt->second;
                              newTupletData.voice = newChord.voice;
                              track.tuplets.insert({tupletIt->first, newTupletData});
                              }
                        if (notes.isEmpty()) {
                              MidiTuplet::removeEmptyTuplets(track);

                              chordIt = chords.erase(chordIt);
                              continue;
                              }
                        }
                  ++chordIt;
                  }
            for (const auto &event: newChordEvents)
                  chords.insert(event);
            }
      }

std::map<int, MTrack> splitDrumTrack(MTrack &drumTrack)
      {
      std::map<int, MTrack> newTracks;         // <percussion note pitch, track>
      if (drumTrack.chords.empty())
            return newTracks;

      while (!drumTrack.chords.empty()) {
            int pitch = -1;
            MTrack *curTrack = nullptr;
            for (auto it = drumTrack.chords.begin(); it != drumTrack.chords.end(); ) {
                  MidiChord &chord = it->second;
                  for (auto noteIt = chord.notes.begin(); noteIt != chord.notes.end(); ) {
                        if (pitch == -1) {
                              pitch = noteIt->pitch;
                              MTrack newTrack = drumTrack;
                              newTrack.chords.clear();
                              newTrack.tuplets.clear();
                              newTrack.name = smDrumset->name(pitch);
                              newTracks.insert({pitch, newTrack});
                              curTrack = &newTracks.find(pitch)->second;
                              }
                        if (noteIt->pitch == pitch) {
                              MidiChord newChord;
                              newChord.voice = chord.voice;
                              newChord.notes.push_back(*noteIt);
                              curTrack->chords.insert({it->first, newChord});

                              const auto tupletIt = MidiTuplet::findTupletContainsTime(
                                                chord.voice, it->first, drumTrack.tuplets);
                              if (tupletIt != drumTrack.tuplets.end()) {
                                    auto newTupletIt = MidiTuplet::findTupletContainsTime(
                                                      newChord.voice, it->first, curTrack->tuplets);
                                    if (newTupletIt == curTrack->tuplets.end()) {
                                          MidiTuplet::TupletData newTupletData = tupletIt->second;
                                          newTupletData.voice = newChord.voice;
                                          curTrack->tuplets.insert({tupletIt->first, newTupletData});
                                          }
                                    }
                              noteIt = chord.notes.erase(noteIt);
                              continue;
                              }
                        ++noteIt;
                        }
                  if (chord.notes.isEmpty()) {
                        it = drumTrack.chords.erase(it);
                        continue;
                        }
                  ++it;
                  }
            }

      return newTracks;
      }

void splitDrumTracks(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (!it->second.mtrack->drumTrack())
                  continue;
            const auto operations = preferences.midiImportOperations.trackOperations(
                                                            it->second.indexOfOperation);
            if (!operations.drums.doSplit)
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
            staff->setBracket(0, BRACKET_NORMAL);
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
      const auto &opers = preferences.midiImportOperations;

      for (const MTrack &track: tracks) {
            if (track.mtrack->drumTrack()) {
                  if (opIndex != track.indexOfOperation) {
                        setBracket(firstDrumStaff, counter);
                        if (opers.trackOperations(track.indexOfOperation).drums.showStaffBracket) {
                              opIndex = track.indexOfOperation;
                              firstDrumStaff = track.staff;
                              }
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
