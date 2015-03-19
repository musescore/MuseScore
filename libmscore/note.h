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
#include "accidental.h"

class QPainter;

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
enum class SymId;

static const int MAX_DOTS = 3;

//---------------------------------------------------------
//   @@ NoteHead
//---------------------------------------------------------

class NoteHead : public Symbol {
      Q_OBJECT

      Q_ENUMS(Group)
      Q_ENUMS(Type)
   public:
      enum class Group : signed char {
            HEAD_NORMAL = 0,
            HEAD_CROSS,
            HEAD_DIAMOND,
            HEAD_TRIANGLE,
            HEAD_MI,
            HEAD_SLASH,
            HEAD_XCIRCLE,
            HEAD_DO,
            HEAD_RE,
            HEAD_FA,
            HEAD_LA,
            HEAD_TI,
            HEAD_SOL,
            HEAD_BREVIS_ALT,
            HEAD_GROUPS,
            HEAD_INVALID = -1
            };
      enum class Type : signed char {
            HEAD_AUTO    = -1,
            HEAD_WHOLE   = 0,
            HEAD_HALF    = 1,
            HEAD_QUARTER = 2,
            HEAD_BREVIS  = 3,
            HEAD_TYPES
            };

      NoteHead(Score* s = 0) : Symbol(s) {}
      NoteHead &operator=(const NoteHead&) = delete;
      virtual NoteHead* clone() const override    { return new NoteHead(*this); }
      virtual Element::Type type() const override { return Element::Type::NOTEHEAD; }

      virtual void write(Xml& xml) const override;

      Group headGroup() const;

      static const char* groupToGroupName(Group group);
      };

//---------------------------------------------------------
//   NoteVal
//    helper structure
//---------------------------------------------------------

struct NoteVal {
      int pitch                 { -1 };
      int tpc1                  { Tpc::TPC_INVALID };
      int tpc2                  { Tpc::TPC_INVALID };
      int fret                  { FRET_NONE };
      int string                { STRING_NONE };
      NoteHead::Group headGroup { NoteHead::Group::HEAD_NORMAL };

      NoteVal() {}
      NoteVal(int p) : pitch(p) {}
      };

//---------------------------------------------------------------------------------------
//   @@ Note
///    Graphic representation of a note.
//
//   @P subchannel       int                     midi subchannel (for midi articulation) (read only)
//   @P line             int                     notehead position (read only)
//   @P fret             int                     fret number in tablature
//   @P string           int                     string number in tablature
//   @P tpc              int                     tonal pitch class, as per concert pitch setting
//   @P tpc1             int                     tonal pitch class, non transposed
//   @P tpc2             int                     tonal pitch class, transposed
//   @P pitch            int                     midi pitch
//   @P ppitch           int                     actual played midi pitch (honoring ottavas) (read only)
//   @P ghost            bool                    ghost note (guitar: death note)
//   @P hidden           bool                    hidden, not played note (read only)
//   @P mirror           bool                    mirror note head on x axis (read only)
//   @P small            bool                    small note head
//   @P play             bool                    play note
//   @P tuning           qreal                   tuning offset in cent
//   @P veloType         Ms::Note::ValueType     (OFFSET_VAL, USER_VAL)
//   @P veloOffset       int
//   @P userMirror       Ms::MScore::DirectionH  (AUTO, LEFT, RIGHT)
//   @P userDotPosition  Ms::MScore::Direction   (AUTO, UP, DOWN)
//   @P headGroup        Ms::NoteHead::Group     (HEAD_NORMAL, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE, HEAD_MI, HEAD_SLASH, HEAD_XCIRCLE, HEAD_DO, HEAD_RE, HEAD_FA, HEAD_LA, HEAD_TI, HEAD_SOL, HEAD_BREVIS_ALT)
//   @P headType         Ms::NoteHead::Type      (HEAD_AUTO, HEAD_WHOLE, HEAD_HALF, HEAD_QUARTER, HEAD_BREVIS)
//   @P elements         array[Ms::Element]      list of elements attached to note head
//   @P accidental       Ms::Accidental          note accidental (null if none)
//   @P accidentalType   Ms::Accidental::Type    note accidental type
//   @P dots             array[Ms::NoteDot]      list of note dots (some can be null, read only)
//   @P dotsCount        int                     number of note dots (read only)
//   @P tieFor           Ms::Tie                 note forward tie (null if none, read only)
//   @P tieBack          Ms::Tie                 note backward tie (null if none, read only)
//---------------------------------------------------------------------------------------

