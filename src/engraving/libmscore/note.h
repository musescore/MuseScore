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

#ifndef __NOTE_H__
#define __NOTE_H__

/**
 \file
 Definition of classes Note and NoteHead.
*/

#include "containers.h"

#include "engravingitem.h"

#include "noteevent.h"
#include "pitchspelling.h"
#include "shape.h"
#include "symbol.h"
#include "types.h"

namespace mu::engraving {
class Factory;
class Tie;
class Chord;
class Text;
class Score;
class Bend;
class AccidentalState;
class Accidental;
class NoteDot;
class Spanner;
class StaffType;
class NoteEditData;
enum class AccidentalType;

static constexpr int MAX_DOTS = 4;

//--------------------------------------------------------------------------------
// LINE ATTACHMENT POINT
// Represents the attachment point of any line (tie, slur, glissando...)
// with respect to the note. Each note can hold a vector of line attach points, which
// it may use to make spacing decision with the surrounding items.
//--------------------------------------------------------------------------------
class LineAttachPoint
{
private:
    EngravingItem* _line = nullptr;
    PointF _pos = PointF(0.0, 0.0);

public:
    LineAttachPoint(EngravingItem* l, double x, double y)
        : _line(l), _pos(PointF(x, y)) {}

    const EngravingItem* line() const { return _line; }
    const PointF pos() const { return _pos; }
};

//---------------------------------------------------------
//   @@ NoteHead
//---------------------------------------------------------

class NoteHead final : public Symbol
{
    OBJECT_ALLOCATOR(engraving, NoteHead)
public:

    NoteHead(Note* parent = 0);
    NoteHead(const NoteHead&) = default;
    NoteHead& operator=(const NoteHead&) = delete;
    NoteHead* clone() const override { return new NoteHead(*this); }

    NoteHeadGroup headGroup() const;
};

//---------------------------------------------------------
//   NoteVal
///    helper structure
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

struct NoteVal {
    int pitch                 { -1 };
    int tpc1                  { Tpc::TPC_INVALID };
    int tpc2                  { Tpc::TPC_INVALID };
    int fret                  { INVALID_FRET_INDEX };
    int string                { INVALID_STRING_INDEX };
    NoteHeadGroup headGroup { NoteHeadGroup::HEAD_NORMAL };

    NoteVal() {}
    NoteVal(int p)
        : pitch(p) {}
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
    enum class SlideType {
        Undefined = 0,
        Shift, // connects 2 notes
        Legato, // connects 2 notes and adds a slur
        Plop, // from up to note
        Lift, // from down to note
        Doit, // from note to up
        Fall, // from note to down
    };

    struct Slide {
        SlideType type { SlideType::Undefined };
        Note* startNote = nullptr;   // note to start slide (for 2 notes slides)
        Note* endNote = nullptr;     // note to end slide (for 2 notes slides)
        bool isValid() const { return type != SlideType::Undefined; }
        bool is(SlideType t) const { return t == type; }
    };

    enum DisplayFretOption {
        Hide = -1,
        NoHarmonic,
        NaturalHarmonic,
        ArtificialHarmonic
    };

private:
    bool _ghost = false;        ///< ghost note
    bool _deadNote = false;     ///< dead note

    bool _hidden = false;                 ///< marks this note as the hidden one if there are
                                          ///< overlapping notes; hidden notes are not played
                                          ///< and heads + accidentals are not shown
    bool _dotsHidden = false;        ///< dots of hidden notes are hidden too
                                     ///< except if only one note is dotted
    bool _fretConflict = false;      ///< used by TAB staves to mark a fretting conflict:
                                     ///< two or more notes on the same string
    bool dragMode = false;
    bool _mirror = false;        ///< True if note is mirrored at stem.
    bool m_isSmall = false;
    bool _play = true;           ///< note is not played if false
    mutable bool _mark = false;  ///< for use in sequencer
    bool _fixed = false;         ///< for slash notation
    StretchedBend* m_bend = nullptr;

    DirectionH _userMirror = DirectionH::AUTO;        ///< user override of mirror
    DirectionV _userDotPosition = DirectionV::AUTO;   ///< user override of dot position

