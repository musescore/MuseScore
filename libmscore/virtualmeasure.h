//=============================================================================
//  virtualmeasure.h
//
//  Copyright (C) 2016 Peter Jonas <shoogle@users.noreply.github.com>
//
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License version 2 as published
//  by the Free Software Foundation and appearing in the file LICENCE.GPL
//=============================================================================

#ifndef VIRTUALMEASURE_H
#define VIRTUALMEASURE_H

#include <vector>

namespace Ms {

//---------------------------------------------------------
//   VirtualMeasure
//   ==============
//    The VirtualMeasure provides an abstraction layer for re-arranging a section
//   of music. It provides a place to store and operate on musical notes as simple
//   data structures without the restrictions that normally apply. For example, the
//   number of voices in the virtual measure is unlimited. This is helpful if you
//   want to move notes between voices without overwriting notes already in other
//   voice, since any overlapping notes can just be sent to a new voice. It is
//   especially helpful when you do not know what the final arrangement is going to
//   look like in advance. Once the re-arrangement is complete the RawNotes can be
//   read from the VirtualMeasure and converted to "real" Notes in a real Score.
//
//   Usage:
//    1) Create a VirtualMeasure object and fill it with notes from a section of a
//      real score. (The section doesn't have to be equal to a Measure, but a
//      Measure is a sensible choice because notes cannot cross a Measure boundary).
//    2) Call the VirtualMeasure's "arrange()" method to optimise the voicing.
//    3) Read the new arrangement from the VirtualMeasure and add notes to the real score.
//
//    The default algorithm simply optimises the voices such that everything gets
//    put in the lowest-numbered voice possible. New voices are only added when
//    necessary due to overlapping notes. If you want to do something more fancy then
//    override the "arrange()" method. Some examples of things you could do:
//
//      * Use different voices for notes that are separated by large pitch intervals.
//      * Reduce the number of voices required by splitting some notes into tied notes.
//---------------------------------------------------------

class Note;
class TDuration;

//---------------------------------------------------------
//   RawNote
//---------------------------------------------------------

class RawNote {
      int _pitch;

      Note* _tiedNoteFor;     // "real" Note outside the VirtualMeasure that this
      Note* _tiedNoteBack;    // RawNote should be tied to when it is made real.

      RawNote* _next;         // Another RawNote within the VirtualMeasure to which
      RawNote* _prev;         // this one should be tied when they are made real.

   public:
      RawNote(int pitch, Note* tiedNoteFor, Note* tiedNoteBack, RawNote* next, RawNote* prev) :
                  _pitch(pitch), _tiedNoteFor(tiedNoteFor), _tiedNoteBack(tiedNoteBack), _next(next), _prev(prev) {}

      int pitch() const             { return _pitch;   }
      Note* tiedNoteFor()           { return _tiedNoteFor;  }
      Note* tiedNoteBack()          { return _tiedNoteBack; }
      RawNote* next()               { return _next; }
      RawNote* prev()               { return _prev; }

      void setNext(RawNote* rn)     { _next = rn; }
      void setPrev(RawNote* rn)     { _prev = rn; }
      void setTiedNoteFor(Note* n)  { _tiedNoteFor = n; }
      void setTiedNoteBack(Note* n) { _tiedNoteBack = n; }
      };

//---------------------------------------------------------
//   RawChord
//---------------------------------------------------------

class RawChord {
      int _tick;
      int _ticks;
      std::vector<RawNote*> _notes;

   public:
      RawChord(int tick, int ticks, RawNote* note) : _tick(tick), _ticks(ticks) { _notes.push_back(note); }
      ~RawChord()                               { for (RawNote* rn : _notes ) delete rn; }

      std::vector<RawNote*>& notes()            { return _notes; }

      void addNote(RawNote* note)               { _notes.push_back(note); }
      bool addNotesFromChord(RawChord* chord);
      bool overlaps(RawChord* c)                { return !(c->tick() >= endTick() || c->endTick() < tick()); }
      bool coincides(RawChord* c)               { return c->tick() == tick() && c->ticks() == ticks(); }

      int tick() const                          { return _tick; }
      int ticks() const                         { return _ticks; }
      int endTick() const                       { return tick() + ticks(); }
      int numNotes() const                      { return _notes.size(); }
      int sumPitches() const;
      float averagePitch() const                { return sumPitches() * 1.0 / numNotes(); }

      void sortNotesByPitch();
      };

//---------------------------------------------------------
//   VirtualVoice
//---------------------------------------------------------

class VirtualVoice {
      std::vector<RawChord*> _chords;

   public:
      VirtualVoice(RawChord* c)           { addChord(c); }
      ~VirtualVoice()                     { for (RawChord* rc : _chords) delete rc; }

      std::vector<RawChord*>& chords()    { return _chords; }

      bool addChord(RawChord* chord);

      int numChords() const               { return _chords.size(); }
      int numNotes() const;
      int sumPitches() const;
      float averagePitch() const          { return sumPitches() * 1.0 / numNotes(); }

      void sortChordsByTick();
      };

//---------------------------------------------------------
//   VirtualMeasure
//    Provides useful abstractions for manipulating voices.
//---------------------------------------------------------

class VirtualMeasure {
      std::vector<VirtualVoice*> _voices; // Note: not limited to 4 voices

   public:
      ~VirtualMeasure()                         { for (VirtualVoice* vv : _voices) delete vv; }
      std::vector<VirtualVoice*>& voices()      { return _voices; }

      void addTiedNotes(int pitch, int tick, const std::vector<TDuration>& dList, Note* tiedNoteFor = 0, Note* tiedNoteBack = 0);

      int numVoices() const   { return _voices.size(); }
      virtual void arrange();

   private:
      void addChord(RawChord* chord);
      void sortVoicesByAveragePitch();
      };

} // namespace Ms

#endif // VIRTUALMEASURE_H