class Note : public Element {
      Q_OBJECT
      Q_PROPERTY(int subchannel                          READ subchannel)
      Q_PROPERTY(int line                                READ line)
      Q_PROPERTY(int fret                                READ fret             WRITE undoSetFret)
      Q_PROPERTY(int string                              READ string           WRITE undoSetString)
      Q_PROPERTY(int tpc                                 READ tpc)
      Q_PROPERTY(int tpc1                                READ tpc1             WRITE undoSetTpc1)
      Q_PROPERTY(int tpc2                                READ tpc2             WRITE undoSetTpc2)
      Q_PROPERTY(int pitch                               READ pitch            WRITE undoSetPitch)
      Q_PROPERTY(int ppitch                              READ ppitch)
      Q_PROPERTY(bool ghost                              READ ghost            WRITE undoSetGhost)
      Q_PROPERTY(bool hidden                             READ hidden)
      Q_PROPERTY(bool mirror                             READ mirror)
      Q_PROPERTY(bool small                              READ small            WRITE undoSetSmall)
      Q_PROPERTY(bool play                               READ play             WRITE undoSetPlay)
      Q_PROPERTY(qreal tuning                            READ tuning           WRITE undoSetTuning)
      Q_PROPERTY(Ms::Note::ValueType veloType            READ veloType         WRITE undoSetVeloType)
      Q_PROPERTY(int veloOffset                          READ veloOffset       WRITE undoSetVeloOffset)
      Q_PROPERTY(Ms::MScore::DirectionH userMirror       READ userMirror       WRITE undoSetUserMirror)
      Q_PROPERTY(Ms::MScore::Direction userDotPosition   READ userDotPosition  WRITE undoSetUserDotPosition)
      Q_PROPERTY(Ms::NoteHead::Group headGroup           READ headGroup        WRITE undoSetHeadGroup)
      Q_PROPERTY(Ms::NoteHead::Type headType             READ headType         WRITE undoSetHeadType)
      Q_PROPERTY(QQmlListProperty<Ms::Element> elements  READ qmlElements)
      Q_PROPERTY(Ms::Accidental* accidental              READ accidental)
      Q_PROPERTY(Ms::Accidental::Type accidentalType     READ accidentalType   WRITE setAccidentalType)
      Q_PROPERTY(QQmlListProperty<Ms::NoteDot> dots      READ qmlDots)
      Q_PROPERTY(int dotsCount                           READ qmlDotsCount)
      Q_PROPERTY(Ms::Tie* tieFor                         READ tieFor)
      Q_PROPERTY(Ms::Tie* tieBack                        READ tieBack)
      Q_ENUMS(ValueType)
      Q_ENUMS(Ms::MScore::Direction)
      Q_ENUMS(Ms::MScore::DirectionH)

   public:
      enum class ValueType : char { OFFSET_VAL, USER_VAL };

   private:
      int _subchannel     { 0  };   ///< articulation
      int _line           { 0  };   ///< y-Position; 0 - top line.
      int _fret           { -1 };   ///< for tablature view
      int _string         { -1 };
      mutable int _tpc[2] { Tpc::TPC_INVALID, Tpc::TPC_INVALID }; ///< tonal pitch class  (concert/transposing)
      mutable int _pitch  { 0  };   ///< Note pitch as midi value (0 - 127).

      bool _ghost         { false };      ///< ghost note (guitar: death note)
      bool _hidden        { false };      ///< markes this note as the hidden one if there are
                                          ///< overlapping notes; hidden notes are not played
                                          ///< and heads + accidentals are not shown
      bool _dotsHidden    { false };      ///< dots of hidden notes are hidden too
                                          ///< except if only one note is dotted
      bool _fretConflict  { false };      ///< used by TAB staves to mark a fretting conflict:
                                          ///< two or mor enotes on the same string

      bool dragMode       { false };
      bool _mirror        { false };      ///< True if note is mirrored at stem.
      bool _small         { false };
      bool _play          { true  };      // note is not played if false
      mutable bool _mark  { false };      // for use in sequencer
      bool _fixed         { false };      // for slash notation

      MScore::DirectionH _userMirror { MScore::DirectionH::AUTO };    ///< user override of mirror
      MScore::Direction _userDotPosition { MScore::Direction::AUTO }; ///< user override of dot position

