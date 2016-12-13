//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "utils.h"
#include "score.h"
#include "chord.h"
#include "measure.h"
#include "tie.h"
#include "tuplet.h"
#include "staff.h"
#include "part.h"
#include "drumset.h"
#include "slur.h"
#include "navigate.h"
#include "stringdata.h"
#include "undo.h"
#include "range.h"
#include "excerpt.h"

namespace Ms {

//---------------------------------------------------------
//   cmdAddPitch
//---------------------------------------------------------

void Score::cmdAddPitch(int step, bool addFlag, bool insert)
      {
      startCmd();
      Position pos;
      if (addFlag) {
            Element* el = selection().element();
            if (el && el->isNote()) {
                  Note* selectedNote = toNote(el);
                  Chord* chord  = selectedNote->chord();
                  Segment* seg  = chord->segment();
                  pos.segment   = seg;
                  pos.staffIdx  = selectedNote->track() / VOICES;
                  ClefType clef = staff(pos.staffIdx)->clef(seg->tick());
                  pos.line      = relStep(step, clef);
                  bool error;
                  NoteVal nval = noteValForPosition(pos, error);
                  if (error) {
                        endCmd();
                        return;
                        }
                  addNote(chord, nval);
                  endCmd();
                  return;
                  }
            }

      pos.segment   = inputState().segment();
      pos.staffIdx  = inputState().track() / VOICES;
      ClefType clef = staff(pos.staffIdx)->clef(pos.segment->tick());
      pos.line      = relStep(step, clef);

      if (inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH))
            repitchNote(pos, !addFlag);
      else {
            if (insert)
                  insertChord(pos);
            else
                  putNote(pos, !addFlag);
            }
      endCmd();
      }

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(NoteVal& nval, bool addFlag)
      {
      if (addFlag) {
            Chord* c = toChord(_is.lastSegment()->element(_is.track()));

            if (c == 0 || !c->isChord()) {
                  qDebug("Score::addPitch: cr %s", c ? c->name() : "zero");
                  return 0;
                  }
            Note* note = addNote(c, nval);
            if (_is.lastSegment() == _is.segment()) {
                  NoteEntryMethod entryMethod = _is.noteEntryMethod();
                  if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL)
                        _is.moveToNextInputPos();
                  }
            return note;
            }
      expandVoice();

      // insert note
      Direction stemDirection = Direction::AUTO;
      int track               = _is.track();
      if (_is.drumNote() != -1) {
            nval.pitch        = _is.drumNote();
            const Drumset* ds = _is.drumset();
            nval.headGroup    = ds->noteHead(nval.pitch);
            stemDirection     = ds->stemDirection(nval.pitch);
            track             = ds->voice(nval.pitch) + (_is.track() / VOICES) * VOICES;
            _is.setTrack(track);
            expandVoice();
            }
      if (!_is.cr())
            return 0;
      Fraction duration;
      if (_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
            duration = _is.cr()->duration();
            }
      else {
            duration = _is.duration().fraction();
            }
      Note* note = 0;
      Note* firstTiedNote = 0;
      Note* lastTiedNote = 0;
      if (_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH) && _is.cr()->isChord()) {
            // repitch mode for MIDI input (where we are given a pitch) is handled here
            // for keyboard input (where we are given a staff position), there is a separate function Score::repitchNote()
            // the code is similar enough that it could possibly be refactored
            Chord* chord = toChord(_is.cr());
            note = new Note(this);
            note->setParent(chord);
            note->setTrack(chord->track());
            note->setNval(nval);
            lastTiedNote = note;
            if (!addFlag) {
                  std::vector<Note*> notes = chord->notes();
                  // break all ties into current chord
                  // these will exist only if user explicitly moved cursor to a tied-into note
                  // in ordinary use, cursor will autoamtically skip past these during note entry
                  for (Note* n : notes) {
                        if (n->tieBack())
                              undoRemoveElement(n->tieBack());
                        }
                  // for single note chords only, preserve ties by changing pitch of all forward notes
                  // the tie forward itself will be added later
                  // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
                  // so there will be no way to preserve those ties
                  if (notes.size() == 1 && notes.front()->tieFor()) {
                        Note* tn = notes.front()->tieFor()->endNote();
                        while (tn) {
                              Chord* tc = tn->chord();
                              if (tc->notes().size() != 1) {
                                    undoRemoveElement(tn->tieBack());
                                    break;
                                    }
                              if (!firstTiedNote)
                                    firstTiedNote = tn;
                              lastTiedNote = tn;
                              undoChangePitch(tn, note->pitch(), note->tpc1(), note->tpc2());
                              if (tn->tieFor())
                                    tn = tn->tieFor()->endNote();
                              else
                                    break;
                              }
                        }
                  // remove all notes from chord
                  // the new note will be added below
                  while (!chord->notes().empty())
                        undoRemoveElement(chord->notes().front());
                  }
            // add new note to chord
            undoAddElement(note);
            // recreate tie forward if there is a note to tie to
            // one-sided ties will not be recreated
            if (firstTiedNote) {
                  Tie* tie = new Tie(this);
                  tie->setStartNote(note);
                  tie->setEndNote(firstTiedNote);
                  tie->setTrack(note->track());
                  undoAddElement(tie);
                  }
            select(lastTiedNote);
            }
      else if (!_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
            Segment* seg = setNoteRest(_is.segment(), track, nval, duration, stemDirection);
            if (seg) {
                  note = toChord(seg->element(track))->upNote();
                  }
            }

      if (_is.slur()) {
            //
            // extend slur
            //
            ChordRest* e = searchNote(_is.tick(), _is.track());
            if (e) {
                  int stick = 0;
                  Element* ee = _is.slur()->startElement();
                  if (ee->isChordRest())
                        stick = toChordRest(ee)->tick();
                  else if (ee->isNote())
                        stick = toNote(ee)->chord()->tick();
                  if (stick == e->tick()) {
                        _is.slur()->setTick(stick);
                        _is.slur()->setStartElement(e);
                        }
                  else {
                        _is.slur()->setTick2(e->tick());
                        _is.slur()->setEndElement(e);
                        }
                  }
            else
                  qDebug("addPitch: cannot find slur note");
            }
      if (_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
            // move cursor to next note, but skip tied notes (they were already repitched above)
            ChordRest* next = lastTiedNote ? nextChordRest(lastTiedNote->chord()) : nextChordRest(_is.cr());
            while (next && !next->isChord())
                  next = nextChordRest(next);
            if (next)
                  _is.moveInputPos(next->segment());
            }
      else {
            NoteEntryMethod entryMethod = _is.noteEntryMethod();
            if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL)
                  _is.moveToNextInputPos();
            }
      return note;
      }

