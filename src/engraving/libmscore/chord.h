/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __CHORD_H__
#define __CHORD_H__

/**
 \file
 Definition of classes Chord, HelpLine and NoteList.
*/

#include <set>
#include <functional>
#include <vector>

#include "infrastructure/draw/color.h"
#include "chordrest.h"
#include "articulation.h"

namespace mu::engraving {
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

enum class TremoloChordType : char {
    TremoloSingle, TremoloFirstNote, TremoloSecondNote
};

class GraceNotesGroup final : public std::vector<Chord*>, public EngravingItem
{
public:
    GraceNotesGroup* clone() const override { return new GraceNotesGroup(*this); }
    GraceNotesGroup(Chord* c);

    Chord* parent() const { return _parent; }
    Shape shape() const override { return _shape; }
    void layout() override;
    void setPos(qreal x, qreal y) override;

private:
    Chord* _parent = nullptr;
    Shape _shape;
};

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

class Chord final : public ChordRest
{
    std::vector<Note*> _notes;           // sorted to decreasing line step
    LedgerLine* _ledgerLines = nullptr;  // single linked list

    Stem* _stem = nullptr;
    Hook* _hook = nullptr;
    StemSlash* _stemSlash = nullptr;     // for acciacatura

    Arpeggio* _arpeggio = nullptr;
    Tremolo* _tremolo = nullptr;
    bool _endsGlissando;                 ///< true if this chord is the ending point of a glissando (needed for layout)
    std::vector<Chord*> _graceNotes; // storage for all grace notes
    mutable GraceNotesGroup _graceNotesBefore = GraceNotesGroup(this); // will store before-chord grace notes
    mutable GraceNotesGroup _graceNotesAfter = GraceNotesGroup(this); // will store after-chord grace notes
    size_t _graceIndex;                     ///< if this is a grace note, index in parent list

    DirectionV _stemDirection;
    NoteType _noteType;                  ///< mark grace notes: acciaccatura and appoggiatura
    bool _noStem;
    PlayEventType _playEventType;        ///< play events were modified by user

    qreal _spaceLw;
    qreal _spaceRw;

    qreal _defaultStemLength;
    qreal _minStemLength;

    bool _isUiItem = false;

    std::vector<Articulation*> _articulations;

    friend class Factory;
    Chord(Segment* parent = 0);
    Chord(const Chord&, bool link = false);

    qreal upPos()   const override;
    qreal downPos() const override;
    qreal centerX() const;
    void addLedgerLines();
    void processSiblings(std::function<void(EngravingItem*)> func) const;

    void layoutPitched();
    void layoutTablature();
    qreal noteHeadWidth() const;

    bool shouldHaveStem() const;
    bool shouldHaveHook() const;

    void createStem();
    void removeStem();
    void createHook();
    void layoutHook();

    int stemLengthBeamAddition() const;
    int maxReduction(int extensionOutsideStaff) const;
    int stemOpticalAdjustment(int stemEndPosition) const;
    int calcMinStemLength();
    int calc4BeamsException(int stemLength) const;
    qreal calcDefaultStemLength();

public:

    ~Chord();
    Chord& operator=(const Chord&) = delete;

    bool containsEqualArticulations(const Chord* other) const;
    bool containsEqualArpeggio(const Chord* other) const;
    bool containsEqualTremolo(const Chord* other) const;

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    Chord* clone() const override { return new Chord(*this, false); }
    EngravingItem* linkedClone() override { return new Chord(*this, true); }
    void undoUnlink() override;

    void setScore(Score* s) override;
    qreal chordMag() const;
    qreal mag() const override;

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    bool readProperties(XmlReader&) override;
    EngravingItem* drop(EditData&) override;

    void setColor(const mu::draw::Color& c) override;
    void setStemDirection(DirectionV d);
    DirectionV stemDirection() const { return _stemDirection; }

    Chord* prev() const;
    Chord* next() const;

    void setIsUiItem(bool val) { _isUiItem = val; }

    LedgerLine* ledgerLines() { return _ledgerLines; }

    qreal defaultStemLength() const { return _defaultStemLength; }
    qreal minStemLength() const { return _minStemLength; }
    void setBeamExtension(qreal extension);
    static int minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, qreal beamSpacing, bool useWideBeams);

    void layoutStem();
    void layoutArpeggio2();
    void layoutSpanners();
    void layoutSpanners(System* system, const Fraction& stick);

