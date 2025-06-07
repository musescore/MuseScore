/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_CHORD_H
#define MU_ENGRAVING_CHORD_H

/**
 \file
 Definition of classes Chord, HelpLine and NoteList.
*/

#include <functional>
#include <set>
#include <vector>

#include "chordrest.h"

#include "articulation.h"
#include "types.h"

#include "draw/types/color.h"

namespace mu::engraving {
class AccidentalState;
class Arpeggio;
class Chord;
class Hook;
class LedgerLine;
class Note;
class NoteEventList;
class Stem;
class StemSlash;
class TremoloTwoChord;
class TremoloSingleChord;

class GraceNotesGroup final : public std::vector<Chord*>, public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, GraceNotesGroup)
public:
    GraceNotesGroup* clone() const override { return new GraceNotesGroup(*this); }
    GraceNotesGroup(Chord* c);

    Chord* parent() const { return _parent; }

    void setPos(double x, double y) override;
    Segment* appendedSegment() const { return _appendedSegment; }
    void setAppendedSegment(Segment* s) { _appendedSegment = s; }
    void addToShape();

private:
    Chord* _parent = nullptr;
    Segment* _appendedSegment = nullptr; // the graceNoteGroup is appended to this segment
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
    OBJECT_ALLOCATOR(engraving, Chord)
    DECLARE_CLASSOF(ElementType::CHORD)

