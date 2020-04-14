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

#ifndef __TIE_H__
#define __TIE_H__

#include "slurtie.h"

namespace Ms {

//---------------------------------------------------------
//   @@ TieSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class TieSegment final : public SlurTieSegment {
      QPointF autoAdjustOffset;

      void setAutoAdjust(const QPointF& offset);
      void setAutoAdjust(qreal x, qreal y)      { setAutoAdjust(QPointF(x, y)); }
      QPointF getAutoAdjust() const             { return autoAdjustOffset; }

   protected:
      void changeAnchor(EditData&, Element*) override;

   public:
      TieSegment(Score* s) : SlurTieSegment(s) { autoAdjustOffset = QPointF(); }
      TieSegment(const TieSegment& s) : SlurTieSegment(s) { autoAdjustOffset = QPointF(); }

      TieSegment* clone() const override   { return new TieSegment(*this); }
      ElementType type() const override    { return ElementType::TIE_SEGMENT; }
      int subtype() const override         { return static_cast<int>(spanner()->type()); }
      void draw(QPainter*) const override;

      void layoutSegment(const QPointF& p1, const QPointF& p2);

      bool isEdited() const;
      void editDrag(EditData&) override;
      bool edit(EditData&) override;

      Tie* tie() const { return (Tie*)spanner(); }

      void computeBezier(QPointF so = QPointF()) override;
      };

//---------------------------------------------------------
//   @@ Tie
//!    a Tie has a Note as startElement/endElement
//---------------------------------------------------------

class Tie final : public SlurTie {
      static Note* editStartNote;
      static Note* editEndNote;

   public:
      Tie(Score* = 0);

      Tie* clone() const override       { return new Tie(*this);  }
      ElementType type() const override { return ElementType::TIE; }

      void setStartNote(Note* note);
      void setEndNote(Note* note)                 { setEndElement((Element*)note); }
      Note* startNote() const;
      Note* endNote() const;

      void calculateDirection();
      void write(XmlWriter& xml) const override;
      void slurPos(SlurPos*) override;

      TieSegment* layoutFor(System*);
      TieSegment* layoutBack(System*);

      TieSegment* frontSegment()               { return toTieSegment(Spanner::frontSegment()); }
      const TieSegment* frontSegment() const   { return toTieSegment(Spanner::frontSegment()); }
      TieSegment* backSegment()                { return toTieSegment(Spanner::backSegment());  }
      const TieSegment* backSegment() const    { return toTieSegment(Spanner::backSegment());  }
      TieSegment* segmentAt(int n)             { return toTieSegment(Spanner::segmentAt(n));   }
      const TieSegment* segmentAt(int n) const { return toTieSegment(Spanner::segmentAt(n));   }

      SlurTieSegment* newSlurTieSegment() override { return new TieSegment(score()); }
      };

}     // namespace Ms
#endif