//---------------------------------------------------------
//   putNote
//    mouse click in state NoteType::ENTRY
//---------------------------------------------------------

void Score::putNote(const QPointF& pos, bool replace, bool insert)
      {
      Position p;
      if (!getPosition(&p, pos, _is.voice())) {
            qDebug("cannot put note here, get position failed");
            return;
            }
      if (inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH))
            repitchNote(p, replace);
      else {
            if (insert)
                  insertChord(p);
            else
                  putNote(p, replace);
            }
      }

void Score::putNote(const Position& p, bool replace)
      {
      int staffIdx    = p.staffIdx;
      Staff* st       = staff(staffIdx);
      Segment* s      = p.segment;

      _is.setTrack(staffIdx * VOICES + _is.voice());
      _is.setSegment(s);

      if (score()->excerpt() && !score()->excerpt()->tracks().isEmpty()
       && score()->excerpt()->tracks().key(_is.track(), -1) == -1)
            return;

      Direction stemDirection = Direction::AUTO;
      bool error;
      NoteVal nval = noteValForPosition(p, error);
      if (error)
            return;

      const StringData* stringData = 0;
      switch (st->staffType(s->tick())->group()) {
            case StaffGroup::PERCUSSION: {
                  const Drumset* ds = st->part()->instrument(s->tick())->drumset();
                  stemDirection     = ds->stemDirection(nval.pitch);
                  break;
                  }
            case StaffGroup::TAB:
                  stringData = st->part()->instrument(s->tick())->stringData();
                  break;
            case StaffGroup::STANDARD:
                  break;
            }

      expandVoice();
      ChordRest* cr = _is.cr();
      bool addToChord = false;

      if (cr) {
            // retrieve total duration of current chord
            TDuration d = cr->durationType();
            // if not in replace mode AND chord duration == input duration AND not rest input
            // we need to add to current chord (otherwise, we will need to replace it or create a new one)
            if (!replace
               && (d == _is.duration())
               && cr->isChord()
               && !_is.rest())
                  {
                  if (st->isTabStaff(cr->tick())) {      // TAB
                        // if a note on same string already exists, update to new pitch/fret
                        foreach (Note* note, toChord(cr)->notes())
                              if (note->string() == nval.string) {       // if string is the same
                                    // if adding a new digit will keep fret number within fret limit,
                                    // add a digit to existing fret number
                                    if (stringData) {
                                          int fret = note->fret() * 10 + nval.fret;
                                          if (fret <= stringData->frets() ) {
                                                nval.fret = fret;
                                                nval.pitch = stringData->getPitch(nval.string, nval.fret, st, s->tick());
                                                }
                                          else
                                                qDebug("can't increase fret to %d", fret);
                                          }
                                    // set fret number (original or combined) in all linked notes
                                    int tpc1 = note->tpc1default(nval.pitch);
                                    int tpc2 = note->tpc2default(nval.pitch);
                                    undoChangeFretting(note, nval.pitch, nval.string, nval.fret, tpc1, tpc2);
                                    return;
                                    }
                        }
                  else {                        // not TAB
                        // if a note with the same pitch already exists in the chord, remove it
                        Chord* chord = toChord(cr);
                        Note* note = chord->findNote(nval.pitch);
                        if (note) {
                              if (chord->notes().size() > 1)
                                    undoRemoveElement(note);
                              return;
                              }
                        }
                  addToChord = true;            // if no special case, add note to chord
                  }
            }
      if (addToChord && cr->isChord()) {
            // if adding, add!
            addNote(toChord(cr), nval);
            return;
            }
      else {
            // if not adding, replace current chord (or create a new one)

            if (_is.rest())
                  nval.pitch = -1;
            setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction(), stemDirection);
            }
      if (!st->isTabStaff(cr->tick()))
            _is.moveToNextInputPos();
      }

