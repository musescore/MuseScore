//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __NOTE_H__
#define __NOTE_H__

/**
 \file
 Definition of classes Note and NoteHead.
*/

#include "element.h"
#include "symbol.h"
#include "noteevent.h"
#include "pitchspelling.h"
#include "shape.h"
#include "key.h"
#include "sym.h"

namespace Ms {

class Tie;
class Chord;
class NoteEvent;
class Text;
class Score;
class Sym;
class MuseScoreView;
class Bend;
class AccidentalState;
class Accidental;
class NoteDot;
class Spanner;
class StaffType;
class NoteEditData;
enum class AccidentalType;

static const int MAX_DOTS = 4;

//---------------------------------------------------------
//   @@ NoteHead
//---------------------------------------------------------

class NoteHead final : public Symbol {
      Q_GADGET
   public:
      // keep in sync with noteHeadSchemeNames array in note.cpp
      enum class Scheme : signed char {
            ///.\{
            HEAD_AUTO = -1,
            HEAD_NORMAL,
            HEAD_PITCHNAME,
            HEAD_PITCHNAME_GERMAN,
            HEAD_SOLFEGE,
            HEAD_SOLFEGE_FIXED,
            HEAD_SHAPE_NOTE_4,
            HEAD_SHAPE_NOTE_7_AIKIN,
            HEAD_SHAPE_NOTE_7_FUNK,
            HEAD_SHAPE_NOTE_7_WALKER,
            HEAD_SCHEMES
            ///\}
            };
      // keep in sync with noteHeadGroupNames array in note.cpp
      enum class Group : signed char {
            ///.\{
            HEAD_NORMAL = 0,
            HEAD_CROSS,
            HEAD_PLUS,
            HEAD_XCIRCLE,
            HEAD_WITHX,
            HEAD_TRIANGLE_UP,
            HEAD_TRIANGLE_DOWN,
            HEAD_SLASHED1,
            HEAD_SLASHED2,
            HEAD_DIAMOND,
            HEAD_DIAMOND_OLD,
            HEAD_CIRCLED,
            HEAD_CIRCLED_LARGE,
            HEAD_LARGE_ARROW,
            HEAD_BREVIS_ALT,

            HEAD_SLASH,
            HEAD_LARGE_DIAMOND,

            HEAD_SOL,
            HEAD_LA,
            HEAD_FA,
            HEAD_MI,
            HEAD_DO,
            HEAD_RE,
            HEAD_TI,

            HEAD_HEAVY_CROSS,
            HEAD_HEAVY_CROSS_HAT,

            // not exposed from here
            HEAD_DO_WALKER,
            HEAD_RE_WALKER,
            HEAD_TI_WALKER,
            HEAD_DO_FUNK,
            HEAD_RE_FUNK,
            HEAD_TI_FUNK,

            HEAD_DO_NAME,
            HEAD_RE_NAME,
            HEAD_MI_NAME,
            HEAD_FA_NAME,
            HEAD_SOL_NAME,
            HEAD_LA_NAME,
            HEAD_TI_NAME,
            HEAD_SI_NAME,

            HEAD_A_SHARP,
            HEAD_A,
            HEAD_A_FLAT,
            HEAD_B_SHARP,
            HEAD_B,
            HEAD_B_FLAT,
            HEAD_C_SHARP,
            HEAD_C,
            HEAD_C_FLAT,
            HEAD_D_SHARP,
            HEAD_D,
            HEAD_D_FLAT,
            HEAD_E_SHARP,
            HEAD_E,
            HEAD_E_FLAT,
            HEAD_F_SHARP,
            HEAD_F,
            HEAD_F_FLAT,
            HEAD_G_SHARP,
            HEAD_G,
            HEAD_G_FLAT,
            HEAD_H,
            HEAD_H_SHARP,

            HEAD_SWISS_RUDIMENTS_FLAM,
            HEAD_SWISS_RUDIMENTS_DOUBLE,

            HEAD_CUSTOM,
            HEAD_GROUPS,
            HEAD_INVALID = -1
            ///\}
            };
      // keep in sync with noteHeadTypeNames array in note.cpp
      enum class Type : signed char {
            ///.\{
            HEAD_AUTO    = -1,
            HEAD_WHOLE   = 0,
            HEAD_HALF    = 1,
            HEAD_QUARTER = 2,
            HEAD_BREVIS  = 3,
            HEAD_TYPES
            ///\}
            };

      Q_ENUM(Scheme);
      Q_ENUM(Group);
      Q_ENUM(Type);

      NoteHead(Score* s = 0) : Symbol(s) {}
      NoteHead(const NoteHead&) = default;
      NoteHead &operator=(const NoteHead&) = delete;
      NoteHead* clone() const override    { return new NoteHead(*this); }
      ElementType type() const override { return ElementType::NOTEHEAD; }

