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

#ifndef __SLUR_H__
#define __SLUR_H__

#include "mscore.h"
#include "spanner.h"

class QPainter;

namespace Ms {

class Note;
class Chord;
class System;
class SlurTie;
class Score;
class MuseScoreView;
class ChordRest;
struct SlurPos;

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
//   UP
//---------------------------------------------------------

struct UP {
      QPointF p;            // layout position relative to pos()
      QPointF off;          // user offset in spatium units

      bool operator!=(const UP& up) const {
            return p != up.p || off != up.off;
            }
      };

struct SlurOffsets {
      QPointF o[4];
      };

//---------------------------------------------------------
//   @@ SlurSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class SlurSegment : public SpannerSegment {
      Q_OBJECT

   protected:
      struct UP _ups[int(Grip::GRIPS)];

      QPainterPath path;
      QPainterPath shapePath;
      QPointF autoAdjustOffset;

      void computeBezier();
      void changeAnchor(MuseScoreView*, Grip, Element*);
      void setAutoAdjust(const QPointF& offset);
      void setAutoAdjust(qreal x, qreal y)      { setAutoAdjust(QPointF(x, y)); }
      QPointF getAutoAdjust() const             { return autoAdjustOffset; }

   public:
      SlurSegment(Score*);
      SlurSegment(const SlurSegment&);
      virtual SlurSegment* clone() const { return new SlurSegment(*this); }
      virtual Element::Type type() const { return Element::Type::SLUR_SEGMENT; }
      virtual int subtype() const         { return static_cast<int>(spanner()->type()); }
      virtual QString subtypeName() const { return name(spanner()->type()); }

      void layoutSegment(const QPointF& p1, const QPointF& p2);
      virtual QPainterPath outline() const { return shapePath; }
      virtual void draw(QPainter*) const;

      bool isEdited() const;
      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual bool edit(MuseScoreView*, Grip grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return int(Grip::GRIPS); }
      virtual QPointF gripAnchor(Grip grip) const;

      QPointF getGrip(Grip) const override;
      void setGrip(Grip, const QPointF&) override;

      virtual void move(qreal xd, qreal yd) { move(QPointF(xd, yd)); }
      virtual void move(const QPointF& s);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID id) const;

      SlurTie* slurTie() const { return (SlurTie*)spanner(); }

      void writeSlur(Xml& xml, int no) const;
      void read(XmlReader&);
      virtual void reset() override;
      void setSlurOffset(Grip i, const QPointF& val) { _ups[int(i)].off = val;  }
      const struct UP& ups(Grip i) const             { return _ups[int(i)]; }
      struct UP& ups(Grip i)                         { return _ups[int(i)]; }

      friend class Tie;
      friend class Slur;
      friend class SlurTie;
      };

//-------------------------------------------------------------------
//   @@ SlurTie
//   @P lineType       int    (0 - solid, 1 - dotted, 2 - dashed)
//   @P slurDirection  enum (Direction.AUTO, Direction.DOWN, Direction.UP)
//-------------------------------------------------------------------

class SlurTie : public Spanner {
      Q_OBJECT
      Q_PROPERTY(int lineType                         READ lineType       WRITE undoSetLineType)
//TODO-WS      Q_PROPERTY(Ms::Direction slurDirection  READ slurDirection  WRITE undoSetSlurDirection)
//TODO-WS      Q_ENUMS(Ms::MScore::Direction)

      int _lineType;    // 0 = solid, 1 = dotted, 2 = dashed

      static Element* editStartElement;
      static Element* editEndElement;
      static QList<SlurOffsets> editUps;

   protected:
      bool _up;               // actual direction

      QQueue<SlurSegment*> delSegments;   // "deleted" segments
      Direction _slurDirection;
      qreal firstNoteRestSegmentX(System* system);
      void fixupSegments(unsigned nsegs);

   public:
      SlurTie(Score*);
      SlurTie(const SlurTie&);
      ~SlurTie();

      virtual Element::Type type() const = 0;
      bool up() const                             { return _up; }

      Direction slurDirection() const     { return _slurDirection; }
      void setSlurDirection(Direction d)  { _slurDirection = d; }
      void undoSetSlurDirection(Direction d);

      virtual void layout2(const QPointF, int, struct UP&)  {}
      virtual bool contains(const QPointF&) const { return false; }  // not selectable

      void writeProperties(Xml& xml) const;
      bool readProperties(XmlReader&);

      int lineType() const                { return _lineType; }
      void setLineType(int val)           { _lineType = val;  }
      void undoSetLineType(int);

      SlurSegment* frontSegment() const   { return (SlurSegment*)spannerSegments().front(); }
      SlurSegment* backSegment() const    { return (SlurSegment*)spannerSegments().back();  }
      SlurSegment* takeLastSegment()      { return (SlurSegment*)spannerSegments().takeLast(); }
      SlurSegment* segmentAt(int n) const { return (SlurSegment*)spannerSegments().at(n); }
      virtual void slurPos(SlurPos*) = 0;
      virtual void computeBezier(SlurSegment*, QPointF so = QPointF()) = 0;

      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID id) const;
      };

//---------------------------------------------------------
//   @@ Slur
//---------------------------------------------------------

class Slur : public SlurTie {
      Q_OBJECT

      void slurPosChord(SlurPos*);

   public:
      Slur(Score* = 0);
      ~Slur();
      virtual Slur* clone() const override        { return new Slur(*this); }
      virtual Element::Type type() const override { return Element::Type::SLUR; }
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;
      virtual SpannerSegment* layoutSystem(System*) override;
      virtual void setTrack(int val) override;
      virtual void slurPos(SlurPos*) override;
      virtual void computeBezier(SlurSegment*, QPointF so = QPointF()) override;
      friend SlurSegment;
      };

}     // namespace Ms
#endif