    NoteHeadScheme _headScheme = NoteHeadScheme::HEAD_AUTO;
    NoteHeadGroup _headGroup = NoteHeadGroup::HEAD_NORMAL;
    NoteHeadType _headType = NoteHeadType::HEAD_AUTO;

    VeloType _veloType = VeloType::USER_VAL;

    int _offTimeType = 0;     ///< compatibility only 1 - user(absolute), 2 - offset (%)
    int _onTimeType = 0;      ///< compatibility only 1 - user, 2 - offset

    int _subchannel = 0;       ///< articulation
    int _line = INVALID_LINE;  ///< y-Position; 0 - top line.
    int _fret = -1;            ///< for tablature view
    float m_harmonicFret = -1.0;
    DisplayFretOption m_displayFret = DisplayFretOption::NoHarmonic;
    int _string = -1;
    mutable int _tpc[2] = { Tpc::TPC_INVALID, Tpc::TPC_INVALID };   ///< tonal pitch class  (concert/transposing)
    mutable int _pitch = 0;      ///< Note pitch as midi value (0 - 127).

    int _userVelocity = 0;    ///< velocity user offset in percent, or absolute velocity for this note
    int _fixedLine = 0;     ///< fixed line number if _fixed == true
    double _tuning = 0.0;    ///< pitch offset in cent, playable only by internal synthesizer

    Accidental* _accidental = nullptr;

    Tie* _tieFor = nullptr;
    Tie* _tieBack = nullptr;

    Slide _attachedSlide;           ///< slide which starts from note
    Slide* _relatedSlide = nullptr; ///< slide which goes to note

    Symbol* _leftParenthesis = nullptr;
    Symbol* _rightParenthesis = nullptr;
    bool _hasHeadParentheses = false;

    bool _isHammerOn = false;
    bool _harmonic = false;

    ElementList _el;          ///< fingering, other text, symbols or images
    std::vector<NoteDot*> _dots;
    NoteEventList _playEvents;
    std::vector<Spanner*> _spannerFor;
    std::vector<Spanner*> _spannerBack;

    SymId _cachedNoteheadSym;   // use in draw to avoid recomputing at every update
    SymId _cachedSymNull;   // additional symbol for some transparent notehead

    String _fretString;

    friend class Factory;
    Note(Chord* ch = 0);
    Note(const Note&, bool link = false);

    void startDrag(EditData&) override;
    mu::RectF drag(EditData& ed) override;
    void endDrag(EditData&) override;
    void editDrag(EditData& editData) override;

    void verticalDrag(EditData& ed);
    void horizontalDrag(EditData& ed);

    void addSpanner(Spanner*);
    void removeSpanner(Spanner*);
    int concertPitchIdx() const;
    void updateRelLine(int relLine, bool undoable);
    bool isNoteName() const;
    SymId noteHead() const;

    void normalizeLeftDragDelta(Segment* seg, EditData& ed, NoteEditData* ned);

    static String tpcUserName(int tpc, int pitch, bool explicitAccidental, bool full = false);

    bool sameVoiceKerningLimited() const override { return true; }

    void getNoteListForDots(std::vector<Note*>& topDownNotes, std::vector<Note*>& bottomUpNotes, std::vector<int>& anchoredDots);

    std::vector<LineAttachPoint> _lineAttachPoints;

public:

    ~Note();

    std::vector<const Note*> compoundNotes() const;

    double computePadding(const EngravingItem* nextItem) const override;
    KerningType doComputeKerningType(const EngravingItem* nextItem) const override;

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

    void layout() override;
    void layout2();
    //setter is used only in drumset tools to setup the notehead preview in the drumset editor and the palette
    void setCachedNoteheadSym(SymId i) { _cachedNoteheadSym = i; }
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all = true) override;
    void setTrack(track_idx_t val) override;

    int playTicks() const;
    Fraction playTicksFraction() const;

    double headWidth() const;
    double headHeight() const;
    double tabHeadWidth(const StaffType* tab = 0) const;
    double tabHeadHeight(const StaffType* tab = 0) const;
    mu::PointF stemDownNW() const;
    mu::PointF stemUpSE() const;
    double bboxXShift() const;
    double noteheadCenterX() const;
    double bboxRightPos() const;
    double headBodyWidth() const;
    double outsideTieAttachX(bool up) const;