      Group headGroup() const;

      static QString scheme2userName(Scheme scheme);
      static QString group2userName(Group group);
      static QString type2userName(Type type);
      static QString scheme2name(Scheme scheme);
      static QString group2name(Group group);
      static QString type2name(Type type);
      static Scheme name2scheme(const QString& s);
      static Group name2group(const QString& s);
      static Type name2type(const QString& s);
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
      NoteHead::Group headGroup { NoteHead::Group::HEAD_NORMAL };

      NoteVal() {}
      NoteVal(int p) : pitch(p) {}
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
//   @P elements         array[Element]   list of elements attached to notehead
//   @P fret             int              fret number in tablature
//   @P ghost            bool             ghost note (guitar: death note)
//   @P headScheme       enum (NoteHeadScheme.HEAD_AUTO, .HEAD_NORMAL, .HEAD_PITCHNAME, .HEAD_PITCHNAME_GERMAN, .HEAD_SHAPE_NOTE_4, .HEAD_SHAPE_NOTE_7_AIKIN, .HEAD_SHAPE_NOTE_7_FUNK, .HEAD_SHAPE_NOTE_7_WALKER, .HEAD_SOLFEGE, .HEAD_SOLFEGE_FIXED)
//   @P headGroup        enum (NoteHeadGroup.HEAD_NORMAL, .HEAD_BREVIS_ALT, .HEAD_CROSS, .HEAD_DIAMOND, .HEAD_DO, .HEAD_FA, .HEAD_LA, .HEAD_MI, .HEAD_RE, .HEAD_SLASH, .HEAD_SOL, .HEAD_TI, .HEAD_XCIRCLE, .HEAD_TRIANGLE)
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

class Note final : public Element {
      Q_GADGET
   public:
      enum class ValueType : char { OFFSET_VAL, USER_VAL };
      Q_ENUM(ValueType);

   private:
      bool _ghost         { false };      ///< ghost note (guitar: death note)
      bool _hidden        { false };      ///< marks this note as the hidden one if there are
                                          ///< overlapping notes; hidden notes are not played
                                          ///< and heads + accidentals are not shown
      bool _dotsHidden    { false };      ///< dots of hidden notes are hidden too
                                          ///< except if only one note is dotted
      bool _fretConflict  { false };      ///< used by TAB staves to mark a fretting conflict:
                                          ///< two or more notes on the same string
      bool dragMode       { false };
      bool _mirror        { false };      ///< True if note is mirrored at stem.
      bool m_isSmall      { false };
      bool _play          { true  };      // note is not played if false
      mutable bool _mark  { false };      // for use in sequencer
      bool _fixed         { false };      // for slash notation

      MScore::DirectionH _userMirror { MScore::DirectionH::AUTO };      ///< user override of mirror
      Direction _userDotPosition     { Direction::AUTO };               ///< user override of dot position

      NoteHead::Scheme _headScheme { NoteHead::Scheme::HEAD_AUTO };
      NoteHead::Group _headGroup { NoteHead::Group::HEAD_NORMAL };
      NoteHead::Type  _headType  { NoteHead::Type::HEAD_AUTO    };

      ValueType _veloType { ValueType::OFFSET_VAL };

      char _offTimeType    { 0 };    // compatibility only 1 - user(absolute), 2 - offset (%)
      char _onTimeType     { 0 };    // compatibility only 1 - user, 2 - offset

      int _subchannel     { 0  };   ///< articulation
      int _line           { INVALID_LINE  };   ///< y-Position; 0 - top line.
      int _fret           { -1 };   ///< for tablature view
      int _string         { -1 };
      mutable int _tpc[2] { Tpc::TPC_INVALID, Tpc::TPC_INVALID }; ///< tonal pitch class  (concert/transposing)
      mutable int _pitch  { 0  };   ///< Note pitch as midi value (0 - 127).

      int _veloOffset     { 0 };    ///< velocity user offset in percent, or absolute velocity for this note
      int _fixedLine      { 0 };    // fixed line number if _fixed == true
      qreal _tuning       { 0.0 };  ///< pitch offset in cent, playable only by internal synthesizer

      Accidental* _accidental { 0 };

      Tie* _tieFor        { 0 };
      Tie* _tieBack       { 0 };

      ElementList _el;        ///< fingering, other text, symbols or images
      QVector<NoteDot*> _dots;
      NoteEventList _playEvents;
      QVector<Spanner*> _spannerFor;
      QVector<Spanner*> _spannerBack;

