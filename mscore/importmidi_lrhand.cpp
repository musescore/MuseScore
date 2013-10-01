#include "importmidi_lrhand.h"
#include "importmidi_tuplet.h"
#include "importmidi_inner.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"
#include "preferences.h"


namespace Ms {

extern Preferences preferences;

namespace LRHand {

void insertNewLeftHandTrack(std::multimap<int, MTrack> &tracks,
                            std::multimap<int, MTrack>::iterator &it,
                            const std::multimap<ReducedFraction, MidiChord> &leftHandChords)
      {
      auto leftHandTrack = it->second;
      leftHandTrack.chords = leftHandChords;
      MidiTuplet::removeEmptyTuplets(leftHandTrack);
      it = tracks.insert({it->first, leftHandTrack});
      }

void addNewLeftHandChord(std::multimap<ReducedFraction, MidiChord> &leftHandChords,
                         const QList<MidiNote> &leftHandNotes,
                         const std::multimap<ReducedFraction, MidiChord>::iterator &it)
      {
      MidiChord leftHandChord = it->second;
      leftHandChord.notes = leftHandNotes;
      leftHandChords.insert({it->first, leftHandChord});
      }

void splitByFixedPitch(std::multimap<int, MTrack> &tracks,
                       std::multimap<int, MTrack>::iterator &it)
      {
      auto &srcTrack = it->second;
      const auto trackOpers = preferences.midiImportOperations.trackOperations(srcTrack.indexOfOperation);
      const int splitPitch = 12 * (int)trackOpers.LHRH.splitPitchOctave
                           + (int)trackOpers.LHRH.splitPitchNote;
      std::multimap<ReducedFraction, MidiChord> leftHandChords;

      for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end(); ) {
            auto &notes = i->second.notes;
            QList<MidiNote> leftHandNotes;
            for (auto j = notes.begin(); j != notes.end(); ) {
                  auto &note = *j;
                  if (note.pitch < splitPitch) {
                        leftHandNotes.push_back(note);
                        j = notes.erase(j);
                        continue;
                        }
                  ++j;
                  }
            if (!leftHandNotes.empty())
                  addNewLeftHandChord(leftHandChords, leftHandNotes, i);
            if (notes.isEmpty()) {
                  i = srcTrack.chords.erase(i);
                  continue;
                  }
            ++i;
            }
      if (!leftHandChords.empty())
            insertNewLeftHandTrack(tracks, it, leftHandChords);
      }

void splitByHandWidth(std::multimap<int, MTrack> &tracks,
                      std::multimap<int, MTrack>::iterator &it)
      {
      auto &srcTrack = it->second;
      MChord::sortNotesByPitch(srcTrack.chords);
      const int octave = 12;
      std::multimap<ReducedFraction, MidiChord> leftHandChords;
                  // chords after MIDI import are sorted by onTime values
      for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end(); ) {
            auto &notes = i->second.notes;
            QList<MidiNote> leftHandNotes;
            const int minPitch = notes.front().pitch;
            const int maxPitch = notes.back().pitch;
            if (maxPitch - minPitch > octave) {
                              // need both hands
                              // assign all chords in range [minPitch .. minPitch + OCTAVE]
                              // to left hand and all other chords - to right hand
                  for (auto j = notes.begin(); j != notes.end(); ) {
                        const auto &note = *j;
                        if (note.pitch <= minPitch + octave) {
                              leftHandNotes.push_back(note);
                              j = notes.erase(j);
                              continue;
                              }
                        ++j;
                        // maybe todo later: if range of right-hand chords > OCTAVE
                        // => assign all bottom right-hand chords to another, third track
                        }
                  }
            else {            // check - use two hands or one hand will be enough (right or left?)
                              // assign top chord for right hand, all the rest - to left hand
                  while (notes.size() > 1) {
                        leftHandNotes.push_back(notes.front());
                        notes.erase(notes.begin());
                        }
                  }
            if (!leftHandNotes.empty())
                  addNewLeftHandChord(leftHandChords, leftHandNotes, i);
            if (notes.isEmpty()) {
                  i = srcTrack.chords.erase(i);
                  continue;
                  }
            ++i;
            }
      if (!leftHandChords.empty())
            insertNewLeftHandTrack(tracks, it, leftHandChords);
      }

void splitIntoLeftRightHands(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (it->second.mtrack->drumTrack())
                  continue;
            const auto operations = preferences.midiImportOperations.trackOperations(
                                                              it->second.indexOfOperation);
            if (!operations.LHRH.doIt)
                  continue;
                        // iterator 'it' will change after track split to ++it
                        // C++11 guarantees that newely inserted item with equal key will go after:
                        //    "The relative ordering of elements with equivalent keys is preserved,
                        //     and newly inserted elements follow those with equivalent keys
                        //     already in the container"
            switch (operations.LHRH.method) {
                  case MidiOperation::LHRHMethod::HAND_WIDTH:
                        splitByHandWidth(tracks, it);
                        break;
                  case MidiOperation::LHRHMethod::SPECIFIED_PITCH:
                        splitByFixedPitch(tracks, it);
                        break;
                  }
            }
      }

} // namespace LRHand
} // namespace Ms
