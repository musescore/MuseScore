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

      bool operator!=(const UP& up) const { return p != up.p || off != up.off; }
      };

class SlurTie;

//---------------------------------------------------------
//   SlurTieSegment
//---------------------------------------------------------

class SlurTieSegment : public SpannerSegment {
   protected:
      struct UP _ups[int(Grip::GRIPS)];

      QPainterPath path;
      QPainterPath shapePath;
      Shape _shape;

      virtual void changeAnchor(EditData&, Element*) = 0;
      virtual QPointF gripAnchor(Grip grip) const override;

   public:
      SlurTieSegment(Score*);
      SlurTieSegment(const SlurTieSegment&);
      virtual void spatiumChanged(qreal, qreal) override;
      SlurTie* slurTie() const { return (SlurTie*)spanner(); }

      virtual void startEdit(EditData&) override;
      virtual void endEdit(EditData&) override;
      virtual void startEditDrag(EditData& ed) override;
      virtual void endEditDrag(EditData& ed) override;
      virtual void editDrag(EditData&) override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;
      virtual void reset() override;
      virtual void move(const QPointF& s) override;
      virtual bool isEditable() const override { return true; }

      void setSlurOffset(Grip i, const QPointF& val) { _ups[int(i)].off = val;  }
      const struct UP& ups(Grip i) const             { return _ups[int(i)]; }
      struct UP& ups(Grip i)                         { return _ups[int(i)]; }
      virtual Shape shape() const override           { return _shape; }

      void writeSlur(XmlWriter& xml, int no) const;
      void read(XmlReader&);
      virtual void drawEditMode(QPainter*, EditData&) override;
      virtual void computeBezier(QPointF so = QPointF()) = 0;
      };

//-------------------------------------------------------------------
//   @@ SlurTie
//   @P lineType       int  (0 - solid, 1 - dotted, 2 - dashed, 3 - wide dashed)
//   @P slurDirection  enum (Direction.AUTO, Direction.DOWN, Direction.UP)
//-------------------------------------------------------------------

class SlurTie : public Spanner {
      int _lineType;    // 0 = solid, 1 = dotted, 2 = dashed, 3 = wide dashed

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
      bool readProperties300(XmlReader&);

      int lineType() const                { return _lineType; }
      void setLineType(int val)           { _lineType = val;  }
      void undoSetLineType(int);

      virtual void slurPos(SlurPos*) = 0;
      virtual SlurTieSegment* newSlurTieSegment() = 0;


      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;
      };

}

#endif

