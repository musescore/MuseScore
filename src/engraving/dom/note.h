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

#pragma once

/**
 \file
 Definition of classes Note and NoteHead.
*/

#include "containers.h"

#include "engravingitem.h"

#include "mscore.h"
#include "noteevent.h"
#include "pitchspelling.h"
#include "symbol.h"
#include "tie.h"
#include "tiejumppointlist.h"
#include "types.h"
#include "noteval.h"

namespace mu::engraving {
class Factory;
class Tie;
class Chord;
class Text;
class Score;
class AccidentalState;
class Accidental;
class NoteDot;
class Spanner;
class StaffType;
class StretchedBend;
class NoteEditData;
enum class AccidentalType : unsigned char;

static constexpr int MAX_DOTS = 4;

//--------------------------------------------------------------------------------
// LINE ATTACHMENT POINT
// Represents the attachment point of any line (tie, slur, glissando...)
// with respect to the note. Each note can hold a vector of line attach points, which
// it may use to make spacing decision with the surrounding items.
//--------------------------------------------------------------------------------
class LineAttachPoint
{
public:
    LineAttachPoint(EngravingItem* l, double x, double y, bool start)
        : m_line(l), m_pos(PointF(x, y)), m_start(start) {}

    const EngravingItem* line() const { return m_line; }
    const PointF pos() const { return m_pos; }
    bool start() const { return m_start; }

private:
    EngravingItem* m_line = nullptr;
    PointF m_pos = PointF(0.0, 0.0);
    bool m_start = true;
};

//---------------------------------------------------------
//   @@ NoteHead
//---------------------------------------------------------

class NoteHead final : public Symbol
{
    OBJECT_ALLOCATOR(engraving, NoteHead)
    DECLARE_CLASSOF(ElementType::NOTEHEAD)

public:
    NoteHead(Note* parent = 0);
    NoteHead(const NoteHead&) = default;
    NoteHead& operator=(const NoteHead&) = delete;
    NoteHead* clone() const override { return new NoteHead(*this); }

    NoteHeadGroup headGroup() const;
};

static const int INVALID_LINE = -10000;

//---------------------------------------------------------------------------------------
//   @@ Note
///    Graphic representation of a note.
//
//   @P accidental       Accidental       note accidental (null if none)
//   @P accidentalType   int              note accidental type
//   @P dots             array[NoteDot]   list of note dots (some can be null, read only)
//   @P dotsCount        int              number of note dots (read only)
//   @P elements         array[EngravingItem]   list of elements attached to notehead
//   @P fret             int              fret number in tablature
//   @P ghost            bool             ghost note (guitar: death note)
//   @P headScheme       enum (NoteHeadScheme.HEAD_AUTO, .HEAD_NORMAL, .HEAD_PITCHNAME, .HEAD_PITCHNAME_GERMAN, .HEAD_SHAPE_NOTE_4, .HEAD_SHAPE_NOTE_7_AIKIN, .HEAD_SHAPE_NOTE_7_FUNK, .HEAD_SHAPE_NOTE_7_WALKER, .HEAD_SOLFEGE, .HEAD_SOLFEGE_FIXED)
//   @P headGroup        enum (NoteHeadGroup.HEAD_NORMAL, .HEAD_BREVIS_ALT, .HEAD_CROSS, .HEAD_DIAMOND, .HEAD_DO, .HEAD_FA, .HEAD_LA, .HEAD_MI, .HEAD_RE, .HEAD_SLASH, .HEAD_LARGE_DIAMOND, .HEAD_SOL, .HEAD_TI, .HEAD_XCIRCLE, .HEAD_TRIANGLE)
//   @P headType         enum (NoteHeadType.HEAD_AUTO, .HEAD_BREVIS, .HEAD_HALF, .HEAD_QUARTER, .HEAD_WHOLE)
//   @P hidden           bool             hidden, not played note (read only)
//   @P line             int              notehead position (read only)
//   @P mirror           bool             mirror notehead on x axis (read only)
//   @P pitch            int              midi pitch
//   @P play             bool             play note
//   @P ppitch           int              actual played midi pitch (honoring ottavas) (read only)
//   @P isSmall          bool             small notehead
//   @P string           int              string number in tablature
//   @P subchannel       int              midi subchannel (for midi articulation) (read only)
//   @P tieBack          Tie              note backward tie (null if none, read only)
//   @P tieFor           Tie              note forward tie (null if none, read only)
//   @P tpc              int              tonal pitch class, as per concert pitch setting
//   @P tpc1             int              tonal pitch class, non transposed
//   @P tpc2             int              tonal pitch class, transposed
//   @P tuning           float            tuning offset in cent
//   @P userDotPosition  enum (Direction.AUTO, Direction.DOWN, Direction.UP)
//   @P userMirror       enum (DirectionH.AUTO, DirectionH.LEFT, DirectionH.RIGHT)
//   @P veloOffset       int
//   @P veloType         enum (Note.OFFSET_VAL, Note.USER_VAL)
//---------------------------------------------------------------------------------------

class Note final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Note)
    DECLARE_CLASSOF(ElementType::NOTE)