    std::vector<Note*>& notes() { return _notes; }
    const std::vector<Note*>& notes() const { return _notes; }

    bool isChordPlayable() const;
    void setIsChordPlayable(const bool isPlayable);

    // Chord has at least one Note
    Note* upNote() const;
    Note* downNote() const;
    int upString() const;
    int downString() const;
    std::vector<int> noteDistances() const;

    qreal maxHeadWidth() const;

    Note* findNote(int pitch, int skip = 0) const;

    Stem* stem() const { return _stem; }
    Arpeggio* arpeggio() const { return _arpeggio; }
    Tremolo* tremolo() const { return _tremolo; }
    ChordLine* chordLine() const;
    void setTremolo(Tremolo* t);
    bool endsGlissando() const { return _endsGlissando; }
    void setEndsGlissando(bool val) { _endsGlissando = val; }
    void updateEndsGlissando();
    StemSlash* stemSlash() const { return _stemSlash; }
    bool slash();
    void setSlash(bool flag, bool stemless);
    void removeMarkings(bool keepTremolo = false) override;

    const std::vector<Chord*>& graceNotes() const { return _graceNotes; }
    std::vector<Chord*>& graceNotes() { return _graceNotes; }

    GraceNotesGroup& graceNotesBefore() const;
    GraceNotesGroup& graceNotesAfter() const;

    size_t graceIndex() const { return _graceIndex; }
    void setGraceIndex(size_t val) { _graceIndex = val; }

    int upLine() const override;
    int downLine() const override;
    mu::PointF stemPos() const override;            ///< page coordinates
    mu::PointF stemPosBeam() const override;        ///< page coordinates
    qreal stemPosX() const override;
    qreal rightEdge() const override;

    bool underBeam() const;
    Hook* hook() const { return _hook; }

    //@ add an element to the Chord
    Q_INVOKABLE void add(EngravingItem*) override;
    //@ remove the element from the Chord
    Q_INVOKABLE void remove(EngravingItem*) override;

    Note* selectedNote() const;
    void layout() override;
    mu::PointF pagePos() const override;        ///< position in page coordinates
    void cmdUpdateNotes(AccidentalState*);

    NoteType noteType() const { return _noteType; }
    void setNoteType(NoteType t) { _noteType = t; }
    bool isGrace() const { return _noteType != NoteType::NORMAL; }
    void toGraceAfter();

    void setTrack(track_idx_t val) override;

    void computeUp() override;
    static int computeAutoStemDirection(const std::vector<int>& noteDistances);

    qreal dotPosX() const;

    bool noStem() const { return _noStem; }
    void setNoStem(bool val) { _noStem = val; }

    PlayEventType playEventType() const { return _playEventType; }
    void setPlayEventType(PlayEventType v) { _playEventType = v; }
    std::vector<NoteEventList> getNoteEventLists();
    void setNoteEventLists(std::vector<NoteEventList>& nel);

    TremoloChordType tremoloChordType() const;

    void layoutArticulations();
    void layoutArticulations2();
    void layoutArticulations3(Slur* s);

    std::vector<Articulation*>& articulations() { return _articulations; }
    const std::vector<Articulation*>& articulations() const { return _articulations; }
    std::set<SymId> articulationSymbolIds() const;
    Articulation* hasArticulation(const Articulation*);
    bool hasSingleArticulation() const { return _articulations.size() == 1; }

    void updateArticulations(const std::set<SymId>& newArticulationIds,
                             ArticulationsUpdateMode replaceMode = ArticulationsUpdateMode::Insert);

    void crossMeasureSetup(bool on) override;

    void localSpatiumChanged(qreal oldValue, qreal newValue) override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void reset() override;

    Segment* segment() const override;
    Measure* measure() const override;

    void sortNotes();

    Chord* nextTiedChord(bool backwards = false, bool sameSize = true);

    EngravingItem* nextElement() override;
    EngravingItem* prevElement() override;
    EngravingItem* nextSegmentElement() override;
    EngravingItem* lastElementBeforeSegment();
    EngravingItem* prevSegmentElement() override;
    String accessibleExtraInfo() const override;

    Shape shape() const override;
    void undoChangeProperty(Pid id, const PropertyValue& newValue);
    void undoChangeProperty(Pid id, const PropertyValue& newValue, PropertyFlags ps) override;
};
} // namespace mu::engraving
#endif
