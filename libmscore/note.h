//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: note.h 5534 2012-04-12 17:40:51Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
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
#include "durationtype.h"

class Tie;
class Chord;
class NoteEvent;
class Text;
class Score;
class Sym;
class MuseScoreView;
class Bend;
class QPainter;
class AccidentalState;
class Accidental;
class Spanner;
class NoteDot;
class Note;

//---------------------------------------------------------
//   NoteVal
//    helper structure
//---------------------------------------------------------

struct NoteVal {
      int pitch;
      int tpc;
      int fret;
      int string;
      int headGroup;
      NoteVal();
      };

//---------------------------------------------------------
//   @@ NoteHead
//---------------------------------------------------------

class NoteHead : public Symbol {
      Q_OBJECT

   public:
      NoteHead(Score* s) : Symbol(s) {}
      NoteHead &operator=(const NoteHead&);
      virtual NoteHead* clone() const  { return new NoteHead(*this); }
      virtual ElementType type() const { return NOTEHEAD; }
      virtual void write(Xml& xml) const;
      };

//-------------------------------------------------------------------
//   @@ Note
///  Graphic representation of a note.
//
//   @P pitch             int   midi pitch
//   @P tpc               int   tonal pitch class
//   @P line              int   notehead position
//   @P fret              int   fret number in tablature
//   @P string            int   string number in tablature
//   @P subchannel        int   midi subchannel (for midi articulation)
//   @P ppitch            int   actual played midi pitch (honoring ottavas)
//   @P ghost             bool  ghost note (guitar: death note)
//   @P hidden            bool  hidden, not played note
//   @P mirror            bool  mirror note head on x axis
//   @P small             bool  small note head
//   @P tuning            qreal tuning offset in cent
//   @P veloType          enum  OFFSET_VAL, USER_VAL
//   @P veloOffset        int
//   @P onTimeOffset      int
//   @P onTimeUserOffset  int
//   @P offTimeOffset     int
//   @P offTimeUserOffset int
//   @P userMirror        enum DH_AUTO, DH_LEFT, DH_RIGHT
//   @P dotPosition       enum AUTO, UP, DOWN
//   @P headGroup         enum HEAD_NORMAL, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE, HEAD_MI, HEAD_SLASH, HEAD_XCIRCLE, HEAD_DO, HEAD_RE, HEAD_FA, HEAD_LA, HEAD_TI, HEAD_SOL, HEAD_BREVIS_ALT
//   @P headType          enum HEAD_AUTO, HEAD_WHOLE, HEAD_HALF, HEAD_QUARTER, HEAD_BREVIS
//-------------------------------------------------------------------

class Note : public Element {
   public:
      enum NoteHeadGroup {
            HEAD_NORMAL = 0, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE, HEAD_MI,
            HEAD_SLASH, HEAD_XCIRCLE, HEAD_DO, HEAD_RE, HEAD_FA, HEAD_LA, HEAD_TI,
            HEAD_SOL,
            HEAD_BREVIS_ALT,
            HEAD_GROUPS,
            HEAD_INVALID = -1
            };
      enum NoteHeadType { HEAD_AUTO, HEAD_WHOLE, HEAD_HALF, HEAD_QUARTER, HEAD_BREVIS };