      SymId _cachedNoteheadSym; // use in draw to avoid recomputing at every update
      SymId _cachedSymNull; // additional symbol for some transparent notehead

      QString _fretString;

      void startDrag(EditData&) override;
      QRectF drag(EditData&ed) override;
      void endDrag(EditData&) override;
      void editDrag(EditData &editData) override;

      void verticalDrag(EditData& ed);
      void horizontalDrag(EditData& ed);

      void addSpanner(Spanner*);
      void removeSpanner(Spanner*);
      int concertPitchIdx() const;
      void updateRelLine(int absLine, bool undoable);
      bool isNoteName() const;

      void normalizeLeftDragDelta(Segment* seg, EditData &ed, NoteEditData* ned);

      static QString tpcUserName(int tpc, int pitch, bool explicitAccidental);

   public:
      Note(Score* s = 0);
      Note(const Note&, bool link = false);
      ~Note();

      Note& operator=(const Note&) = delete;
      virtual Note* clone() const override  { return new Note(*this, false); }
      ElementType type() const override   { return ElementType::NOTE; }

      void undoUnlink() override;

      qreal mag() const override;

      void layout() override;
      void layout2();
      //setter is used only in drumset tools to setup the notehead preview in the drumset editor and the palette
      void setCachedNoteheadSym(SymId i) { _cachedNoteheadSym = i; }
      void scanElements(void* data, void (*func)(void*, Element*), bool all = true) override;
      void setTrack(int val) override;

      int playTicks() const;
      Fraction playTicksFraction() const;

      qreal headWidth() const;
      qreal headHeight() const;
      qreal tabHeadWidth(const StaffType* tab = 0) const;
      qreal tabHeadHeight(const StaffType* tab = 0) const;
      QPointF stemDownNW() const;
      QPointF stemUpSE() const;
      qreal bboxXShift() const;
      qreal noteheadCenterX() const;
      qreal bboxRightPos() const;
      qreal headBodyWidth() const;

      NoteHead::Scheme headScheme() const { return _headScheme; }
      NoteHead::Group headGroup() const   { return _headGroup; }
      NoteHead::Type headType() const     { return _headType;  }
      void setHeadScheme(NoteHead::Scheme val);
      void setHeadGroup(NoteHead::Group val);
      void setHeadType(NoteHead::Type t);

      int subtype() const override { return int(_headGroup); }
      QString subtypeName() const override;

      void setPitch(int val);
      void setPitch(int pitch, int tpc1, int tpc2);
      int pitch() const                   { return _pitch;    }
      int ottaveCapoFret() const;
      int ppitch() const;           ///< playback pitch
      int epitch() const;           ///< effective pitch
      qreal tuning() const                { return _tuning;   }
      void setTuning(qreal v)             { _tuning = v;      }
      void undoSetTpc(int v);
      int transposition() const;
      bool fixed() const                  { return _fixed;     }
      void setFixed(bool v)               { _fixed = v;        }
      int fixedLine() const               { return _fixedLine; }
      void setFixedLine(int v)            { _fixedLine = v;    }

      int tpc() const;
      int tpc1() const            { return _tpc[0]; }     // non transposed tpc
      int tpc2() const            { return _tpc[1]; }     // transposed tpc
      QString tpcUserName(bool explicitAccidental = false) const;

      void setTpc(int v);
      void setTpc1(int v)         { _tpc[0] = v; }
      void setTpc2(int v)         { _tpc[1] = v; }
      void setTpcFromPitch();
      int tpc1default(int pitch) const;
      int tpc2default(int pitch) const;
      int transposeTpc(int tpc);

      Accidental* accidental() const      { return _accidental; }
      void setAccidental(Accidental* a)   { _accidental = a;    }

      AccidentalType accidentalType() const;
      void setAccidentalType(AccidentalType type);

      int line() const;
      void setLine(int n)             { _line = n;      }

      int fret() const                { return _fret;   }
      void setFret(int val)           { _fret = val;    }
      int string() const              { return _string; }
      void setString(int val);
      bool ghost() const              { return _ghost;  }
      void setGhost(bool val)         { _ghost = val;   }
      bool fretConflict() const       { return _fretConflict; }
      void setFretConflict(bool val)  { _fretConflict = val; }

      void add(Element*) override;
      void remove(Element*) override;

      bool mirror() const             { return _mirror;  }
      void setMirror(bool val)        { _mirror = val;   }

      bool isSmall() const            { return m_isSmall; }
      void setSmall(bool val);

      bool play() const               { return _play;    }
      void setPlay(bool val)          { _play = val;     }