public:
    enum class SlideType : unsigned char {
        Undefined = 0,
        UpToNote,
        DownToNote,
        UpFromNote,
        DownFromNote
    };

    enum DisplayFretOption : signed char {
        Hide = -1,
        NoHarmonic,
        NaturalHarmonic,
        ArtificialHarmonic
    };

    ~Note();

    std::vector<Note*> compoundNotes() const;

    Note& operator=(const Note&) = delete;
    virtual Note* clone() const override { return new Note(*this, false); }

    Chord* chord() const { return (Chord*)explicitParent(); }
    void setParent(Chord* ch);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    void undoUnlink() override;

    double mag() const override;
    EngravingItem* elementBase() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all = true) override;
    void setTrack(track_idx_t val) override;

    int playTicks() const;
    Fraction playTicksFraction() const;

    double headWidth() const;
    double headHeight() const;
    double tabHeadWidth(const StaffType* tab = 0) const;
    double tabHeadHeight(const StaffType* tab = 0) const;
    PointF stemDownNW() const;
    PointF stemUpSE() const;
    double bboxXShift() const;
    double noteheadCenterX() const;
    double bboxRightPos() const;
    double headBodyWidth() const;
    double outsideTieAttachX(bool up) const;

    NoteHeadScheme headScheme() const { return m_headScheme; }
    void updateHeadGroup(const NoteHeadGroup headGroup);
    NoteHeadGroup headGroup() const { return m_headGroup; }
    NoteHeadType headType() const { return m_headType; }
    void setHeadScheme(NoteHeadScheme val);
    void setHeadGroup(NoteHeadGroup val);
    void setHeadType(NoteHeadType t);

    int subtype() const override { return int(m_headGroup); }
    TranslatableString subtypeUserName() const override;

    void setPitch(int val, bool notifyAboutChanged = true);
    void setPitch(int pitch, int tpc1, int tpc2);
    int pitch() const { return m_pitch; }
    int ottaveCapoFret() const;
    int linkedOttavaPitchOffset() const;
    int ppitch() const;             // playback pitch
    int epitch() const;             // effective pitch
    int octave() const;
    int playingOctave() const;
    double tuning() const { return m_tuning; }
    void setTuning(double v) { m_tuning = v; }
    double playingTuning() const;
    void undoSetTpc(int v);
    int transposition() const;
    bool fixed() const { return m_fixed; }
    void setFixed(bool v) { m_fixed = v; }
    int fixedLine() const { return m_fixedLine; }
    void setFixedLine(int v) { m_fixedLine = v; }

    int tpc() const;
    int tpc1() const { return m_tpc[0]; }                  // non transposed tpc
    int tpc2() const { return m_tpc[1]; }                  // transposed tpc
    String tpcUserName(bool explicitAccidental = false, bool full = false) const;

    void setTpc(int v);
    void setTpc1(int v) { m_tpc[0] = v; }
    void setTpc2(int v) { m_tpc[1] = v; }
    void setTpcFromPitch(Prefer prefer = Prefer::NEAREST);
    int tpc1default(int pitch) const;
    int tpc2default(int pitch) const;
    int transposeTpc(int tpc) const;

    int playingTpc() const;

    Accidental* accidental() const { return m_accidental; }
    void setAccidental(Accidental* a) { m_accidental = a; }

    AccidentalType accidentalType() const;
    void setAccidentalType(AccidentalType type);

    int line() const;
    void setLine(int n) { m_line = n; }

    int fret() const { return m_fret; }
    void setFret(int val) { m_fret = val; }
    float harmonicFret() const { return m_harmonicFret; }
    void setHarmonicFret(float val) { m_harmonicFret = val; }
    DisplayFretOption displayFret() const { return m_displayFret; }
    void setDisplayFret(DisplayFretOption val) { m_displayFret = val; }
    String fretString() const { return m_fretString; }
    void setFretString(const String& s) { m_fretString = s; }
    bool negativeFretUsed() const;
    int string() const { return m_string; }
    void setString(int val) { m_string = val; }
    int stringOrLine() const;

    bool ghost() const { return m_ghost; }
    void setGhost(bool val) { m_ghost = val; }
    bool deadNote() const { return m_deadNote; }
    void setDeadNote(bool deadNote) { m_deadNote = deadNote; }

    bool fretConflict() const { return m_fretConflict; }
    void setFretConflict(bool val) { m_fretConflict = val; }

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val);

    bool play() const { return m_play; }
    void setPlay(bool val) { m_play = val; }

    GuitarBend* bendFor() const;
    GuitarBend* bendBack() const;
    Tie* tieFor() const { return m_tieFor; }
    Tie* tieBack() const { return m_tieBack; }
    Tie* tieForNonPartial() const;
    Tie* tieBackNonPartial() const;
    LaissezVib* laissezVib() const;
    PartialTie* incomingPartialTie() const;
    PartialTie* outgoingPartialTie() const;
    void setTieFor(Tie* t);
    void setTieBack(Tie* t);
    Note* firstTiedNote(bool ignorePlayback = true) const;
    Note* lastTiedNote(bool ignorePlayback = true) const;

    int unisonIndex() const;
    void disconnectTiedNotes();
    void connectTiedNotes();

    void setupAfterRead(const Fraction& tick, bool pasteMode);

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool hidden() const { return m_hidden; }
    void setHidden(bool val) { m_hidden = val; }
    bool dotsHidden() const { return m_dotsHidden; }
    void setDotsHidden(bool val) { m_dotsHidden = val; }

    NoteType noteType() const;
    String  noteTypeUserName() const;

    ElementList& el() { return m_el; }
    const ElementList& el() const { return m_el; }

    int subchannel() const { return m_subchannel; }
    void setSubchannel(int val) { m_subchannel = val; }

    DirectionH userMirror() const { return m_userMirror; }
    void setUserMirror(DirectionH d) { m_userMirror = d; }

    DirectionV userDotPosition() const { return m_userDotPosition; }
    void setUserDotPosition(DirectionV d) { m_userDotPosition = d; }
    DirectionV dotPosition() const { return m_dotPosition; }
    void setDotPosition(DirectionV d) { m_dotPosition = d; }
    bool dotIsUp() const;                 // actual dot position

    void reset() override;

    float userVelocityFraction() const;
    int userVelocity() const { return m_userVelocity; }
    void setUserVelocity(int v) { m_userVelocity = v; }

    void setOnTimeOffset(int v);
    void setOffTimeOffset(int v);

    int customizeVelocity(int velo) const;
    NoteDot* dot(int n) { return m_dots.at(n); }
    const std::vector<NoteDot*>& dots() const { return m_dots; }
    std::vector<NoteDot*>& dots() { return m_dots; }

    int qmlDotsCount();
    void updateAccidental(AccidentalState*);
    void updateLine();
    void setNval(const NoteVal&, Fraction = { -1, 1 });
    NoteEventList& playEvents() { return m_playEvents; }
    const NoteEventList& playEvents() const { return m_playEvents; }
    NoteEvent* noteEvent(int idx) { return &m_playEvents[idx]; }
    void setPlayEvents(const NoteEventList& l) { m_playEvents = l; }

    const std::vector<Spanner*>& spannerFor() const { return m_spannerFor; }
    const std::vector<Spanner*>& spannerBack() const { return m_spannerBack; }

    void addSpannerBack(Spanner* e)
    {
        if (!muse::contains(m_spannerBack, e)) {
            m_spannerBack.push_back(e);
        }
    }

    bool removeSpannerBack(Spanner* e) { return muse::remove(m_spannerBack, e); }
    void addSpannerFor(Spanner* e)
    {
        if (!muse::contains(m_spannerFor, e)) {
            m_spannerFor.push_back(e);
        }
    }

    bool removeSpannerFor(Spanner* e) { return muse::remove(m_spannerFor, e); }

    void transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals);

    void localSpatiumChanged(double oldValue, double newValue) override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool mark() const { return m_mark; }
    void setMark(bool v) const { m_mark = v; }
    void setScore(Score* s) override;
    void setDotRelativeLine(int);

    void setHeadHasParentheses(bool hasParentheses, bool addToLinked = true, bool generated = false);
    bool headHasParentheses() const { return m_hasUserParentheses; }

    static SymId noteHead(int direction, NoteHeadGroup, NoteHeadType, int tpc, Key key, NoteHeadScheme scheme);
    static SymId noteHead(int direction, NoteHeadGroup, NoteHeadType);
    NoteVal noteVal() const;

    EngravingItem* nextInEl(EngravingItem* e);
    EngravingItem* prevInEl(EngravingItem* e);
    EngravingItem* nextElement() override;
    EngravingItem* prevElement() override;
    virtual EngravingItem* lastElementBeforeSegment();
    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    String accessibleInfo() const override;
    String screenReaderInfo() const override;
    String accessibleExtraInfo() const override;

    std::vector<Note*> tiedNotes() const;

    void setOffTimeType(int v) { m_offTimeType = v; }
    void setOnTimeType(int v) { m_onTimeType = v; }
    int offTimeType() const { return m_offTimeType; }
    int onTimeType() const { return m_onTimeType; }

    void attachSlide(SlideType slideType);

    bool hasSlideToNote() const;
    bool hasSlideFromNote() const;
    SlideType slideToType() const { return m_slideToType; }
    SlideType slideFromType() const { return m_slideFromType; }

    void setStretchedBend(StretchedBend* s) { m_stretchedBend = s; }
    StretchedBend* stretchedBend() const { return m_stretchedBend; }

    void setHarmonic(bool val) { m_harmonic = val; }
    bool harmonic() const { return m_harmonic; }

    bool isGrace() const { return noteType() != NoteType::NORMAL; }

    bool isPreBendStart() const;
    bool isGraceBendStart() const;
    bool isContinuationOfBend() const;

    bool hasAnotherStraightAboveOrBelow(bool above) const;

    std::vector<LineAttachPoint>& lineAttachPoints() { return m_lineAttachPoints; }
    const std::vector<LineAttachPoint>& lineAttachPoints() const { return m_lineAttachPoints; }
    void addStartLineAttachPoint(PointF point, EngravingItem* line) { addLineAttachPoint(point, line, true); }
    void addEndLineAttachPoint(PointF point, EngravingItem* line) { addLineAttachPoint(point, line, false); }

    PointF posInStaffCoordinates();

    bool isTrillCueNote() const { return m_isTrillCueNote; }
    void setIsTrillCueNote(bool v);

    SymId noteHead() const;
    bool isNoteName() const;

    void updateFrettingForTiesAndBends();
    bool shouldHideFret() const;
    bool shouldForceShowFret() const;

    void setVisible(bool v) override;

    TieJumpPointList* tieJumpPoints() { return &m_jumpPoints; }
    const TieJumpPointList* tieJumpPoints() const { return &m_jumpPoints; }

    struct LayoutData : public EngravingItem::LayoutData {
        ld_field<bool> useTablature = { "[Note] useTablature", false };
        ld_field<SymId> cachedNoteheadSym = { "[Note] cachedNoteheadSym", SymId::noSym };    // use in draw to avoid recomputing at every update
        ld_field<SymId> cachedSymNull = { "[Note] cachedSymNull", SymId::noSym };            // additional symbol for some transparent notehead
        ld_field<bool> mirror = { "[Note] mirror", false };                                  // True if note is mirrored at stem.
    };
    DECLARE_LAYOUTDATA_METHODS(Note)

