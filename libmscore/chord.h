//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CHORD_H__
#define __CHORD_H__

/**
 \file
 Definition of classes Chord, HelpLine and NoteList.
*/

#include <functional>
#include "chordrest.h"

namespace Ms {

class Note;
class Hook;
class Arpeggio;
class Tremolo;
class Chord;
//class Glissando;
class Stem;
class Chord;
class StemSlash;
class LedgerLine;
class AccidentalState;

enum class TremoloChordType : char { TremoloSingle, TremoloFirstNote, TremoloSecondNote };

//---------------------------------------------------------
//   @@ Chord
///    Graphic representation of a chord.
///    Single notes are handled as degenerated chords.
//
//   @P beam          Beam          the beam of the chord, if any (read only)
//   @P graceNotes    array[Chord]  the list of grace note chords (read only)
//   @P hook          Hook          the hook of the chord, if any (read only)
//   @P lyrics        array[Lyrics] the list of lyrics (read only)
//   @P notes         array[Note]   the list of notes (read only)
//   @P stem          Stem          the stem of the chord, if any (read only)
//   @P stemSlash     StemSlash     the stem slash of the chord (acciaccatura), if any (read only)
//   @P stemDirection Direction     the stem direction of the chord: AUTO, UP, DOWN (read only)
//---------------------------------------------------------

class Chord final : public ChordRest {
      std::vector<Note*>   _notes;       // sorted to decreasing line step
      LedgerLine*          _ledgerLines; // single linked list

      Stem*               _stem;
      Hook*               _hook;
      StemSlash*          _stemSlash;    // for acciacatura

      Arpeggio*           _arpeggio;
      Tremolo*            _tremolo;
      bool                _endsGlissando;///< true if this chord is the ending point of a glissando (needed for layout)
      QVector<Chord*>     _graceNotes;
      int                 _graceIndex;   ///< if this is a grace note, index in parent list

      Direction          _stemDirection;
      NoteType           _noteType;      ///< mark grace notes: acciaccatura and appoggiatura
      bool               _noStem;
      PlayEventType      _playEventType; ///< play events were modified by user

      qreal _spaceLw;
      qreal _spaceRw;

      QVector<Articulation*> _articulations;

      qreal upPos()   const override;
      qreal downPos() const override;
      qreal centerX() const;
      void addLedgerLines();
      void processSiblings(std::function<void(Element*)> func) const;

      void layoutPitched();
      void layoutTablature();
      qreal noteHeadWidth() const;

   public:
      Chord(Score* s = 0);
      Chord(const Chord&, bool link = false);
      ~Chord();
      Chord &operator=(const Chord&) = delete;

      Chord* clone() const override       { return new Chord(*this, false); }
      Element* linkedClone() override     { return new Chord(*this, true); }
      void undoUnlink() override;

      void setScore(Score* s) override;
      ElementType type() const override   { return ElementType::CHORD; }
      qreal chordMag() const;
      qreal mag() const override;

      void write(XmlWriter& xml) const override;
      void read(XmlReader&) override;
      bool readProperties(XmlReader&) override;
      Element* drop(EditData&) override;

      void setStemDirection(Direction d) { _stemDirection = d; }
      Direction stemDirection() const    { return _stemDirection; }

      LedgerLine* ledgerLines()                  { return _ledgerLines; }

      qreal defaultStemLength() const;
      qreal minAbsStemLength() const;

      void layoutStem1() override;
      void layoutStem();
      void layoutArpeggio2();
      void layoutSpanners();
      void layoutSpanners(System* system, const Fraction& stick);

      std::vector<Note*>& notes()                 { return _notes; }
      const std::vector<Note*>& notes() const     { return _notes; }

      // Chord has at least one Note
      Note* upNote() const;
      Note* downNote() const;
      int upString() const;
      int downString() const;

      qreal maxHeadWidth() const;

      Note* findNote(int pitch, int skip = 0) const;

      Stem* stem() const                     { return _stem; }
      Arpeggio* arpeggio() const             { return _arpeggio;  }
      Tremolo* tremolo() const               { return _tremolo;   }
      void setTremolo(Tremolo* t);
      bool endsGlissando() const             { return _endsGlissando; }
      void setEndsGlissando (bool val)       { _endsGlissando = val; }
      void updateEndsGlissando();
      StemSlash* stemSlash() const           { return _stemSlash; }
      bool slash();
      void setSlash(bool flag, bool stemless);
      void removeMarkings(bool keepTremolo = false) override;

      const QVector<Chord*>& graceNotes() const { return _graceNotes; }
      QVector<Chord*>& graceNotes()             { return _graceNotes; }

      QVector<Chord*> graceNotesBefore() const;
      QVector<Chord*> graceNotesAfter() const;

      int graceIndex() const                        { return _graceIndex; }
      void setGraceIndex(int val)                   { _graceIndex = val;  }

      int upLine() const override;
      int downLine() const override;
      QPointF stemPos() const override;          ///< page coordinates
      QPointF stemPosBeam() const override;      ///< page coordinates
      qreal stemPosX() const override;
      qreal rightEdge() const override;

      bool underBeam() const;
      Hook* hook() const                     { return _hook; }

      //@ add an element to the Chord
      Q_INVOKABLE void add(Ms::Element*) override;
      //@ remove the element from the Chord
      Q_INVOKABLE void remove(Ms::Element*) override;

      Note* selectedNote() const;
      void layout() override;
      QPointF pagePos() const override;      ///< position in page coordinates
      void layout2();
      void cmdUpdateNotes(AccidentalState*);

      NoteType noteType() const       { return _noteType; }
      void setNoteType(NoteType t)    { _noteType = t; }
      bool isGrace() const            { return _noteType != NoteType::NORMAL; }
      void toGraceAfter();
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      void setTrack(int val) override;

      void computeUp() override;

      qreal dotPosX() const;

      bool noStem() const                           { return _noStem;  }
      void setNoStem(bool val)                      { _noStem = val;   }

      PlayEventType playEventType() const           { return _playEventType; }
      void setPlayEventType(PlayEventType v)        { _playEventType = v;    }
      QList<NoteEventList> getNoteEventLists();
      void setNoteEventLists(QList<NoteEventList>& nel);

      TremoloChordType tremoloChordType() const;

      void layoutArticulations();
      void layoutArticulations2();
      void layoutArticulations3(Slur* s);

      QVector<Articulation*>& articulations()             { return _articulations; }
      const QVector<Articulation*>& articulations() const { return _articulations; }
      Articulation* hasArticulation(const Articulation*);
      bool hasSingleArticulation() const                  { return _articulations.size() == 1; }

      void crossMeasureSetup(bool on) override;

      void localSpatiumChanged(qreal oldValue, qreal newValue) override;
      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      void reset() override;

      Segment* segment() const override;
      Measure* measure() const override;

      void sortNotes();

      Chord* nextTiedChord(bool backwards = false, bool sameSize = true);

      Element* nextElement() override;
      Element* prevElement() override;
      Element* nextSegmentElement() override;
      Element* lastElementBeforeSegment();
      Element* prevSegmentElement() override;
      QString accessibleExtraInfo() const override;

      Shape shape() const override;
      };


}     // namespace Ms
#endif

