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
enum class PlayEventType : char    {
      Auto,       // Play events for all notes are calculated by MuseScore.
      User,       // Some play events are modified by user. The events must be written into the mscx file.
      InvalidUser // The user modified play events must be replaced by MuseScore generated ones on
                  // next recalculation. The actual play events must be saved on the undo stack.
      };

//---------------------------------------------------------
//   @@ Chord
///    Graphic representation of a chord.
///    Single notes are handled as degenerated chords.
//
//   @P beam        Beam            the beam of the chord if any (read only)
//   @P graceNotes  array[Chord]    the list of grace note chords (read only)
//   @P hook        Hook            the hook of the chord if any (read only)
//   @P lyrics      array[Lyrics]   the list of lyrics (read only)
//   @P notes       array[Note]     the list of notes (read only)
//   @P stem        Stem            the stem of the chord if any (read only)
//   @P stemSlash   StemSlash       the stem slash of the chord (acciaccatura) if any (read only)
//   @P stemDirection Direction       the stem slash of the chord (acciaccatura) if any (read only)
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

      virtual qreal upPos()   const;
      virtual qreal downPos() const;
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

      virtual Chord* clone() const       { return new Chord(*this, false); }
      virtual Element* linkedClone()     { return new Chord(*this, true); }
      virtual void undoUnlink() override;

      virtual void setScore(Score* s);
      virtual ElementType type() const         { return ElementType::CHORD; }
      virtual qreal mag() const;

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual bool readProperties(XmlReader&) override;
      virtual Element* drop(EditData&) override;

      void setStemDirection(Direction d) { _stemDirection = d; }
      Direction stemDirection() const    { return _stemDirection; }

      LedgerLine* ledgerLines()                  { return _ledgerLines; }

      qreal defaultStemLength();

      virtual void layoutStem1() override;
      void layoutStem();
      void layoutArpeggio2();

      std::vector<Note*>& notes()                 { return _notes; }
      const std::vector<Note*>& notes() const     { return _notes; }

      // Chord has at least one Note
      Note* upNote() const;
      Note* downNote() const;
      virtual int upString() const;
      virtual int downString() const;

      qreal maxHeadWidth() const;

      Note* findNote(int pitch) const;

      Stem* stem() const                     { return _stem; }
      Arpeggio* arpeggio() const             { return _arpeggio;  }
      Tremolo* tremolo() const               { return _tremolo;   }
      void setTremolo(Tremolo* t)            { _tremolo = t;      }
      bool endsGlissando() const             { return _endsGlissando; }
      void setEndsGlissando (bool val)       { _endsGlissando = val; }
      void updateEndsGlissando();
      StemSlash* stemSlash() const           { return _stemSlash; }
      bool slash();
      void setSlash(bool flag, bool stemless);
      virtual void removeMarkings(bool keepTremolo = false) override;

      const QVector<Chord*>& graceNotes() const { return _graceNotes; }
      QVector<Chord*>& graceNotes()             { return _graceNotes; }

      QVector<Chord*> graceNotesBefore() const;
      QVector<Chord*> graceNotesAfter() const;

      int graceIndex() const                        { return _graceIndex; }
      void setGraceIndex(int val)                   { _graceIndex = val;  }

      virtual int upLine() const;
      virtual int downLine() const;
      virtual QPointF stemPos() const;          ///< page coordinates
      virtual QPointF stemPosBeam() const;      ///< page coordinates
      virtual qreal stemPosX() const;
      bool underBeam() const;
      Hook* hook() const                     { return _hook; }

      //@ add an element to the Chord
      Q_INVOKABLE virtual void add(Ms::Element*);
      //@ remove the element from the Chord
      Q_INVOKABLE virtual void remove(Ms::Element*);

      Note* selectedNote() const;
      virtual void layout();
      virtual QPointF pagePos() const override;      ///< position in page coordinates
      void layout2();
      void cmdUpdateNotes(AccidentalState*);

      NoteType noteType() const       { return _noteType; }
      void setNoteType(NoteType t)    { _noteType = t; }
      bool isGrace() const            { return _noteType != NoteType::NORMAL; }
      void toGraceAfter();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      virtual void setTrack(int val) override;

      virtual void computeUp() override;

      qreal dotPosX() const;

      bool noStem() const                           { return _noStem;  }
      void setNoStem(bool val)                      { _noStem = val;   }

      PlayEventType playEventType() const           { return _playEventType; }
      void setPlayEventType(PlayEventType v)        { _playEventType = v;    }

      TremoloChordType tremoloChordType() const;

      void layoutArticulations();
      void layoutArticulations2();

      QVector<Articulation*>& articulations()             { return _articulations; }
      const QVector<Articulation*>& articulations() const { return _articulations; }
      Articulation* hasArticulation(const Articulation*);
      bool hasSingleArticulation() const                  { return _articulations.size() == 1; }

      virtual void crossMeasureSetup(bool on);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual void reset();

      virtual Segment* segment() const;
      virtual Measure* measure() const;

      void sortNotes();

      Chord* nextTiedChord(bool backwards = false, bool sameSize = true);

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      virtual Element* nextSegmentElement() override;
      virtual Element* lastElementBeforeSegment();
      virtual Element* prevSegmentElement() override;
      virtual QString accessibleExtraInfo() const override;

      virtual Shape shape() const override;
      };


}     // namespace Ms
#endif