public:

    ~Chord();
    Chord& operator=(const Chord&) = delete;

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    Chord* clone() const override { return new Chord(*this, false); }
    EngravingItem* linkedClone() override { return new Chord(*this, true); }
    void undoUnlink() override;

    void setScore(Score* s) override;
    double intrinsicMag() const override;
    double mag() const override;
    double noteHeadWidth() const;

    EngravingItem* drop(EditData&) override;

    void setColor(const Color& c) override;
    void setStemDirection(DirectionV d);
    DirectionV stemDirection() const { return m_stemDirection; }

    Chord* prev() const;
    Chord* next() const;

    bool isUiItem() const { return m_isUiItem; }
    void setIsUiItem(bool val) { m_isUiItem = val; }

    const std::vector<LedgerLine*>& ledgerLines() const { return m_ledgerLines; }
    void resizeLedgerLinesTo(size_t newSize);
    void updateLedgerLines();

    double defaultStemLength() const { return m_defaultStemLength; }
    void setDefaultStemLength(double l) { m_defaultStemLength = l; }
    double minStemLength() const { return m_minStemLength; }
    void setBeamExtension(double extension);
    static int minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, double beamSpacing, bool useWideBeams,
                               bool isFullSize);

    std::vector<Note*>& notes() { return m_notes; }
    const std::vector<Note*>& notes() const { return m_notes; }

    bool isChordPlayable() const;
    void setIsChordPlayable(const bool isPlayable);

    // Chord has at least one Note
    Note* upNote() const;
    Note* downNote() const;
    int upString() const;
    int downString() const;
    std::vector<int> noteDistances() const;

    double maxHeadWidth() const;

    Note* findNote(int pitch, int skip = 0) const;

    Stem* stem() const { return m_stem; }
    Arpeggio* arpeggio() const { return m_arpeggio; }
    void setArpeggio(Arpeggio* a) { m_arpeggio = a; }

    Arpeggio* spanArpeggio() const { return m_spanArpeggio; }
    void setSpanArpeggio(Arpeggio* a) { m_spanArpeggio = a; }
    void undoChangeSpanArpeggio(Arpeggio* a);

    TremoloType tremoloType() const;
    TremoloTwoChord* tremoloTwoChord() const;
    TremoloSingleChord* tremoloSingleChord() const;

    void setTremoloTwoChord(TremoloTwoChord* tr, bool applyLogic = true);
    void setTremoloSingleChord(TremoloSingleChord* tr);

    ChordLine* chordLine() const;
    bool endsNoteAnchoredLine() const { return m_endsNoteAnchoredLine; }
    void setEndsNoteAnchoredLine(bool val) { m_endsNoteAnchoredLine = val; }
    void updateEndsNoteAnchoredLine();
    StemSlash* stemSlash() const { return m_stemSlash; }
    bool showStemSlash() const { return m_showStemSlash; }
    void setShowStemSlashInAdvance();
    void requestShowStemSlash(bool show);
    void setShowStemSlash(bool show) { m_showStemSlash = show; }
    bool slash();
    void setSlash(bool flag, bool stemless);
    void removeMarkings(bool keepTremolo = false) override;

    const std::vector<Chord*>& graceNotes() const { return m_graceNotes; }
    std::vector<Chord*>& graceNotes() { return m_graceNotes; }
    std::vector<Chord*> allGraceChordsOfMainChord();

    GraceNotesGroup& graceNotesBefore(bool filterUnplayable = false) const;
    GraceNotesGroup& graceNotesAfter(bool filterUnplayable = false) const;

    size_t graceIndex() const { return m_graceIndex; }
    void setGraceIndex(size_t val) { m_graceIndex = val; }

    Chord* graceNoteAt(size_t idx) const;

    int line(bool up) const { return up ? upLine() : downLine(); }
    int line() const { return ldata()->up ? upLine() : downLine(); }
    int upLine() const;
    int downLine() const;
    PointF stemPos() const override;            ///< page coordinates
    PointF stemPosBeam() const override;        ///< page coordinates
    double stemPosX() const override;
    double rightEdge() const override;

    bool underBeam() const;
    Hook* hook() const { return m_hook; }
    void setHook(Hook* h) { m_hook = h; }

    //@ add an element to the Chord
    void add(EngravingItem*) override;
    //@ remove the element from the Chord
    void remove(EngravingItem*) override;

    Note* selectedNote() const;

    PointF pagePos() const override;        ///< position in page coordinates
    void cmdUpdateNotes(AccidentalState*, staff_idx_t staffIdx);

    NoteType noteType() const { return m_noteType; }
    void setNoteType(NoteType t);
    bool isGrace() const { return m_noteType != NoteType::NORMAL; }
    void toGraceAfter();

    bool isPreBendOrGraceBendStart() const;
    bool isGraceBendEnd() const;
    bool preOrGraceBendSpacingExceptionInTab() const;

    bool isTrillCueNote() const { return m_isTrillCueNote; }
    void setIsTrillCueNote(bool v);

    void setTrack(track_idx_t val) override;

    double dotPosX() const { return m_dotPosX; }
    void setDotPosX(double x) { m_dotPosX = x; }

    bool noStem() const { return m_noStem; }
    void setNoStem(bool val) { m_noStem = val; }

    double spaceLw() { return m_spaceLw; }
    void setSpaceLw(double lw) { m_spaceLw = lw; }
    double spaceRw() { return m_spaceRw; }
    void setSpaceRw(double rw) { m_spaceRw = rw; }

    AutoOnOff combineVoice() const { return m_combineVoice; }
    void setCombineVoice(AutoOnOff v) { m_combineVoice = v; }
    bool shouldCombineVoice() const;
    static bool combineVoice(const Chord* chord1, const Chord* chord2);

    PlayEventType playEventType() const { return m_playEventType; }
    void setPlayEventType(PlayEventType v) { m_playEventType = v; }
    std::vector<NoteEventList> getNoteEventLists();
    void setNoteEventLists(std::vector<NoteEventList>& nel);

    TremoloChordType tremoloChordType() const;

    std::vector<Articulation*>& articulations() { return m_articulations; }
    const std::vector<Articulation*>& articulations() const { return m_articulations; }
    std::set<SymId> articulationSymbolIds() const;
    Articulation* hasArticulation(const Articulation*);
    bool hasSingleArticulation() const { return m_articulations.size() == 1; }

    void updateArticulations(const std::set<SymId>& newArticulationIds,
                             ArticulationsUpdateMode replaceMode = ArticulationsUpdateMode::Insert);

    void localSpatiumChanged(double oldValue, double newValue) override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void reset() override;

    Segment* segment() const override;
    Measure* measure() const override;

    void sortNotes();

    Chord* nextTiedChord(bool backwards = false, bool sameSize = true) const;

    Fraction endTickIncludingTied() const;

    EngravingItem* nextElement() override;
    EngravingItem* prevElement() override;
    EngravingItem* nextSegmentElement() override;
    EngravingItem* lastElementBeforeSegment();
    EngravingItem* prevSegmentElement() override;

    String accessibleExtraInfo() const override;

    Note* firstGraceOrNote();

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    void undoChangeProperty(Pid id, const PropertyValue& newValue);
    void undoChangeProperty(Pid id, const PropertyValue& newValue, PropertyFlags ps) override;

    void styleChanged() override;

    bool allowKerningAbove() const { return m_allowKerningAbove; }
    bool allowKerningBelow() const { return m_allowKerningBelow; }
    void computeKerningExceptions();

    Ornament* findOrnament(bool forPlayback = false) const;

    const std::set<Spanner*>& startingSpanners() const { return m_startingSpanners; }
    const std::set<Spanner*>& endingSpanners() const { return m_endingSpanners; }
    void addStartingSpanner(Spanner* spanner) { m_startingSpanners.insert(spanner); }
    void removeStartingSpanner(Spanner* spanner) { m_startingSpanners.erase(spanner); }
    void addEndingSpanner(Spanner* spanner) { m_endingSpanners.insert(spanner); }
    void removeEndingSpanner(Spanner* spanner) { m_endingSpanners.erase(spanner); }

    bool shouldHaveStem() const;
    bool shouldHaveHook() const;
    void createStem();
    void removeStem();
    void createHook();

    double upPos()   const override;
    double downPos() const override;
    double centerX() const;

    double calcDefaultStemLength();

    struct StartEndSlurs {
        bool startUp = false;
        bool startDown = false;
        bool endUp = false;
        bool endDown = false;
        void reset()
        {
            startUp = false;
            startDown = false;
            endUp = false;
            endDown = false;
        }
    };

    StartEndSlurs& startEndSlurs() { return m_startEndSlurs; }