    NoteHeadScheme headScheme() const { return _headScheme; }
    void updateHeadGroup(const NoteHeadGroup headGroup);
    NoteHeadGroup headGroup() const { return _headGroup; }
    NoteHeadType headType() const { return _headType; }
    void setHeadScheme(NoteHeadScheme val);
    void setHeadGroup(NoteHeadGroup val);
    void setHeadType(NoteHeadType t);

    int subtype() const override { return int(_headGroup); }
    TranslatableString subtypeUserName() const override;

    void setPitch(int val, bool notifyAboutChanged = true);
    void setPitch(int pitch, int tpc1, int tpc2);
    int pitch() const { return _pitch; }
    int ottaveCapoFret() const;
    int ppitch() const;             ///< playback pitch
    int epitch() const;             ///< effective pitch
    int octave() const;
    int playingOctave() const;
    double tuning() const { return _tuning; }
    void setTuning(double v) { _tuning = v; }
    void undoSetTpc(int v);
    int transposition() const;
    bool fixed() const { return _fixed; }
    void setFixed(bool v) { _fixed = v; }
    int fixedLine() const { return _fixedLine; }
    void setFixedLine(int v) { _fixedLine = v; }

    int tpc() const;
    int tpc1() const { return _tpc[0]; }                  // non transposed tpc
    int tpc2() const { return _tpc[1]; }                  // transposed tpc
    String tpcUserName(bool explicitAccidental = false, bool full = false) const;

    void setTpc(int v);
    void setTpc1(int v) { _tpc[0] = v; }
    void setTpc2(int v) { _tpc[1] = v; }
    void setTpcFromPitch();
    int tpc1default(int pitch) const;
    int tpc2default(int pitch) const;
    int transposeTpc(int tpc) const;

    int playingTpc() const;

    Accidental* accidental() const { return _accidental; }
    void setAccidental(Accidental* a) { _accidental = a; }

    AccidentalType accidentalType() const;
    void setAccidentalType(AccidentalType type);

    int line() const;
    void setLine(int n) { _line = n; }

    int fret() const { return _fret; }
    void setFret(int val) { _fret = val; }
    float harmonicFret() const { return m_harmonicFret; }
    void setHarmonicFret(float val) { m_harmonicFret = val; }
    DisplayFretOption displayFret() const { return m_displayFret; }
    void setDisplayFret(DisplayFretOption val) { m_displayFret = val; }
    bool negativeFretUsed() const;
    int string() const { return _string; }
    void setString(int val);
    bool ghost() const { return _ghost; }
    void setGhost(bool val) { _ghost = val; }
    bool deadNote() const { return _deadNote; }
    void setDeadNote(bool deadNote) { _deadNote = deadNote; }