   private:
      Q_OBJECT
      Q_PROPERTY(int subchannel READ subchannel WRITE setSubchannel)
      Q_PROPERTY(int line       READ line       WRITE setLine)
      Q_PROPERTY(int fret       READ fret       WRITE setFret)
      Q_PROPERTY(int string     READ string     WRITE setString)
      Q_PROPERTY(int tpc        READ tpc        WRITE undoSetTpc)
      Q_PROPERTY(int pitch      READ pitch      WRITE undoSetPitch)
      Q_PROPERTY(int ppitch     READ ppitch)
      Q_PROPERTY(bool ghost     READ ghost      WRITE setGhost)
      Q_PROPERTY(bool hidden    READ hidden     WRITE setHidden)
      Q_PROPERTY(bool mirror    READ mirror     WRITE setMirror)
      Q_PROPERTY(bool small     READ small      WRITE setSmall)
      Q_PROPERTY(qreal tuning   READ tuning     WRITE setTuning)
      Q_PROPERTY(MScore::ValueType veloType    READ veloType          WRITE setVeloType)
      Q_PROPERTY(int veloOffset                READ veloOffset        WRITE setVeloOffset)
      Q_PROPERTY(int onTimeOffset              READ onTimeOffset      WRITE setOnTimeOffset)
      Q_PROPERTY(int onTimeUserOffset          READ onTimeUserOffset  WRITE setOnTimeUserOffset)
      Q_PROPERTY(int offTimeOffset             READ offTimeOffset     WRITE setOffTimeOffset)
      Q_PROPERTY(int offTimeUserOffset         READ offTimeUserOffset WRITE setOffTimeUserOffset)
      Q_PROPERTY(MScore::DirectionH userMirror READ userMirror        WRITE setUserMirror)
      Q_PROPERTY(MScore::Direction dotPosition READ dotPosition       WRITE setDotPosition)
      Q_PROPERTY(NoteHeadGroup     headGroup   READ headGroup         WRITE setHeadGroup)
      Q_PROPERTY(NoteHeadType      headType    READ headType          WRITE setHeadType)

      int _subchannel;        ///< articulation
      int _line;              ///< y-Position; 0 - top line.
      int _fret;              ///< for tablature view
      int _string;
      NoteHeadGroup _headGroup;
      mutable int _tpc;       ///< tonal pitch class
      mutable int _pitch;     ///< Note pitch as midi value (0 - 127).
      int  _ppitch;           ///< played pitch (honor ottavas etc.); cached value
      bool _ghost;            ///< ghost note (guitar: death note)
      bool _hidden;           ///< markes this note as the hidden one if there are
                              ///< overlapping notes; hidden notes are not played
                              ///< and heads + accidentals are not shown
      bool _fretConflict;     ///< used by TAB staves to mark a fretting conflict:
                              ///< two or mor enotes on the same string

      bool dragMode;
      bool _mirror;           ///< True if note is mirrored at stem.
      bool _small;

      NoteHeadType _headType;
      MScore::ValueType _veloType;
      int _veloOffset;        ///< velocity user offset in percent, or absolute velocity for this note

      qreal _tuning;         ///< pitch offset in cent, playable only by internal synthesizer

      int _onTimeOffset;      ///< start note offset in ticks
      int _onTimeUserOffset;  ///< start note user offset

      int _offTimeOffset;     ///< stop note offset in ticks
      int _offTimeUserOffset; ///< stop note user offset

      MScore::DirectionH _userMirror; ///< user override of mirror
      MScore::Direction _dotPosition; ///< dot position: above or below current staff line

      Accidental* _accidental;

      ElementList _el;        ///< fingering, other text, symbols or images
      Tie* _tieFor;
      Tie* _tieBack;
      Bend* _bend;

      NoteDot* _dots[3];

      QList<NoteEvent*> _playEvents;

      int _lineOffset;        ///< Used during mouse dragging.

      virtual QRectF drag(const EditData& s);
      void endDrag();
      void endEdit();
      void writeProperty(Xml&, P_ID) const;

   public:
      Note(Score* s = 0);
      Note(const Note&);
      Note &operator=(const Note&);
      ~Note();
      Note* clone() const      { return new Note(*this); }
      ElementType type() const { return NOTE; }
      QPointF pagePos() const;      ///< position in page coordinates
      QPointF canvasPos() const;    ///< position in page coordinates
      void layout();
      void layout2();
      void layout10(AccidentalState*);
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      void setTrack(int val);

      int playTicks() const;

      qreal headWidth() const;
      qreal headHeight() const;
      int noteHead() const;
      NoteHeadGroup headGroup() const  { return _headGroup; }
      NoteHeadType headType() const    { return _headType;  }
      void setHeadGroup(NoteHeadGroup val);
      void setHeadType(NoteHeadType t) { _headType = t;     }

      int pitch() const               { return _pitch;    }
      void setPitch(int val);
      void undoSetPitch(int val);
      void setPitch(int a, int b);
      int ppitch() const;
      qreal tuning() const           { return _tuning;   }
      void setTuning(qreal v)        { _tuning = v;      }

