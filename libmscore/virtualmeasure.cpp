//=============================================================================
//  virtualmeasure.cpp
//
//  Copyright (C) 2016 Peter Jonas <shoogle@users.noreply.github.com>
//
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License version 2 as published
//  by the Free Software Foundation and appearing in the file LICENCE.GPL
//=============================================================================

#include <algorithm>    // std::sort

#include "virtualmeasure.h"
#include "durationtype.h"

namespace Ms {

//---------------------------------------------------------
//   arrange
//    Re-arrange the passage of music in the Virtual Measure.
//---------------------------------------------------------

void VirtualMeasure::arrange()
      {
      // Notes were already added to lowest available voice, so the arangement is already optimum.
      // Just need to sort the voices to ensure that voices 1 and 3 are higher than voices 2 and 4.
      for (VirtualVoice* vv: _voices) {
            vv->sortChordsByTick();
            for (RawChord* rc : vv->chords()) {
                  rc->sortNotesByPitch();
                  }
            }

      sortVoicesByAveragePitch();
      }

//---------------------------------------------------------
//   addTiedNotes
//    Adds a note for each duration in the list and ties it to the others in the list.
//---------------------------------------------------------

void VirtualMeasure::addTiedNotes(int pitch, int tick, const std::vector<TDuration>& dList, Note* tiedNoteFor, Note* tiedNoteBack)
      {
      RawNote* prev = 0;
      int cumulativeTick = tick;
      for (TDuration d : dList) {
            int ticks = d.ticks();
            RawNote* rn = new RawNote(pitch, 0, tiedNoteBack, 0, prev);
            RawChord* rc = new RawChord(cumulativeTick, ticks, rn);
            addChord(rc);
            if (prev)
                  prev->setNext(rn); // update the previous note to point to this one
            prev = rn;
            tiedNoteBack = 0;
            cumulativeTick += ticks;
            }
      prev->setTiedNoteFor(tiedNoteFor); // tie the final note to a note outside the VirtualMeasure
      }

//---------------------------------------------------------
//   addChord
//    Adds a chord to the lowest-numbered voice it can go in.
//---------------------------------------------------------

void VirtualMeasure::addChord(RawChord* chord)
      {
      for (VirtualVoice* v : _voices) {
            if (v->addChord(chord))
                  return; // chord added to existing voice
            }
      // couldn't add chord to existing voice so create a new one
      VirtualVoice* vv = new VirtualVoice(chord);
      _voices.push_back(vv);
      }

bool VirtualVoice::addChord(RawChord* chord) {
      for (RawChord* rc : _chords) {
            // try to combine chord with an existing chord in the voice
            if (rc->addNotesFromChord(chord))
                  return true; // chords coincide (same start and end tick) so were combined
            if (rc->overlaps(chord))
                  return false; // chords overlap so can't go in this voice
            }
      // Chord neither coincides nor overlaps with any chords in voice, so it cannot be combined
      // with an exiting chord, but it can still be added to the voice as a separate chord.
      _chords.push_back(chord);
      return true;
      }

//---------------------------------------------------------
//   addNotesFromChord
//---------------------------------------------------------

bool RawChord::addNotesFromChord(RawChord* chord)
      {
      if (!coincides(chord))
            return false;
      for (RawNote* rn : chord->notes())
            _notes.push_back(rn);
      return true;
      }

//---------------------------------------------------------
//   sortVoicesByAveragePitch
//---------------------------------------------------------

void VirtualMeasure::sortVoicesByAveragePitch()
      {
      qDebug("Sort Voices");
      int n = numVoices();
      if (n < 2)
            return;

      // sort voices by average pitch, highest first
      std::sort(_voices.begin(), _voices.end(),
            [] (VirtualVoice* v1, VirtualVoice* v2) { return v1->averagePitch() > v2->averagePitch(); });

      // In MuseScore odd voices are higher than even voices (Voice 1 > Voice 3 > Voice 4 > Voice 2) so now
      // sort into order: highest, lowest, 2nd highest, 2nd lowest, etc (can have more than 4 virtual voices).
      for (int i = 1; i < n; i += 2) {
            // move the final element (the lowest pitch
            // note not yet moved) up to the i-th place
            VirtualVoice* v = _voices[n-1];
            _voices.pop_back();
            _voices.insert(_voices.begin()+i, v);
            }
      }

//---------------------------------------------------------
//   sortChordsByTick
//---------------------------------------------------------

void VirtualVoice::sortChordsByTick()
      {
      // sort chords by tick, lowest first
      std::sort(_chords.begin(), _chords.end(),
            [] (RawChord* rc1, RawChord* rc2) { return rc1->tick() < rc2->tick(); });
      }

//---------------------------------------------------------
//   sortNotesByPitch
//---------------------------------------------------------

void RawChord::sortNotesByPitch()
      {
      // sort notes by pitch, highest first
      std::sort(_notes.begin(), _notes.end(),
            [] (RawNote* rn1, RawNote* rn2) { return rn1->pitch() > rn2->pitch(); });
      }

//---------------------------------------------------------
//   numNotes
//---------------------------------------------------------

int VirtualVoice::numNotes() const
      {
      int sum = 0;
      for (RawChord* rc : _chords)
            sum += rc->numNotes();
      return sum;
      }

//---------------------------------------------------------
//   sumPitches
//---------------------------------------------------------

int VirtualVoice::sumPitches() const
      {
      int sum = 0;
      for (RawChord* rc : _chords)
            sum += rc->sumPitches();
      return sum;
      }

int RawChord::sumPitches() const
      {
      int sum = 0;
      for (RawNote* rn : _notes)
            sum += rn->pitch();
      return sum;
      }

#if 0
//---------------------------------------------------------
//   print
//---------------------------------------------------------

void VirtualMeasure::print()
      {
      int i = 0;
      for (VirtualVoice* v : _voices) {
            qDebug("~VirtualVoice %i", i);
            v->print();
            i++;
            }
      }

void VirtualVoice::print()
      {
      int i = 0;
      for (RawChord* rc : _chords) {
            qDebug("~~VirtualVoiceChord %i", i);
            rc->print();
            i++;
            }
      }

void RawChord::print()
      {
      qDebug("~~~RawChord: tick = %i, ticks = %i (endTick = %i)", tick(), ticks(), endTick());
      for (RawNote* rn : _notes)
            rn->print();
      }
#endif

} // namespace Ms