      Ms::Tie* tieFor() const         { return _tieFor;  }
      Ms::Tie* tieBack() const        { return _tieBack; }
      void setTieFor(Tie* t)          { _tieFor = t;     }
      void setTieBack(Tie* t)         { _tieBack = t;    }
      Note* firstTiedNote() const;
      const Note* lastTiedNote() const;
      Note* lastTiedNote()            { return const_cast<Note*>(static_cast<const Note*>(this)->lastTiedNote()); }
      int unisonIndex() const;
      void disconnectTiedNotes();
      void connectTiedNotes();

      Chord* chord() const            { return (Chord*)parent(); }
      void setChord(Chord* a)         { setParent((Element*)a);  }
      void draw(QPainter*) const override;

      void read(XmlReader&) override;
      bool readProperties(XmlReader&) override;
      void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;
      void write(XmlWriter&) const override;

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;

      bool hidden() const                       { return _hidden; }
      void setHidden(bool val)                  { _hidden = val;  }
      bool dotsHidden() const                   { return _dotsHidden; }
      void setDotsHidden(bool val)              { _dotsHidden = val;  }

      NoteType noteType() const;
      QString  noteTypeUserName() const;

      ElementList& el()                           { return _el; }
      const ElementList& el() const               { return _el; }

      int subchannel() const                    { return _subchannel; }
      void setSubchannel(int val)               { _subchannel = val;  }

      MScore::DirectionH userMirror() const             { return _userMirror; }
      void setUserMirror(MScore::DirectionH d)          { _userMirror = d; }

      Direction userDotPosition() const         { return _userDotPosition; }
      void setUserDotPosition(Direction d)      { _userDotPosition = d;    }
      bool dotIsUp() const;               // actual dot position

      void reset() override;

      ValueType veloType() const            { return _veloType;          }
      void setVeloType(ValueType v)         { _veloType = v;             }
      int veloOffset() const                { return _veloOffset;        }
      void setVeloOffset(int v)             { _veloOffset = v;           }

      void setOnTimeOffset(int v);
      void setOffTimeOffset(int v);

      int customizeVelocity(int velo) const;
      NoteDot* dot(int n)                         { return _dots[n];          }
      const QVector<NoteDot*>& dots() const       { return _dots;             }
      QVector<NoteDot*>& dots()                   { return _dots;             }

      int qmlDotsCount();
      void updateAccidental(AccidentalState*);
      void updateLine();
      void setNval(const NoteVal&, Fraction = { -1, 1} );
      NoteEventList& playEvents()                { return _playEvents; }
      const NoteEventList& playEvents() const    { return _playEvents; }
      NoteEvent* noteEvent(int idx)              { return &_playEvents[idx]; }
      void setPlayEvents(const NoteEventList& l) { _playEvents = l;    }

      const QVector<Spanner*>& spannerFor() const   { return _spannerFor;         }
      const QVector<Spanner*>& spannerBack() const  { return _spannerBack;        }

      void addSpannerBack(Spanner* e)            { if (!_spannerBack.contains(e)) _spannerBack.push_back(e);  }
      bool removeSpannerBack(Spanner* e)         { return _spannerBack.removeOne(e); }
      void addSpannerFor(Spanner* e)             { if (!_spannerFor.contains(e)) _spannerFor.push_back(e);    }
      bool removeSpannerFor(Spanner* e)          { return _spannerFor.removeOne(e);  }

      void transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals);

      void localSpatiumChanged(qreal oldValue, qreal newValue) override;
      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      void undoChangeDotsVisible(bool v);
      QVariant propertyDefault(Pid) const override;
      QString propertyUserValue(Pid) const override;

      bool mark() const               { return _mark;   }
      void setMark(bool v) const      { _mark = v;   }
      void setScore(Score* s) override;
      void setDotY(Direction);

      void addParentheses();

      SymId noteHead() const;
      static SymId noteHead(int direction, NoteHead::Group, NoteHead::Type, int tpc, Key key, NoteHead::Scheme scheme);
      static SymId noteHead(int direction, NoteHead::Group, NoteHead::Type);
      NoteVal noteVal() const;

      Element* nextInEl(Element* e);
      Element* prevInEl(Element* e);
      Element* nextElement() override;
      Element* prevElement() override;
      virtual Element* lastElementBeforeSegment();
      Element* nextSegmentElement() override;
      Element* prevSegmentElement() override;

      QString accessibleInfo() const override;
      QString screenReaderInfo() const override;
      QString accessibleExtraInfo() const override;

      Shape shape() const override;
      std::vector<Note*> tiedNotes() const;

      void setOffTimeType(int v) { _offTimeType = v; }
      void setOnTimeType(int v)  { _onTimeType = v; }
      int offTimeType() const    { return _offTimeType; }
      int onTimeType() const     { return _onTimeType; }
      };

}     // namespace Ms
#endif