      int tpc() const                 { return _tpc;      }
      void setTpc(int v);
      void undoSetTpc(int v);
      void setTpcFromPitch();

      Q_INVOKABLE Accidental* accidental() const    { return _accidental; }
      void setAccidental(Accidental* a) { _accidental = a;    }

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n);

      int fret() const                { return _fret;   }
      void setFret(int val)           { _fret = val;    }
      int string() const              { return _string; }
      void setString(int val);
      bool ghost() const              { return _ghost;  }
      void setGhost(bool val)         { _ghost = val;   }
      bool fretConflict() const       { return _fretConflict; }
      void setFretConflict(bool val)  { _fretConflict = val; }

      virtual void add(Element*);
      virtual void remove(Element*);

      bool mirror() const             { return _mirror;  }
      void setMirror(bool val)        { _mirror = val;   }

      bool small() const              { return _small;   }
      void setSmall(bool val)         { _small = val;    }

      Q_INVOKABLE Tie* tieFor() const  { return _tieFor;  }
      Q_INVOKABLE Tie* tieBack() const { return _tieBack; }
      void setTieFor(Tie* t)          { _tieFor = t;     }
      void setTieBack(Tie* t)         { _tieBack = t;    }

      Chord* chord() const            { return (Chord*)parent(); }
      void setChord(Chord* a)         { setParent((Element*)a);  }

      void draw(QPainter*) const;

      void read(const QDomElement&);
      void write(Xml& xml) const;

      QPointF stemPos(bool upFlag) const;    ///< Point to connect stem.

      bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      Element* drop(const DropData&);

      bool hidden() const              { return _hidden; }
      void setHidden(bool val)         { _hidden = val;  }

      NoteType noteType() const;

      ElementList el()                 { return _el; }
      const ElementList el() const     { return _el; }

      int subchannel() const           { return _subchannel; }
      void setSubchannel(int val)      { _subchannel = val;  }

      MScore::DirectionH userMirror() const    { return _userMirror; }
      void setUserMirror(MScore::DirectionH d) { _userMirror = d; }

      MScore::Direction dotPosition() const    { return _dotPosition; }
      void setDotPosition(MScore::Direction d) { _dotPosition = d;    }
      bool dotIsUp() const;               // actual dot position

      void toDefault();
      void setMag(qreal val);

      MScore::ValueType veloType() const    { return _veloType;          }
      void setVeloType(MScore::ValueType v) { _veloType = v;             }
      int veloOffset() const           { return _veloOffset;        }
      void setVeloOffset(int v)        { _veloOffset = v;           }

      int onTimeOffset() const         { return _onTimeOffset;      }
      void setOnTimeOffset(int v)      { _onTimeOffset = v;         }
      int onTimeUserOffset() const     { return _onTimeUserOffset;  }
      void setOnTimeUserOffset(int v)  { _onTimeUserOffset = v;     }

      int offTimeOffset() const        { return _offTimeOffset;     }
      void setOffTimeOffset(int v)     { _offTimeOffset = v;        }
      int offTimeUserOffset() const    { return _offTimeUserOffset; }
      void setOffTimeUserOffset(int v) { _offTimeUserOffset = v;    }

      void setBend(Bend* b)            { _bend = b;    }
      int customizeVelocity(int velo) const;
      Q_INVOKABLE NoteDot* dot(int n)  { return _dots[n];           }
      void updateAccidental(AccidentalState*);
      void updateLine();
      void setNval(NoteVal);
      QList<NoteEvent*>& playEvents()                { return _playEvents; }
      const QList<NoteEvent*>& playEvents() const    { return _playEvents; }
      void setPlayEvents(const QList<NoteEvent*>& v);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      };

Q_DECLARE_METATYPE(Note::NoteHeadGroup)
Q_DECLARE_METATYPE(Note::NoteHeadType)

extern Sym* noteHeadSym(bool up, int group, int n);
extern const int noteHeads[2][Note::HEAD_GROUPS][HEAD_TYPES];

#endif

