//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: slur.h 5500 2012-03-28 16:28:26Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SLUR_H__
#define __SLUR_H__

#include "mscore.h"
#include "spanner.h"

class Note;
class Chord;
class System;
class SlurTie;
class Score;
class MuseScoreView;
class QPainter;
class ChordRest;

//---------------------------------------------------------
//   UP
//---------------------------------------------------------

struct UP {
      QPointF p;            // layout position relative to pos()
      QPointF off;          // user offset in spatium units

      bool operator!=(const UP& up) const {
            return p != up.p || off != up.off;
            }
      };

enum {
      GRIP_START, GRIP_BEZIER1, GRIP_SHOULDER, GRIP_BEZIER2, GRIP_END, GRIP_DRAG,
      SLUR_GRIPS
      };

//---------------------------------------------------------
//   SlurSegment
//    also used for Tie
//---------------------------------------------------------

class SlurSegment : public SpannerSegment {
      Q_OBJECT

   protected:
      struct UP ups[SLUR_GRIPS];
      QPainterPath path;
      QPainterPath shapePath;

      void computeBezier();
      void changeAnchor(MuseScoreView*, int curGrip, ChordRest*);

   public:
      SlurSegment(Score*);
      SlurSegment(const SlurSegment&);
      virtual SlurSegment* clone() const { return new SlurSegment(*this); }
      virtual ElementType type() const   { return SLUR_SEGMENT; }

      void layout(const QPointF& p1, const QPointF& p2);
      virtual QPainterPath shape() const { return shapePath; }
      virtual void draw(QPainter*) const;

      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;
      virtual QPointF getGrip(int) const;
      virtual void setGrip(int, const QPointF&);

      virtual void move(qreal xd, qreal yd) { move(QPointF(xd, yd)); }
      virtual void move(const QPointF& s);

      SlurTie* slurTie() const                      { return (SlurTie*)spanner(); }

      void write(Xml& xml, int no) const;
      void read(const QDomElement&);
      virtual void toDefault();
      void setSlurOffset(int i, const QPointF& val) { ups[i].off = val;  }
      QPointF slurOffset(int i) const               { return ups[i].off; }
      const struct UP* getUps(int idx) const        { return &ups[idx]; }

      virtual bool isEdited(SpannerSegment*) const;

      friend class Tie;
      friend class Slur;
      };

//---------------------------------------------------------
//   SlurPos
//---------------------------------------------------------

struct SlurPos {
      QPointF p1;             // start point of slur
      System* system1;        // start system of slur
      QPointF p2;             // end point of slur
      System* system2;        // end system of slur
      };

//---------------------------------------------------------
//   @@ SlurTie
//---------------------------------------------------------

class SlurTie : public Spanner {
      Q_OBJECT

      int _lineType;          // 0 = solid, 1 = dotted, 2 = dashed

   protected:
      qreal _len;
      bool _up;
      QQueue<SlurSegment*> delSegments;   // "deleted" segments
      Direction _slurDirection;
      qreal firstNoteRestSegmentX(System* system);
      void fixupSegments(unsigned nsegs);

   public:
      SlurTie(Score*);
      SlurTie(const SlurTie&);
      ~SlurTie();

      virtual ElementType type() const = 0;
      bool up() const                    { return _up; }
      void setUp(bool val)               { _up = val;  }
      Direction slurDirection() const    { return _slurDirection; }
      void setSlurDirection(Direction d) { _slurDirection = d; }

      virtual void layout2(const QPointF, int, struct UP&)  {}
      virtual bool contains(const QPointF&) const     { return false; }  // not selectable

      void writeProperties(Xml& xml) const;
      bool readProperties(const QDomElement&);

      virtual void toDefault();
      void setLen(qreal v)               { _len = v; }
      int lineType() const                { return _lineType; }
      void setLineType(int val)           { _lineType = val;  }
      SlurSegment* frontSegment() const   { return (SlurSegment*)spannerSegments().front(); }
      SlurSegment* backSegment() const    { return (SlurSegment*)spannerSegments().back();  }
      SlurSegment* takeLastSegment()      { return (SlurSegment*)spannerSegments().takeLast(); }
      SlurSegment* segmentAt(int n) const { return (SlurSegment*)spannerSegments().at(n); }
      virtual void slurPos(SlurPos*) = 0;
      virtual void computeBezier(SlurSegment*, QPointF so = QPointF()) = 0;
      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      };

//---------------------------------------------------------
//   @@ Slur
//    slurs have Chord's as startElement/endElement
//---------------------------------------------------------

class Slur : public SlurTie {
      Q_OBJECT

      int _track2; // obsolete used temporarily for reading old version

   public:
      Slur(Score* = 0);
      ~Slur();
      virtual Slur* clone() const      { return new Slur(*this); }
      virtual ElementType type() const { return SLUR; }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      virtual void layout();
      virtual void setTrack(int val);
      virtual void slurPos(SlurPos*);
      virtual void computeBezier(SlurSegment*, QPointF so = QPointF());

      int track2() const        { return _track2; }
      int staffIdx2() const     { return _track2 / VOICES; }
      void setTrack2(int val)   { _track2 = val; }

      Chord* startChord() const { return (Chord*)startElement(); }
      Chord* endChord() const   { return (Chord*)endElement();   }

      // obsolete:
      void setStart(int /*tick*/, int /*track*/) {}
      void setEnd(int /*tick*/,   int /*track*/) {}
      };

//---------------------------------------------------------
//   @@ Tie
//    slurs have Note's as startElement/endElement
//---------------------------------------------------------

class Tie : public SlurTie {
      Q_OBJECT

   public:
      Tie(Score* = 0);
      virtual Tie* clone() const          { return new Tie(*this);        }
      virtual ElementType type() const    { return TIE;                   }
      void setStartNote(Note* note);
      void setEndNote(Note* note)         { setEndElement((Element*)note); }
      Note* startNote() const             { return (Note*)startElement(); }
      Note* endNote() const               { return (Note*)endElement();   }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      virtual void layout();
      virtual void slurPos(SlurPos*);
      virtual void computeBezier(SlurSegment*, QPointF so = QPointF());
      };

#endif