private:

    friend class Factory;

    Chord(Segment* parent = 0);
    Chord(const Chord&, bool link = false);

    int stemLengthBeamAddition() const;
    int maxReduction(int extensionOutsideStaff) const;
    int stemOpticalAdjustment(int stemEndPosition) const;
    int calcMinStemLength();
    int calc4BeamsException(int stemLength) const;

    // `includeTemporarySiblings`: whether items that are deleted & recreated during every layout should also be processed
    void processSiblings(std::function<void(EngravingItem*)> func, bool includeTemporarySiblings) const;

    std::vector<Note*> m_notes;           // sorted to decreasing line step
    std::vector<LedgerLine*> m_ledgerLines;

    Stem* m_stem = nullptr;
    Hook* m_hook = nullptr;
    StemSlash* m_stemSlash = nullptr;     // for grace notes
    bool m_showStemSlash = false;

    Arpeggio* m_arpeggio = nullptr;       // arpeggio which starts on the chord
    Arpeggio* m_spanArpeggio = nullptr;   // arpeggio which spans over this chord

    TremoloTwoChord* m_tremoloTwoChord = nullptr;
    TremoloSingleChord* m_tremoloSingleChord = nullptr;

    bool m_endsNoteAnchoredLine = false;        // true if this chord is the ending point of a glissando (needed for layout)
    std::vector<Chord*> m_graceNotes;    // storage for all grace notes
    mutable GraceNotesGroup m_graceNotesBefore = GraceNotesGroup(this); // will store before-chord grace notes
    mutable GraceNotesGroup m_graceNotesAfter = GraceNotesGroup(this); // will store after-chord grace notes
    size_t m_graceIndex = 0;             // if this is a grace note, index in parent list

    bool m_isTrillCueNote = false;

    DirectionV m_stemDirection = DirectionV::AUTO;
    NoteType m_noteType = NoteType::NORMAL; // mark grace notes: acciaccatura and appoggiatura
    bool m_noStem = false;
    PlayEventType m_playEventType = PlayEventType::Auto; // play events were modified by user

    double m_spaceLw = 0.0;
    double m_spaceRw = 0.0;

    double m_defaultStemLength = 0.0;
    double m_minStemLength = 0.0;

    bool m_isUiItem = false;

    double m_dotPosX = 0.0;

    StartEndSlurs m_startEndSlurs;

    std::set<Spanner*> m_startingSpanners; // spanners starting on this item
    std::set<Spanner*> m_endingSpanners;   // spanners ending on this item

    bool m_allowKerningAbove = true;
    bool m_allowKerningBelow = true;

    AutoOnOff m_combineVoice = AutoOnOff::AUTO;

    std::vector<Articulation*> m_articulations;
};
} // namespace mu::engraving
#endif