    bool fretConflict() const { return _fretConflict; }
    void setFretConflict(bool val) { _fretConflict = val; }

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    bool mirror() const { return _mirror; }
    void setMirror(bool val) { _mirror = val; }

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val);

    bool play() const { return _play; }
    void setPlay(bool val) { _play = val; }

    Tie* tieFor() const { return _tieFor; }
    Tie* tieBack() const { return _tieBack; }
    void setTieFor(Tie* t) { _tieFor = t; }
    void setTieBack(Tie* t) { _tieBack = t; }
    Note* firstTiedNote() const;
    const Note* lastTiedNote() const;
    Note* lastTiedNote() { return const_cast<Note*>(static_cast<const Note*>(this)->lastTiedNote()); }
    int unisonIndex() const;
    void disconnectTiedNotes();
    void connectTiedNotes();

    void draw(mu::draw::Painter*) const override;

    void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;
    void setupAfterRead(const Fraction& tick, bool pasteMode);
    void write(XmlWriter&) const override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool hidden() const { return _hidden; }
    void setHidden(bool val) { _hidden = val; }
    bool dotsHidden() const { return _dotsHidden; }
    void setDotsHidden(bool val) { _dotsHidden = val; }

    NoteType noteType() const;
    String  noteTypeUserName() const;

    ElementList& el() { return _el; }
    const ElementList& el() const { return _el; }

    int subchannel() const { return _subchannel; }
    void setSubchannel(int val) { _subchannel = val; }

    DirectionH userMirror() const { return _userMirror; }
    void setUserMirror(DirectionH d) { _userMirror = d; }

    DirectionV userDotPosition() const { return _userDotPosition; }
    void setUserDotPosition(DirectionV d) { _userDotPosition = d; }
    bool dotIsUp() const;                 // actual dot position

    void reset() override;

    float userVelocityFraction() const;
    int userVelocity() const { return _userVelocity; }
    void setUserVelocity(int v) { _userVelocity = v; }

    void setOnTimeOffset(int v);
    void setOffTimeOffset(int v);

    int customizeVelocity(int velo) const;
    NoteDot* dot(int n) { return _dots[n]; }
    const std::vector<NoteDot*>& dots() const { return _dots; }
    std::vector<NoteDot*>& dots() { return _dots; }

    int qmlDotsCount();
    void updateAccidental(AccidentalState*);
    void updateLine();
    void setNval(const NoteVal&, Fraction = { -1, 1 });
    NoteEventList& playEvents() { return _playEvents; }
    const NoteEventList& playEvents() const { return _playEvents; }
    NoteEvent* noteEvent(int idx) { return &_playEvents[idx]; }
    void setPlayEvents(const NoteEventList& l) { _playEvents = l; }

    const std::vector<Spanner*>& spannerFor() const { return _spannerFor; }
    const std::vector<Spanner*>& spannerBack() const { return _spannerBack; }

    void addSpannerBack(Spanner* e)
    {
        if (!mu::contains(_spannerBack, e)) {
            _spannerBack.push_back(e);
        }
    }

    bool removeSpannerBack(Spanner* e) { return mu::remove(_spannerBack, e); }
    void addSpannerFor(Spanner* e)
    {
        if (!mu::contains(_spannerFor, e)) {
            _spannerFor.push_back(e);
        }
    }

    bool removeSpannerFor(Spanner* e) { return mu::remove(_spannerFor, e); }

    void transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals);

    void localSpatiumChanged(double oldValue, double newValue) override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool mark() const { return _mark; }
    void setMark(bool v) const { _mark = v; }
    void setScore(Score* s) override;
    void setDotY(DirectionV);

    void setHeadHasParentheses(bool hasParentheses);

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

    Shape shape() const override;
    std::vector<Note*> tiedNotes() const;

    void setOffTimeType(int v) { _offTimeType = v; }
    void setOnTimeType(int v) { _onTimeType = v; }
    int offTimeType() const { return _offTimeType; }
    int onTimeType() const { return _onTimeType; }

    const Slide& slide() const { return _attachedSlide; }

    void attachSlide(const Slide& s) { _attachedSlide = s; }
    void setRelatedSlide(Slide* pSlide) { _relatedSlide = pSlide; }

    bool hasRelatedSlide() const { return !!_relatedSlide; }
    const Slide& relatedSlide() const { return *_relatedSlide; }

    bool isSlideToNote() const;
    bool isSlideOutNote() const;

    bool isSlideStart() const;
    bool isSlideEnd() const;

    void relateSlide(Note& start) { _relatedSlide = &start._attachedSlide; }

    StretchedBend* bend() const { return m_bend; }

    bool isHammerOn() const { return _isHammerOn; }
    void setIsHammerOn(bool hammerOn) { _isHammerOn = hammerOn; }

    void setHarmonic(bool val) { _harmonic = val; }
    bool harmonic() const { return _harmonic; }

    bool isGrace() const { return noteType() != NoteType::NORMAL; }

    void addLineAttachPoint(mu::PointF point, EngravingItem* line);
    std::vector<LineAttachPoint>& lineAttachPoints() { return _lineAttachPoints; }
    const std::vector<LineAttachPoint>& lineAttachPoints() const { return _lineAttachPoints; }

    mu::PointF posInStaffCoordinates();
};
} // namespace mu::engraving
#endif
