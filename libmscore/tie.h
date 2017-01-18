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

class TieSegment : public SlurTieSegment {
      Q_OBJECT

      QPointF autoAdjustOffset;

      void setAutoAdjust(const QPointF& offset);
      void setAutoAdjust(qreal x, qreal y)      { setAutoAdjust(QPointF(x, y)); }
      QPointF getAutoAdjust() const             { return autoAdjustOffset; }

   protected:
      void changeAnchor(MuseScoreView*, Grip, Element*);

   public:
      TieSegment(Score* s) : SlurTieSegment(s) { autoAdjustOffset = QPointF(); }
      TieSegment(const TieSegment& s) : SlurTieSegment(s) { autoAdjustOffset = QPointF(); }
      virtual TieSegment* clone() const override   { return new TieSegment(*this); }
      virtual ElementType type() const override  { return ElementType::TIE_SEGMENT; }
      virtual int subtype() const override         { return static_cast<int>(spanner()->type()); }
      virtual QString subtypeName() const override { return name(spanner()->type()); }
      virtual void draw(QPainter*) const override;

      void layoutSegment(const QPointF& p1, const QPointF& p2);

      bool isEdited() const;
      virtual void editDrag(const EditData&) override;
      virtual bool edit(MuseScoreView*, Grip grip, int key, Qt::KeyboardModifiers, const QString& s) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return int(Grip::GRIPS); }
      virtual QPointF gripAnchor(Grip grip) const override;

      QPointF getGrip(Grip) const override;
      void setGrip(Grip, const QPointF&) override;

      Tie* tie() const { return (Tie*)spanner(); }

      void computeBezier(QPointF so = QPointF());
      };

//---------------------------------------------------------
//   @@ Tie
//!    a Tie has a Note as startElement/endElement
//---------------------------------------------------------

class Tie : public SlurTie {
      Q_OBJECT

      static Note* editStartNote;
      static Note* editEndNote;

   public:
      Tie(Score* = 0);
      virtual Tie* clone() const override         { return new Tie(*this);  }
      virtual ElementType type() const override { return ElementType::TIE; }

      void setStartNote(Note* note);
      void setEndNote(Note* note)                 { setEndElement((Element*)note); }
      Note* startNote() const;
      Note* endNote() const;

      void calculateDirection();
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;
      virtual void slurPos(SlurPos*) override;
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;

      bool readProperties(XmlReader&);

      TieSegment* frontSegment() const   { return (TieSegment*)spannerSegments().front();    }
      TieSegment* backSegment() const    { return (TieSegment*)spannerSegments().back();     }
      TieSegment* takeLastSegment()      { return (TieSegment*)spannerSegments().takeLast(); }
      TieSegment* segmentAt(int n) const { return (TieSegment*)spannerSegments().at(n);      }

      virtual SlurTieSegment* newSlurTieSegment() override { return new TieSegment(score()); }
      };

}     // namespace Ms
#endif