      NoteHead::Group _headGroup { NoteHead::Group::HEAD_NORMAL };
      NoteHead::Type  _headType  { NoteHead::Type::HEAD_AUTO    };

      ValueType _veloType { ValueType::OFFSET_VAL };
      int _veloOffset     { 0 };    ///< velocity user offset in percent, or absolute velocity for this note
      int _fixedLine      { 0 };    // fixed line number if _fixed == true
      int _offTimeType    { 0 };    // compatibility only 1 - user(absolute), 2 - offset (%)
      int _onTimeType     { 0 };    // compatibility only 1 - user, 2 - offset
      int _lineOffset     { 0 };    ///< Used during mouse dragging.
      qreal _tuning       { 0.0 };  ///< pitch offset in cent, playable only by internal synthesizer

      Accidental* _accidental { 0 };

      ElementList _el;        ///< fingering, other text, symbols or images
      Tie* _tieFor        { 0 };
      Tie* _tieBack       { 0 };

      QList<NoteDot*> _dots { 0, 0, 0 };

      NoteEventList _playEvents;
      QList<Spanner*> _spannerFor;
      QList<Spanner*> _spannerBack;

      virtual QRectF drag(EditData*) override;
      void endDrag();
      void endEdit();
      void addSpanner(Spanner*);
      void removeSpanner(Spanner*);
      int concertPitchIdx() const;
      void updateRelLine(int relLine, bool undoable);

   public:
      Note(Score* s = 0);
      Note(const Note&, bool link = false);
      ~Note();

      Note& operator=(const Note&) = delete;
      virtual Note* clone() const override  { return new Note(*this, false); }
      Element::Type type() const override   { return Element::Type::NOTE; }

      virtual qreal mag() const override;

      QPointF pagePos() const;      ///< position in page coordinates
      QPointF canvasPos() const;    ///< position in page coordinates
      void layout();
      void layout2();
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      void setTrack(int val);

      int playTicks() const;

      qreal headWidth() const;
      qreal headHeight() const;
      qreal tabHeadWidth(StaffType* tab = 0) const;
      qreal tabHeadHeight(StaffType* tab = 0) const;
      QPointF attach() const;

      SymId noteHead() const;
      NoteHead::Group headGroup() const   { return _headGroup; }
      NoteHead::Type headType() const     { return _headType;  }
      void setHeadGroup(NoteHead::Group val);
      void setHeadType(NoteHead::Type t);

      virtual int subtype() const override { return (int) _headGroup; }
      virtual QString subtypeName() const override;

      void setPitch(int val);
      void undoSetPitch(int val);
      void setPitch(int pitch, int tpc1, int tpc2);
      int pitch() const                   { return _pitch;    }
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
      QString tpcUserName(bool explicitAccidental = false);

      void setTpc(int v);
      void setTpc1(int v)         { _tpc[0] = v; }
      void setTpc2(int v)         { _tpc[1] = v; }
      void setTpcFromPitch();
      int tpc1default(int pitch) const;
      int tpc2default(int pitch) const;
      void undoSetTpc1(int tpc)      { undoChangeProperty(P_ID::TPC1, tpc); }
      void undoSetTpc2(int tpc)      { undoChangeProperty(P_ID::TPC2, tpc); }
      int transposeTpc(int tpc);

      Accidental* accidental() const    { return _accidental; }
      void setAccidental(Accidental* a)   { _accidental = a;    }

      Accidental::Type accidentalType() const { return _accidental ? _accidental->accidentalType() : Accidental::Type::NONE; }
      void setAccidentalType(Accidental::Type type);

      int line() const;
      void setLine(int n);

      int fret() const                { return _fret;   }
      void setFret(int val)           { _fret = val;    }
      int string() const              { return _string; }
      void setString(int val);
      bool ghost() const              { return _ghost;  }
      void setGhost(bool val)         { _ghost = val;   }
      bool fretConflict() const       { return _fretConflict; }
      void setFretConflict(bool val)  { _fretConflict = val; }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;

      bool mirror() const             { return _mirror;  }
      void setMirror(bool val)        { _mirror = val;   }

      bool small() const              { return _small;   }
      void setSmall(bool val);

      bool play() const               { return _play;    }
      void setPlay(bool val)          { _play = val;     }

      Ms::Tie* tieFor() const         { return _tieFor;  }
      Ms::Tie* tieBack() const        { return _tieBack; }
      void setTieFor(Tie* t)          { _tieFor = t;     }
      void setTieBack(Tie* t)         { _tieBack = t;    }
      Note* firstTiedNote() const;
      Note* lastTiedNote() const;
      QList<Note*> tiedNotes() const;