private:

    friend class Factory;
    Note(Chord* ch = 0);
    Note(const Note&, bool link = false);

    void startDrag(EditData&) override;
    RectF drag(EditData& ed) override;
    void endDrag(EditData&) override;
    void editDrag(EditData& editData) override;

    void verticalDrag(EditData& ed);
    void horizontalDrag(EditData& ed);

    void addSpanner(Spanner*);
    void removeSpanner(Spanner*);
    int concertPitchIdx() const;
    void updateRelLine(int absLine, bool undoable);

    static std::vector<Note*> findTiedNotes(Note* startNote, bool followPartialTies = true);

    void normalizeLeftDragDelta(Segment* seg, EditData& ed, NoteEditData* ned);

    static String tpcUserName(int tpc, int pitch, bool explicitAccidental, bool full = false);

    void getNoteListForDots(std::vector<Note*>& topDownNotes, std::vector<Note*>& bottomUpNotes, std::vector<int>& anchoredDots);

    void addLineAttachPoint(PointF point, EngravingItem* line, bool start);

    bool m_ghost = false;        // ghost note
    bool m_deadNote = false;     // dead note

    bool m_isTrillCueNote = false;

    bool m_hidden = false;                 // marks this note as the hidden one if there are
                                           // overlapping notes; hidden notes are not played
                                           // and heads + accidentals are not shown
    bool m_dotsHidden = false;        // dots of hidden notes are hidden too
                                      // except if only one note is dotted
    bool m_fretConflict = false;      // used by TAB staves to mark a fretting conflict:
                                      // two or more notes on the same string
    bool m_dragMode = false;
    bool m_isSmall = false;
    bool m_play = true;           // note is not played if false
    mutable bool m_mark = false;  // for use in sequencer
    bool m_fixed = false;         // for slash notation
    StretchedBend* m_stretchedBend = nullptr;
    SlideType m_slideToType = SlideType::Undefined;
    SlideType m_slideFromType = SlideType::Undefined;

    DirectionH m_userMirror = DirectionH::AUTO;        ///< user override of mirror
    DirectionV m_userDotPosition = DirectionV::AUTO;   ///< user override of dot position
    DirectionV m_dotPosition = DirectionV::AUTO;       // used as an intermediate step when resolving dot conflicts

    NoteHeadScheme m_headScheme = NoteHeadScheme::HEAD_AUTO;
    NoteHeadGroup m_headGroup = NoteHeadGroup::HEAD_NORMAL;
    NoteHeadType m_headType = NoteHeadType::HEAD_AUTO;

    VeloType m_veloType = VeloType::USER_VAL;

    int m_offTimeType = 0;     // compatibility only 1 - user(absolute), 2 - offset (%)
    int m_onTimeType = 0;      // compatibility only 1 - user, 2 - offset

    int m_subchannel = 0;       // articulation
    int m_line = INVALID_LINE;  // y-Position; 0 - top line.
    int m_fret = -1;            // for tablature view
    float m_harmonicFret = -1.0;
    DisplayFretOption m_displayFret = DisplayFretOption::NoHarmonic;
    int m_string = -1;
    mutable int m_tpc[2] = { Tpc::TPC_INVALID, Tpc::TPC_INVALID };   // tonal pitch class  (concert/transposing)
    mutable int m_pitch = 0;      // Note pitch as midi value (0 - 127).

    int m_userVelocity = 0;     // velocity user offset in percent, or absolute velocity for this note
    int m_fixedLine = 0;        // fixed line number if _fixed == true
    double m_tuning = 0.0;      // pitch offset in cent, playable only by internal synthesizer

    Accidental* m_accidental = nullptr;

    Tie* m_tieFor = nullptr;
    Tie* m_tieBack = nullptr;

    Symbol* m_leftParenthesis = nullptr;
    Symbol* m_rightParenthesis = nullptr;
    bool m_hasUserParentheses = false;
    bool m_hasGeneratedParens = false;

    bool m_harmonic = false;

    ElementList m_el;          // fingering, other text, symbols or images
    std::vector<NoteDot*> m_dots;
    NoteEventList m_playEvents;
    std::vector<Spanner*> m_spannerFor;
    std::vector<Spanner*> m_spannerBack;

    String m_fretString;

    std::vector<LineAttachPoint> m_lineAttachPoints;
    TieJumpPointList m_jumpPoints { this };
};
} // namespace mu::engraving
