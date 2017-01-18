//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SLURTIE_H__
#define __SLURTIE_H__

#include "spanner.h"
#include "mscore.h"

namespace Ms {

//---------------------------------------------------------
//   SlurPos
//---------------------------------------------------------

struct SlurPos {
      QPointF p1;             // start point of slur
      System* system1;        // start system of slur
      QPointF p2;             // end point of slur
      System* system2;        // end system of slur
      };

struct SlurOffsets {
      QPointF o[4];
      };

//---------------------------------------------------------
//   UP
//---------------------------------------------------------

struct UP {
      QPointF p;            // layout position relative to pos()
      QPointF off;          // user offset in point units

      bool operator!=(const UP& up) const {
            return p != up.p || off != up.off;
            }
      };

class SlurTie;

//---------------------------------------------------------
//   SlurTieSegment
//---------------------------------------------------------

class SlurTieSegment : public SpannerSegment {
      Q_OBJECT

   protected:
      struct UP _ups[int(Grip::GRIPS)];
      QPainterPath path;
      QPainterPath shapePath;
      Shape _shape;

   public:
      SlurTieSegment(Score*);
      SlurTieSegment(const SlurTieSegment&);
      virtual void spatiumChanged(qreal, qreal) override;
      SlurTie* slurTie() const { return (SlurTie*)spanner(); }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual void reset() override;
      virtual void move(const QPointF& s) override;
      virtual bool isEditable() const override { return true; }

      void setSlurOffset(Grip i, const QPointF& val) { _ups[int(i)].off = val;  }
      const struct UP& ups(Grip i) const             { return _ups[int(i)]; }
      struct UP& ups(Grip i)                         { return _ups[int(i)]; }
      virtual Shape shape() const override           { return _shape; }

      void writeSlur(XmlWriter& xml, int no) const;
      void read(XmlReader&);
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

      QQueue<SpannerSegment*> delSegments;   // "deleted" segments
      Direction _slurDirection;
      qreal firstNoteRestSegmentX(System* system);
      void fixupSegments(unsigned nsegs);

   public:
      SlurTie(Score*);
      SlurTie(const SlurTie&);
      ~SlurTie();

      virtual ElementType type() const = 0;
      bool up() const                             { return _up; }

      virtual void reset() override;

      Direction slurDirection() const     { return _slurDirection; }
      void setSlurDirection(Direction d)  { _slurDirection = d; }
      void undoSetSlurDirection(Direction d);

      virtual void layout2(const QPointF, int, struct UP&)  {}
      virtual bool contains(const QPointF&) const { return false; }  // not selectable

      void writeProperties(XmlWriter& xml) const;
      bool readProperties(XmlReader&);

      int lineType() const                { return _lineType; }
      void setLineType(int val)           { _lineType = val;  }
      void undoSetLineType(int);

      virtual void slurPos(SlurPos*) = 0;
      virtual SlurTieSegment* newSlurTieSegment() = 0;

      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID id) const;
      };

}

#endif