      Chord* chord() const            { return (Chord*)parent(); }
      void setChord(Chord* a)         { setParent((Element*)a);  }
      void draw(QPainter*) const;

      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;

      bool acceptDrop(const DropData&) const override;
      Element* drop(const DropData&);

      bool hidden() const                       { return _hidden; }
      void setHidden(bool val)                  { _hidden = val;  }
      bool dotsHidden() const                   { return _dotsHidden; }
      void setDotsHidden(bool val)              { _dotsHidden = val;  }

      NoteType noteType() const;
      QString  noteTypeUserName();

      ElementList el()                            { return _el; }
      const ElementList el() const                { return _el; }
      QQmlListProperty<Ms::Element> qmlElements() { return QQmlListProperty<Ms::Element>(this, _el); }

      int subchannel() const                    { return _subchannel; }
      void setSubchannel(int val)               { _subchannel = val;  }

      MScore::DirectionH userMirror() const             { return _userMirror; }
      void setUserMirror(MScore::DirectionH d)          { _userMirror = d; }

      MScore::Direction userDotPosition() const         { return _userDotPosition; }
      void setUserDotPosition(MScore::Direction d)      { _userDotPosition = d;    }
      bool dotIsUp() const;               // actual dot position

      void reset();

      ValueType veloType() const            { return _veloType;          }
      void setVeloType(ValueType v)         { _veloType = v;             }
      int veloOffset() const                { return _veloOffset;        }
      void setVeloOffset(int v)             { _veloOffset = v;           }

      void setOnTimeOffset(int v);
      void setOffTimeOffset(int v);

      int customizeVelocity(int velo) const;
      NoteDot* dot(int n)                       { return _dots[n];           }
      QQmlListProperty<Ms::NoteDot> qmlDots() { return QQmlListProperty<Ms::NoteDot>(this, _dots);  }
      int qmlDotsCount();
      void updateAccidental(AccidentalState*);
      void updateLine();
      void setNval(const NoteVal&, int tick = -1);
      NoteEventList& playEvents()                { return _playEvents; }
      const NoteEventList& playEvents() const    { return _playEvents; }
      NoteEvent* noteEvent(int idx)              { return &_playEvents[idx]; }
      void setPlayEvents(const NoteEventList& l) { _playEvents = l;    }

      QList<Spanner*> spannerFor() const         { return _spannerFor;         }
      QList<Spanner*> spannerBack() const        { return _spannerBack;        }

      void addSpannerBack(Spanner* e)            { _spannerBack.push_back(e);  }
      bool removeSpannerBack(Spanner* e)         { return _spannerBack.removeOne(e); }
      void addSpannerFor(Spanner* e)             { _spannerFor.push_back(e);         }
      bool removeSpannerFor(Spanner* e)          { return _spannerFor.removeOne(e);  }

      void transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals);

      void undoSetFret(int);
      void undoSetString(int);
      void undoSetGhost(bool);
      void undoSetMirror(bool);
      void undoSetSmall(bool);
      void undoSetPlay(bool);
      void undoSetTuning(qreal);
      void undoSetVeloType(ValueType);
      void undoSetVeloOffset(int);
      void undoSetOnTimeUserOffset(int);
      void undoSetOffTimeUserOffset(int);
      void undoSetUserMirror(MScore::DirectionH);
      void undoSetUserDotPosition(MScore::Direction);
      void undoSetHeadGroup(NoteHead::Group);
      void undoSetHeadType(NoteHead::Type);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      bool mark() const               { return _mark;   }
      void setMark(bool v) const      { _mark = v;   }
      virtual void setScore(Score* s) override;
      void setDotY(MScore::Direction);

      void addBracket();

      static SymId noteHead(int direction, NoteHead::Group, NoteHead::Type);
      NoteVal noteVal() const;

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      virtual QString accessibleInfo() override;
      virtual QString screenReaderInfo() override;
      virtual QString accessibleExtraInfo() override;
      };

// extern const SymId noteHeads[2][int(NoteHead::Group::HEAD_GROUPS)][int(NoteHead::Type::HEAD_TYPES)];


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::NoteHead::Group);
Q_DECLARE_METATYPE(Ms::NoteHead::Type);
Q_DECLARE_METATYPE(Ms::Note::ValueType);

#endif