//---------------------------------------------------------
//   repitchNote
//---------------------------------------------------------

void Score::repitchNote(const Position& p, bool replace)
      {
      Segment* s      = p.segment;
      int tick        = s->tick();
      Staff* st       = staff(p.staffIdx);
      ClefType clef   = st->clef(tick);

      NoteVal nval;
      bool error = false;
      AccidentalVal acci = s->measure()->findAccidental(s, p.staffIdx, p.line, error);
      if (error)
            return;
      int step   = absStep(p.line, clef);
      int octave = step / 7;
      nval.pitch = step2pitch(step) + octave * 12 + int(acci);

      if (styleB(StyleIdx::concertPitch))
            nval.tpc1 = step2tpc(step % 7, acci);
      else {
            nval.pitch += st->part()->instrument(s->tick())->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
            }

      if (!_is.segment())
            return;

      Chord* chord;
      ChordRest* cr = _is.cr();
      if (!cr) {
            cr = _is.segment()->nextChordRest(_is.track());
            if (!cr)
                  return;
            }
      if (cr->isRest()) { //skip rests
            ChordRest* next = nextChordRest(cr);
            while(next && !next->isChord())
                  next = nextChordRest(next);
            if (next)
                  _is.moveInputPos(next->segment());
            return;
            }
      else {
            chord = toChord(cr);
            }
      Note* note = new Note(this);
      note->setParent(chord);
      note->setTrack(chord->track());
      note->setNval(nval);

      Note* firstTiedNote = 0;
      Note* lastTiedNote = note;
      if (replace) {
            std::vector<Note*> notes = chord->notes();
            // break all ties into current chord
            // these will exist only if user explicitly moved cursor to a tied-into note
            // in ordinary use, cursor will autoamtically skip past these during note entry
            for (Note* n : notes) {
                  if (n->tieBack())
                        undoRemoveElement(n->tieBack());
                  }
            // for single note chords only, preserve ties by changing pitch of all forward notes
            // the tie forward itself will be added later
            // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
            // so there will be no way to preserve those ties
            if (notes.size() == 1 && notes.front()->tieFor()) {
                  Note* tn = notes.front()->tieFor()->endNote();
                  while (tn) {
                        Chord* tc = tn->chord();
                        if (tc->notes().size() != 1) {
                              undoRemoveElement(tn->tieBack());
                              break;
                              }
                        if (!firstTiedNote)
                              firstTiedNote = tn;
                        lastTiedNote = tn;
                        undoChangePitch(tn, note->pitch(), note->tpc1(), note->tpc2());
                        if (tn->tieFor())
                              tn = tn->tieFor()->endNote();
                        else
                              break;
                        }
                  }
            // remove all notes from chord
            // the new note will be added below
            while (!chord->notes().empty())
                  undoRemoveElement(chord->notes().front());
            }
      // add new note to chord
      undoAddElement(note);
      setPlayNote(true);
      setPlayChord(true);
      // recreate tie forward if there is a note to tie to
      // one-sided ties will not be recreated
      if (firstTiedNote) {
            Tie* tie = new Tie(this);
            tie->setStartNote(note);
            tie->setEndNote(firstTiedNote);
            tie->setTrack(note->track());
            undoAddElement(tie);
            }
      select(lastTiedNote);
      // move to next Chord
      ChordRest* next = nextChordRest(lastTiedNote->chord());
      while (next && !next->isChord())
            next = nextChordRest(next);
      if (next)
            _is.moveInputPos(next->segment());
      }

