//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SLUR_H__
#define __SLUR_H__

#include "slurtie.h"

namespace Ms {

//---------------------------------------------------------
//   @@ SlurSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class SlurSegment final : public SlurTieSegment {

   protected:
      qreal _extraHeight = 0.0;
      virtual void changeAnchor(EditData&, Element*) override;

   public:
      SlurSegment(Score* s) : SlurTieSegment(s) {}
      SlurSegment(const SlurSegment& ss) : SlurTieSegment(ss) {}

      virtual SlurSegment* clone() const override  { return new SlurSegment(*this); }
      virtual ElementType type() const override    { return ElementType::SLUR_SEGMENT; }
      virtual int subtype() const override         { return static_cast<int>(spanner()->type()); }
      virtual QString subtypeName() const override { return name(spanner()->type()); }
      virtual void draw(QPainter*) const override;

      void layoutSegment(const QPointF& p1, const QPointF& p2);

      bool isEdited() const;
      virtual void startEdit(EditData&) override;
      virtual bool edit(EditData&) override;
      virtual void endEdit(EditData&) override;
      virtual void updateGrips(EditData&) const override;

      Slur* slur() const { return toSlur(spanner()); }

      virtual void computeBezier(QPointF so = QPointF());
      };

//---------------------------------------------------------
//   @@ Slur
//---------------------------------------------------------

class Slur final : public SlurTie {

      void slurPosChord(SlurPos*);

   public:
      Slur(Score* = 0);
      ~Slur() {}
      virtual Slur* clone() const override        { return new Slur(*this); }
      virtual ElementType type() const override { return ElementType::SLUR; }
      virtual void write(XmlWriter& xml) const override;
      virtual void layout() override;
      virtual SpannerSegment* layoutSystem(System*) override;
      virtual void setTrack(int val) override;
      virtual void slurPos(SlurPos*) override;

      SlurSegment* frontSegment()               { return toSlurSegment(Spanner::frontSegment()); }
      const SlurSegment* frontSegment() const   { return toSlurSegment(Spanner::frontSegment()); }
      SlurSegment* backSegment()                { return toSlurSegment(Spanner::backSegment());  }
      const SlurSegment* backSegment() const    { return toSlurSegment(Spanner::backSegment());  }
      SlurSegment* segmentAt(int n)             { return toSlurSegment(Spanner::segmentAt(n));   }
      const SlurSegment* segmentAt(int n) const { return toSlurSegment(Spanner::segmentAt(n));   }
      virtual SlurTieSegment* newSlurTieSegment() override { return new SlurSegment(score()); }
      };

}     // namespace Ms
#endif