//---------------------------------------------------------
//   insertChord
//---------------------------------------------------------

void Score::insertChord(const Position& pos)
      {
      // insert
      // TODO:
      //    - check voices
      //    - split chord/rest

      Element* el = selection().element();
      if (!el || !(el->isNote() || el->isRest()))
            return;
      Segment* seg = pos.segment;
      if (seg->splitsTuplet()) {
            MScore::setError(CANNOT_INSERT_TUPLET);
            return;
            }
      if (_is.insertMode())
            globalInsertChord(pos);
      else
            localInsertChord(pos);
      }

//---------------------------------------------------------
//   localInsertChord
//---------------------------------------------------------

void Score::localInsertChord(const Position& pos)
      {
      TDuration duration = _is.duration();
      Fraction fraction  = duration.fraction();
      int len            = fraction.ticks();
      Segment* seg       = pos.segment;
      int tick           = seg->tick();
      Measure* m         = seg->measure();

      undoInsertTime(tick, len);
      undo(new InsertTime(this, tick, len));

      for (Segment* s = pos.segment; s; s = s-> next())
            s->undoChangeProperty(P_ID::TICK, s->rtick() + len);
      undo(new ChangeMeasureLen(m, m->len() + fraction));

      Segment* s = m->undoGetSegment(Segment::Type::ChordRest, tick);
      Position p(pos);
      p.segment = s;

      int trackI = p.staffIdx * VOICES + _is.voice();
      for (int track = 0; track < _staves.size() * VOICES; ++track) {
            if (track == trackI)
                  putNote(p, true);
            else {
                  Segment* fs = m->first(Segment::Type::ChordRest);
                  if (fs->tick() == tick && m->hasVoice(track)) {
                        setRest(fs->tick(),  track, fraction, false, nullptr, false);
                        continue;
                        }
                  Segment* seg1 = 0;
                  for (Segment* s = fs; s; s = s->next(Segment::Type::ChordRest)) {
                        if (s->element(track)) {
                              ChordRest* cr = toChordRest(s->element(track));
                              if (s->tick() > tick)
                                    break;
                              if (s->tick() + cr->duration().ticks() < tick)
                                    continue;
                              seg1 = s;
                              break;
                              }
                        }
                  if (seg1) {
                        ChordRest* cr = toChordRest(seg1->element(track));
                        if (seg1->tick() + cr->duration().ticks() == tick) {
                              addRest(s, track, duration, nullptr);
                              }
                        else if (cr->isFullMeasureRest()) {
                              // do nothing
                              }
                        else
                              changeCRlen(cr, fraction + cr->duration());
                        }
                  }
            }
      }

//---------------------------------------------------------
//   globalInsertChord
//---------------------------------------------------------

void Score::globalInsertChord(const Position& pos)
      {
      ChordRest* cr = selection().cr();
      int track = cr ? cr->track() : -1;
      deselectAll();
      Segment* s1        = pos.segment;
      Segment* s2        = lastSegment();
      TDuration duration = _is.duration();
      Fraction fraction  = duration.fraction();
      ScoreRange r;

      r.read(s1, s2, false);

      int strack = 0;                      // for now for all tracks
      int etrack = nstaves() * VOICES;
      int stick  = s1->tick();
      int etick  = s2->tick();
      int ticks  = fraction.ticks();

      Fraction len = r.duration();
      if (!r.truncate(fraction))
            appendMeasures(1);

      putNote(pos, true);
      int dtick = s1->tick() + ticks;
      int voiceOffsets[VOICES] { 0, 0, 0, 0 };
      len = r.duration();
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
            makeGap1(dtick, staffIdx, r.duration(), voiceOffsets);
      r.write(this, dtick);

      for (auto i :  spanner()) {
            Spanner* s = i.second;
            if (s->track() >= strack && s->track() < etrack) {
                  if (s->tick() >= stick && s->tick() < etick)
                        s->undoChangeProperty(P_ID::SPANNER_TICK, s->tick() + ticks);
                  else if (s->tick2() >= stick && s->tick2() < etick)
                        s->undoChangeProperty(P_ID::SPANNER_TICKS, s->ticks() + ticks);
                  }
            }

      if (track != -1) {
            Measure* m = tick2measure(dtick);
            Segment* s = m->findSegment(Segment::Type::ChordRest, dtick);
            Element* e = s->element(track);
            if (e)
                  select(e->isChord() ? toChord(e)->notes().front() : e);
            }
      }


} // namespace Ms

